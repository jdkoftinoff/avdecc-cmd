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

const char *network_port = "eth0";
const char *protocol = "adp";
const char *message_type = 0;
const char *entity_id = 0;
const char *talker_entity_id = 0;
const char *talker_unique_id = 0;
const char *listener_entity_id = 0;
const char *listener_unique_id = 0;
const char *connection_count = 0;
const char *command_type = 0;
const char *descriptor_type = 0;
const char *descriptor_index = 0;
const char *payload = 0;

int adp( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv )
{
    int r = 1;
    if ( argc > 4 )
    {
        entity_id = argv[4];
        if ( *entity_id == 0 )
        {
            entity_id = 0;
        }
    }

    if ( message_type )
    {
        if ( adp_form_msg( frame, message_type, entity_id ) == 0 )
        {
            if ( raw_send( net, frame->dest_address.value, frame->payload, frame->length ) > 0 )
            {
                struct jdksavdecc_adpdu adp;
                bzero( &adp, sizeof( adp ) );
                if ( jdksavdecc_adpdu_read( &adp, frame->payload, 0, frame->length ) > 0 )
                {
                    fprintf( stdout, "Sent:\n" );
                    adp_print( stdout, frame, &adp );

                    fprintf( stdout, "\nPacket payload data:\n" );
                    {
                        int i;
                        for ( i = 0; i < frame->length; ++i )
                        {
                            fprintf( stdout, "%02x ", frame->payload[i] );
                        }
                        fprintf( stdout, "\n" );
                    }
                    r = 0;
                }
            }
        }
        else
        {
            fprintf( stderr, "avdecc: unable to form adp message\n" );
        }
    }
    else
    {
        struct jdksavdecc_uint16_name *name = jdksavdecc_adpdu_print_message_type;
        fprintf( stdout, "ADP message type options:\n" );
        while ( name->name )
        {
            fprintf( stdout, "\t%s (0x%x)\n", name->name, name->value );
            name++;
        }
    }
    return r;
}

int main( int argc, char **argv )
{
    int r = 1;
    if ( argc == 1 )
    {
        fprintf( stderr,
                 "avdecc usage:\n"
                 "\tavdecc [network_port] [protocol] ...\n"
                 "\tavdecc [network_port] adp message_type (entity_id)\n"
                 "\tavdecc [network_port] acmp message_type talker_entity_id talker_unique_id listener_entity_id "
                 "listener_unique_id\n"
                 "\tavdecc [network_port] acmp message_type talker_entity_id talker_unique_id listener_entity_id "
                 "listener_unique_id connection_count\n"
                 "\tavdecc [network_port] aem message_type target_entity_id command_type descriptor_type descriptor_index\n"
                 "\tavdecc [network_port] aem message_type target_entity_id command_type descriptor_type descriptor_index "
                 "payload...\n" );
        return 1;
    }
    if ( argc > 1 )
    {
        network_port = argv[1];
    }
    if ( argc > 2 )
    {
        protocol = argv[2];
    }
    if ( argc > 3 )
    {
        message_type = argv[3];
    }
    {
        struct raw_context net;
        int fd = raw_socket( &net, JDKSAVDECC_AVTP_ETHERTYPE, network_port, jdksavdecc_multicast_adp_acmp.value );
        if ( fd >= 0 )
        {
            struct jdksavdecc_frame frame;
            jdksavdecc_frame_init( &frame );
            memcpy( frame.src_address.value, net.m_my_mac, 6 );
            if ( strcmp( protocol, "adp" ) == 0 )
            {
                r = adp( &net, &frame, argc, argv );
            }
            else if ( strcmp( protocol, "acmp" ) == 0 )
            {
            }
            else if ( strcmp( protocol, "aem" ) == 0 )
            {
            }
            raw_close( &net );
        }
        else
        {
            fprintf( stderr, "avdecc: unable to open port '%s'\n", network_port );
        }
    }
    return r;
}
