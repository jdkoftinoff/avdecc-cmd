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
#include "raw.h"

int arg_verbose = 1;
int arg_time_in_ms_to_wait = 1000;
const char *arg_network_port = 0;
const char *arg_protocol = 0;
const char *arg_message_type = 0;
const char *arg_sequence_id = 0;
const char *arg_destination_mac = 0;
const char *arg_entity_id = 0;
const char *arg_talker_entity_id = 0;
const char *arg_talker_unique_id = 0;
const char *arg_listener_entity_id = 0;
const char *arg_listener_unique_id = 0;
const char *arg_connection_count = 0;
const char *arg_target_entity_id = 0;
const char *arg_command = 0;
const char *arg_descriptor_type = 0;
const char *arg_descriptor_index = 0;
const char *arg_payload = 0;

void avdecc_cmd_print_frame_header( struct jdksavdecc_printer *self, const struct jdksavdecc_frame *frame )
{
    jdksavdecc_printer_print_label( self, "DA" );
    jdksavdecc_printer_print_eui48( self, frame->dest_address );
    jdksavdecc_printer_print_eol( self );
    jdksavdecc_printer_print_label( self, "SA" );
    jdksavdecc_printer_print_eui48( self, frame->src_address );
    jdksavdecc_printer_print_eol( self );
    jdksavdecc_printer_print_label( self, "EtherType" );
    jdksavdecc_printer_print_uint16( self, frame->ethertype );
    jdksavdecc_printer_print_eol( self );
    jdksavdecc_printer_print_label( self, "Payload Length" );
    jdksavdecc_printer_print_uint16( self, frame->length );
    jdksavdecc_printer_print_eol( self );
}

void avdecc_cmd_print_frame_payload( FILE *f, const struct jdksavdecc_frame *frame )
{
    fprintf( f, "\nPacket payload data:\n" );
    {
        int i;
        for ( i = 0; i < frame->length; ++i )
        {
            fprintf( f, "%02x ", frame->payload[i] );
        }
        fprintf( f, "\n" );
    }
}

void avdecc_cmd_process_incoming_raw( const void *request_,
                                      struct raw_context *net,
                                      int max_time_in_ms,
                                      int ( *process )( const void *request_,
                                                        struct raw_context *net,
                                                        const struct jdksavdecc_frame *frame ) )
{
    fd_set rd_fds;
    int nfds;
    int r;
    struct timeval timeout;

    raw_set_socket_nonblocking( net->m_fd );
    FD_ZERO( &rd_fds );
    FD_SET( net->m_fd, &rd_fds );
    nfds = net->m_fd + 1;

    timeout.tv_sec = max_time_in_ms / 1000;
    timeout.tv_usec = ( max_time_in_ms % 1000 ) * 1000;

    do
    {
        struct timeval time_portion = timeout;

        // refresh interest in readability of fd
        FD_SET( net->m_fd, &rd_fds );

        do
        {
            // wait for it to become readable
            r = select( nfds, &rd_fds, 0, 0, &time_portion );
        } while ( r < 0 && errno == EINTR );

        // any error aborts now
        if ( r < 0 )
        {
            perror( "Error on select" );
            break;
        }

        // If the socket is readable, process the message
        if ( r > 0 )
        {
            if ( FD_ISSET( net->m_fd, &rd_fds ) )
            {
                ssize_t len;
                struct jdksavdecc_frame frame;
                bzero( &frame, sizeof( frame ) );

                // Receive the ethernet frame
                len = raw_recv(
                    net, frame.src_address.value, frame.dest_address.value, frame.payload, sizeof( frame.payload ) );

                // Did we get one?
                if ( len > 0 )
                {
                    // Yes, fill in the length
                    frame.length = (uint16_t)len;
                    // And ethertype
                    frame.ethertype = net->m_ethertype;
                    // Process it.
                    if ( process( request_, net, &frame ) != 0 )
                    {
                        // Process function wants us to stop.
                        break;
                    }
                }
                else
                {
                    perror( "unable to read ethernet" );
                    break;
                }
            }
        }
    } while ( r > 0 );
}
