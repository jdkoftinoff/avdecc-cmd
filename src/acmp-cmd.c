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
#include "acmp-cmd.h"
#include "acmp.h"
#include "raw.h"

void acmp_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_acmpdu *acmpdu )
{
    struct jdksavdecc_printer p;
    char buf[10240];
    jdksavdecc_printer_init( &p, buf, sizeof( buf ) );

    avdecc_cmd_print_frame_header( &p, frame );
    jdksavdecc_acmpdu_print( &p, acmpdu );
    fprintf( s, "%s", buf );
}

int acmp_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    const struct jdksavdecc_acmpdu *request = (const struct jdksavdecc_acmpdu *)request_;
    struct jdksavdecc_acmpdu acmpdu;

    (void)net;
    if ( acmp_check_listener( frame,
                              &acmpdu,
                              &request->controller_entity_id,
                              request->sequence_id,
                              &request->listener_entity_id,
                              request->listener_unique_id ) == 0 )
    {
        fprintf( stdout, "Received ACMPDU:\n" );
        acmp_print( stdout, frame, &acmpdu );
        fprintf( stdout, "\n" );
    }
    return 0;
}

int acmp( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv )
{
    int r = 1;
    struct jdksavdecc_acmpdu acmpdu;
    uint16_t sequence_id = 0;
    struct jdksavdecc_eui64 talker_entity_id;
    uint16_t talker_unique_id = 0;
    struct jdksavdecc_eui64 listener_entity_id;
    uint16_t listener_unique_id = 0;
    uint16_t connection_count = 0;
    int arg = 3;

    if ( argc > ++arg )
    {
        arg_sequence_id = argv[arg];
    }

    if ( argc > ++arg )
    {
        arg_talker_entity_id = argv[arg];
    }

    if ( argc > ++arg )
    {
        arg_talker_unique_id = argv[arg];
    }

    if ( argc > ++arg )
    {
        arg_listener_entity_id = argv[arg];
    }

    if ( argc > ++arg )
    {
        arg_listener_unique_id = argv[arg];
    }

    if ( argc > ++arg )
    {
        arg_connection_count = argv[arg];
    }

    bzero( &acmpdu, sizeof( acmpdu ) );
    bzero( &talker_entity_id, sizeof( talker_entity_id ) );
    bzero( &listener_entity_id, sizeof( listener_entity_id ) );

    uint16_t message_type = JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_COMMAND;

    if ( !jdksavdecc_get_uint16_value_for_name( jdksavdecc_acmpdu_print_message_type, arg_message_type, &message_type ) )
    {
        fprintf( stderr, "acmp: invalid message_type: '%s'\n", arg_message_type );
        struct jdksavdecc_uint16_name *name = jdksavdecc_acmpdu_print_message_type;
        fprintf( stdout, "ACMP message type options:\n" );
        while ( name->name )
        {
            fprintf( stderr, "\t0x%04x %s\n", name->value, name->name );
            name++;
        }
        return 1;
    }

    if ( arg_sequence_id )
    {
        sequence_id = strtol( arg_sequence_id, 0, 0 );
    }

    if ( arg_talker_entity_id )
    {
        if ( !jdksavdecc_eui64_init_from_cstr( &talker_entity_id, arg_talker_entity_id ) )
        {
            fprintf( stderr, "acmp: invalid talker_entity_id: '%s'\n", arg_talker_entity_id );
            return 1;
        }
    }

    if ( arg_talker_unique_id )
    {
        talker_unique_id = strtol( arg_talker_unique_id, 0, 0 );
    }

    if ( arg_listener_entity_id )
    {
        if ( !jdksavdecc_eui64_init_from_cstr( &listener_entity_id, arg_listener_entity_id ) )
        {
            fprintf( stderr, "acmp: invalid listener_entity_id: '%s'\n", arg_listener_entity_id );
            return 1;
        }
    }

    if ( arg_listener_unique_id )
    {
        listener_unique_id = strtol( arg_listener_unique_id, 0, 0 );
    }

    if ( arg_connection_count )
    {
        connection_count = strtol( arg_connection_count, 0, 0 );
    }

    if ( acmp_form_msg( frame,
                        &acmpdu,
                        message_type,
                        sequence_id,
                        talker_entity_id,
                        talker_unique_id,
                        listener_entity_id,
                        listener_unique_id,
                        connection_count ) == 0 )
    {
        if ( raw_send( net, frame->dest_address.value, frame->payload, frame->length ) > 0 )
        {
            if ( arg_verbose > 0 )
            {
                fprintf( stdout, "Sent ACMPDU:\n" );
                acmp_print( stdout, frame, &acmpdu );
                if ( arg_verbose > 1 )
                {
                    avdecc_cmd_print_frame_payload( stdout, frame );
                }
            }
            r = 0;
            avdecc_cmd_process_incoming_raw( &acmpdu, net, arg_time_in_ms_to_wait, acmp_process );
        }
    }
    else
    {
        fprintf( stderr, "avdecc: unable to form ACMP message\n" );
    }

    return r;
}
