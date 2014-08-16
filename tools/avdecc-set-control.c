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
#include "aecp-cmd.h"
#include "aecp.h"

int main( int argc, char **argv )
{
    int r = 1;
    uint16_t sequence_id = 0;
    struct jdksavdecc_eui48 destination_mac;
    struct jdksavdecc_eui64 target_entity_id;
    uint16_t descriptor_index = 0;
    uint8_t payload[640];
    size_t payload_length = 0;
    int arg = 0;

    if ( argc < 9 )
    {
        fprintf( stderr,
                 "avdecc-set-control usage:\n"
                 "\tavdecc-set-control [verbosity] [timeout_in_ms] [network_port] [sequence_id] [destination_mac] "
                 "[target_entity_id] "
                 "[descriptor_index] [payload...]\n\n" );
        return 1;
    }

    if ( argc > ++arg )
    {
        arg_verbose = atoi( argv[arg] );
    }
    if ( argc > ++arg )
    {
        arg_time_in_ms_to_wait = atoi( argv[arg] );
    }
    if ( argc > ++arg )
    {
        arg_network_port = argv[arg];
    }
    if ( argc > ++arg )
    {
        arg_sequence_id = argv[arg];
    }
    if ( argc > ++arg )
    {
        arg_destination_mac = argv[arg];
    }
    if ( argc > ++arg )
    {
        arg_target_entity_id = argv[arg];
    }
    if ( argc > ++arg )
    {
        arg_descriptor_index = argv[arg];
    }
    if ( argc > ++arg )
    {
        arg_payload = argv[arg];
    }

    sequence_id = (uint16_t)strtol( arg_sequence_id, 0, 0 );

    if ( arg_destination_mac )
    {
        jdksavdecc_eui48_init_from_cstr( &destination_mac, arg_destination_mac );
    }

    if ( arg_target_entity_id )
    {
        jdksavdecc_eui64_init_from_cstr( &target_entity_id, arg_target_entity_id );
    }
    else
    {
        bzero( &target_entity_id, sizeof( target_entity_id ) );
    }

    descriptor_index = (uint16_t)strtol(arg_descriptor_index, 0, 0);

    {
        /* Parse payload */
        size_t len = strlen( arg_payload );
        const char *p = arg_payload;
        size_t i;
        payload_length = len / 2;
        if ( (size_t)payload_length > sizeof( payload ) )
        {
            fprintf( stderr, "Bad payload ascii form length\n" );
            return 1;
        }
        for ( i = 0; i < payload_length; ++i )
        {
            if ( !jdksavdecc_util_parse_byte( &payload[i], p[0], p[1] ) )
            {
                fprintf( stderr, "Bad payload octets ascii form\n" );
                return 1;
            }
            p += 2;
        }
    }

    {
        struct raw_context net;
        int fd = raw_socket( &net, JDKSAVDECC_AVTP_ETHERTYPE, arg_network_port, jdksavdecc_multicast_adp_acmp.value );
        if ( fd >= 0 )
        {
            struct jdksavdecc_aem_command_set_control pdu;
            struct jdksavdecc_frame frame;
            struct jdksavdecc_eui64 zero;
            jdksavdecc_frame_init( &frame );
            bzero( &pdu, sizeof( pdu ) );
            bzero( &zero, sizeof( zero ) );
            memcpy( frame.src_address.value, net.m_my_mac, 6 );

            if ( aecp_aem_form_set_control_command(
                     &frame, &pdu, sequence_id, destination_mac, target_entity_id, descriptor_index, payload, payload_length )
                 == 0 )
            {
                if ( raw_send( &net, frame.dest_address.value, frame.payload, frame.length ) > 0 )
                {
                    if ( arg_verbose > 0 )
                    {
                        fprintf( stdout, "Sent:\n" );
                        aecp_aem_print( stdout, &frame, &pdu.aem_header );

                        if ( arg_verbose > 1 )
                        {
                            avdecc_cmd_print_frame_payload( stdout, &frame );
                        }
                    }
                    r = 0;

                    avdecc_cmd_process_incoming_raw( &pdu, &net, arg_time_in_ms_to_wait, aecp_aem_process );
                }
            }
            raw_close( &net );
        }
        else
        {
            fprintf( stderr, "avdecc-set-control: unable to open port '%s'\n", arg_network_port );
        }
    }
    return r;
}
