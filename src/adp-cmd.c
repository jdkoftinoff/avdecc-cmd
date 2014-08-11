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
#include "adp-cmd.h"
#include "adp.h"

void adp_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_adpdu *adpdu )
{
    struct jdksavdecc_printer p;
    char buf[10240];
    jdksavdecc_printer_init( &p, buf, sizeof( buf ) );

    avdecc_cmd_print_frame_header( &p, frame );
    jdksavdecc_adpdu_print( &p, adpdu );
    fprintf( s, "%s", buf );
}

int adp_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    const struct jdksavdecc_adpdu *request = (const struct jdksavdecc_adpdu *)request_;
    struct jdksavdecc_adpdu adpdu;

    (void)net;
    if ( adp_check( frame, &adpdu, &request->header.entity_id ) == 0 )
    {
        fprintf( stdout, "Received ADPDU:\n" );
        adp_print( stdout, frame, &adpdu );
        fprintf( stdout, "\n" );
    }
    return 0;
}

int adp( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv )
{
    int r = 1;
    struct jdksavdecc_eui64 entity_id;
    struct jdksavdecc_adpdu adpdu;
    bzero( &entity_id, sizeof( entity_id ) );
    bzero( &adpdu, sizeof( adpdu ) );
    uint16_t message_type_code;

    if ( argc > 4 )
    {
        arg_entity_id = argv[4];
        if ( *arg_entity_id == 0 )
        {
            arg_entity_id = 0;
        }
    }

    if ( jdksavdecc_get_uint16_value_for_name( jdksavdecc_adpdu_print_message_type, arg_message_type, &message_type_code ) )
    {
        if ( arg_entity_id )
        {
            if ( !jdksavdecc_eui64_init_from_cstr( &entity_id, arg_entity_id ) )
            {
                fprintf( stderr, "ADP: invalid entity_id: '%s'\n", arg_entity_id );
                return 1;
            }
        }

        if ( adp_form_msg( frame, &adpdu, message_type_code, entity_id ) == 0 )
        {
            if ( raw_send( net, frame->dest_address.value, frame->payload, frame->length ) > 0 )
            {
                if ( arg_verbose > 0 )
                {
                    fprintf( stdout, "Sent:\n" );
                    adp_print( stdout, frame, &adpdu );

                    if ( arg_verbose > 1 )
                    {
                        avdecc_cmd_print_frame_payload( stdout, frame );
                    }
                }
                r = 0;
                avdecc_cmd_process_incoming_raw( &adpdu, net, arg_time_in_ms_to_wait, adp_process );
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
            fprintf( stdout, "\t0x%04x %s\n", name->value, name->name );
            name++;
        }
    }
    return r;
}
