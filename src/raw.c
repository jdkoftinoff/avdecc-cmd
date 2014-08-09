/*
Copyright (c) 2014, J.D. Koftinoff Software, Ltd.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this
  list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice,
  this list of conditions and the following disclaimer in the documentation
  and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include "raw.h"

int raw_socket( struct raw_context *self, uint16_t ethertype, const char *interface_name, const uint8_t join_multicast[6] )
{
    int fd = socket( AF_PACKET, SOCK_RAW, htons( ethertype ) );

    if ( join_multicast )
    {
        memcpy( self->m_default_dest_mac, join_multicast, 6 );
    }

    if ( fd >= 0 && interface_name )
    {
        int i;
        struct ifreq ifr;
        strncpy( ifr.ifr_name, interface_name, sizeof( ifr.ifr_name ) - 1 );
        if ( ioctl( fd, SIOCGIFINDEX, &ifr ) < 0 )
        {
            close( fd );
            return -1;
        }
        self->m_interface_id = ifr.ifr_ifindex;
        if ( ioctl( fd, SIOCGIFHWADDR, &ifr ) < 0 )
        {
            close( fd );
            return -1;
        }
        for ( i = 0; i < 6; ++i )
        {
            self->m_my_mac[i] = (uint8_t)ifr.ifr_hwaddr.sa_data[i];
        }
        self->m_fd = fd;
        self->m_ethertype = ethertype;
        if ( join_multicast )
        {
            raw_join_multicast( self, join_multicast );
        }
        raw_set_socket_nonblocking( fd );
    }
    return fd;
}

void raw_close( struct raw_context *self )
{
    if ( self->m_fd >= 0 )
    {
        close( self->m_fd );
        self->m_fd = -1;
    }
}

ssize_t raw_send( struct raw_context *self, const uint8_t dest_mac[6], const void *payload, ssize_t payload_len )
{
    ssize_t r = -1;
    ssize_t sent_len;
    struct sockaddr_ll socket_address;
    uint8_t buffer[ETH_FRAME_LEN];
    unsigned char *etherhead = buffer;
    unsigned char *data = buffer + 14;
    struct ethhdr *eh = (struct ethhdr *)etherhead;
    socket_address.sll_family = PF_PACKET;
    socket_address.sll_protocol = htons( self->m_ethertype );
    socket_address.sll_ifindex = self->m_interface_id;
    socket_address.sll_hatype = 1; /*ARPHRD_ETHER; */
    socket_address.sll_pkttype = PACKET_OTHERHOST;
    socket_address.sll_halen = ETH_ALEN;
    memcpy( socket_address.sll_addr, self->m_my_mac, ETH_ALEN );
    socket_address.sll_addr[6] = 0x00;
    socket_address.sll_addr[7] = 0x00;

    if ( dest_mac )
    {
        memcpy( (void *)buffer, (void *)dest_mac, ETH_ALEN );
    }
    else
    {
        memcpy( (void *)buffer, (void *)self->m_default_dest_mac, 6 );
    }

    memcpy( (void *)( buffer + ETH_ALEN ), (void *)self->m_my_mac, ETH_ALEN );
    eh->h_proto = htons( self->m_ethertype );
    memcpy( data, payload, payload_len );
    do
    {
        sent_len
            = sendto( self->m_fd, buffer, payload_len + 14, 0, (struct sockaddr *)&socket_address, sizeof( socket_address ) );
    } while ( sent_len < 0 && ( errno == EINTR ) );
    if ( sent_len >= 0 )
    {
        r = sent_len - 14;
    }

    return r;
}

ssize_t raw_recv(
    struct raw_context *self, uint8_t src_mac[6], uint8_t dest_mac[6], void *payload_buf, ssize_t payload_buf_max_size )
{
    ssize_t r = -1;
    ssize_t buf_len;
    uint8_t buf[2048];

    do
    {
        buf_len = recv( self->m_fd, buf, sizeof( buf ), 0 );
    } while ( buf_len < 0 && ( errno == EINTR ) );

    if ( buf_len >= 0 )
    {
        if ( src_mac )
        {
            memcpy( src_mac, &buf[6], 6 );
        }
        if ( dest_mac )
        {
            memcpy( dest_mac, &buf[0], 6 );
        }
        if ( payload_buf && ( payload_buf_max_size > buf_len - 14 ) )
        {
            memcpy( payload_buf, &buf[14], buf_len - 14 );
            r = buf_len - 14;
        }
    }
    return r;
}

int raw_join_multicast( struct raw_context *self, const uint8_t multicast_mac[6] )
{
    int r = 0;
    struct packet_mreq mreq;
    struct sockaddr_ll saddr;
    if ( multicast_mac )
    {
        memset( &saddr, 0, sizeof( saddr ) );
        saddr.sll_family = AF_PACKET;
        saddr.sll_ifindex = self->m_interface_id;
        saddr.sll_pkttype = PACKET_MULTICAST;
        saddr.sll_protocol = htons( self->m_ethertype );
        if ( bind( self->m_fd, (struct sockaddr *)&saddr, sizeof( saddr ) ) >= 0 )
        {
            memset( &mreq, 0, sizeof( mreq ) );
            mreq.mr_ifindex = self->m_interface_id;
            mreq.mr_type = PACKET_MR_MULTICAST;
            mreq.mr_alen = 6;
            mreq.mr_address[0] = multicast_mac[0];
            mreq.mr_address[1] = multicast_mac[1];
            mreq.mr_address[2] = multicast_mac[2];
            mreq.mr_address[3] = multicast_mac[3];
            mreq.mr_address[4] = multicast_mac[4];
            mreq.mr_address[5] = multicast_mac[5];
            if ( setsockopt( self->m_fd, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mreq, sizeof( mreq ) ) >= 0 )
            {
                r = 1;
            }
            else
            {
                fprintf( stderr,
                         "us_rawnet_join_multicast setsockopt[SOL_SOCKET,PACKET_ADD_MEMBERSHIP] error %s",
                         strerror( errno ) );
            }
        }
        else
        {
            fprintf( stderr, "us_rawnet_join_multicast bind error: %s", strerror( errno ) );
        }
    }
    return r;
}

void raw_set_socket_nonblocking( int fd )
{
    int val;
    int flags;
    val = fcntl( fd, F_GETFL, 0 );
    flags = O_NONBLOCK;
    val |= flags;
    fcntl( fd, F_SETFL, val );
}
