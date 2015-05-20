#pragma once
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

#include "avdecc-cmd.h"

#ifdef __cplusplus
extern "C" {
#endif

#if defined( __linux__ )

struct raw_context
{
    int m_fd;
    uint16_t m_ethertype;
    uint8_t m_my_mac[6];
    uint8_t m_default_dest_mac[6];
    int m_interface_id;
    void *m_additional;
};

#elif defined( __APPLE__ )

#define AF_PACKET AF_LINK

struct raw_context
{
    int m_fd;
    uint16_t m_ethertype;
    uint8_t m_my_mac[6];
    uint8_t m_default_dest_mac[6];
    int m_interface_id;
    void *m_additional;
    void *m_pcap;
};

#elif defined( _WIN32 )

struct raw_context
{
    SOCKET m_fd;
    uint16_t m_ethertype;
    uint8_t m_my_mac[6];
    uint8_t m_default_dest_mac[6];
    int m_interface_id;
    void *m_additional;
    void *m_pcap;
};

#endif

int raw_socket( struct raw_context *self, uint16_t ethertype, const char *interface_name, const uint8_t join_multicast[6] );

void raw_close( struct raw_context *self );

ssize_t raw_send( struct raw_context *self, const uint8_t dest_mac[6], const void *arg_payload, ssize_t payload_len );

ssize_t raw_recv(
    struct raw_context *self, uint8_t src_mac[6], uint8_t dest_mac[6], void *payload_buf, ssize_t payload_buf_max_size );

int raw_join_multicast( struct raw_context *self, const uint8_t multicast_mac[6] );

void raw_set_socket_nonblocking( int fd );

jdksavdecc_timestamp_in_milliseconds raw_get_time_of_day_in_milliseconds();

void raw_dispatch_one( const void *context,
                       struct raw_context *net,
                       int max_time_in_ms,
                       int ( *process_incoming )( const void *context,
                                                  struct raw_context *net,
                                                  const struct jdksavdecc_frame *frame ),
                       bool ( *wake_on_writable )( const void *context, struct raw_context *net ),
                       int ( *writeable )( const void *context, struct raw_context *net ) );

#ifdef __cplusplus
}
#endif
