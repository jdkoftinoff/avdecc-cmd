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
#include "adp.h"
#include "acmp.h"

int main( int argc, char **argv )
{
    int r = 1;
    if ( argc == 1 )
    {
        fprintf( stderr,
                 "avdecc usage:\n"
                 "\tavdecc [verbosity] [timeout_in_ms] [network_port] [protocol] ...\n"
                 "\tavdecc [verbosity] [timeout_in_ms] [network_port] adp message_type (entity_id)\n"
                 "\tavdecc [verbosity] [timeout_in_ms] [network_port] acmp message_type sequence_id talker_entity_id "
                 "talker_unique_id "
                 "listener_entity_id "
                 "listener_unique_id\n"
                 "\tavdecc [verbosity] [timeout_in_ms] [network_port] acmp message_type sequence_id talker_entity_id "
                 "talker_unique_id "
                 "listener_entity_id "
                 "listener_unique_id connection_count\n"
                 "\tavdecc [verbosity] [timeout_in_ms] [network_port] aem message_type command sequence_id destination_mac "
                 "target_entity_id "
                 "descriptor_type descriptor_index\n"
                 "\tavdecc [verbosity] [timeout_in_ms] [network_port] aem message_type command sequence_id destination_mac "
                 "target_entity_id "
                 "descriptor_type descriptor_index "
                 "payload...\n" );
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
    if ( argc > 4 )
    {
        arg_protocol = argv[4];
    }
    if ( argc > 5 )
    {
        arg_message_type = argv[5];
    }
    {
        struct raw_context net;
        int fd = raw_socket( &net, JDKSAVDECC_AVTP_ETHERTYPE, arg_network_port, jdksavdecc_multicast_adp_acmp.value );
        if ( fd >= 0 )
        {
            struct jdksavdecc_frame frame;
            jdksavdecc_frame_init( &frame );
            memcpy( frame.src_address.value, net.m_my_mac, 6 );
            if ( strcmp( arg_protocol, "adp" ) == 0 )
            {
                r = adp( &net, &frame, argc - 2, &argv[2] );
            }
            else if ( strcmp( arg_protocol, "acmp" ) == 0 )
            {
            }
            else if ( strcmp( arg_protocol, "aem" ) == 0 )
            {
            }
            raw_close( &net );
        }
        else
        {
            fprintf( stderr, "avdecc: unable to open port '%s'\n", arg_network_port );
        }
    }
    return r;
}
