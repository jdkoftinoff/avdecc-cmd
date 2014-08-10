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
#include "adp.h"
#include "adp-cmd.h"

int main( int argc, char **argv )
{
    int r = 1;
    if ( argc < 4 )
    {
        fprintf( stderr,
                 "avdecc-discover usage:\n"
                 "\tavdecc-discover [verbosity] [timeout_in_ms] [network_port]\n\n" );
        return 1;
    }

    if ( argc > 1 )
    {
        arg_verbose = atoi( argv[1] );
    }
    if ( argc > 2 )
    {
        arg_time_in_ms_to_wait = atoi( argv[2] );
    }
    if ( argc > 3 )
    {
        arg_network_port = argv[3];
    }
    {
        struct raw_context net;
        int fd = raw_socket( &net, JDKSAVDECC_AVTP_ETHERTYPE, arg_network_port, jdksavdecc_multicast_adp_acmp.value );
        if ( fd >= 0 )
        {
            struct jdksavdecc_adpdu adpdu;
            struct jdksavdecc_frame frame;
            struct jdksavdecc_eui64 zero;
            jdksavdecc_frame_init( &frame );
            bzero( &adpdu, sizeof( adpdu ) );
            bzero( &zero, sizeof( zero ) );
            memcpy( frame.src_address.value, net.m_my_mac, 6 );

            if ( adp_form_msg( &frame, &adpdu, JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER, zero ) == 0 )
            {
                if ( raw_send( &net, frame.dest_address.value, frame.payload, frame.length ) > 0 )
                {
                    if ( arg_verbose > 0 )
                    {
                        fprintf( stdout, "Sent:\n" );
                        adp_print( stdout, &frame, &adpdu );

                        if ( arg_verbose > 1 )
                        {
                            avdecc_cmd_print_frame_payload( stdout, &frame );
                        }
                    }
                    r = 0;

                    avdecc_cmd_process_incoming_raw( &adpdu, &net, arg_time_in_ms_to_wait, adp_process );
                }
            }
            raw_close( &net );
        }
        else
        {
            fprintf( stderr, "avdecc-discover: unable to open port '%s'\n", arg_network_port );
        }
    }
    return r;
}
