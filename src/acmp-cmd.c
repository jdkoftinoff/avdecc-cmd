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
    if ( argc > 4 )
    {
        arg_entity_id = argv[4];
        if ( *arg_entity_id == 0 )
        {
            arg_entity_id = 0;
        }
    }

    if ( arg_message_type )
    {
        if ( acmp_form_msg( frame,
                            arg_message_type,
                            arg_sequence_id,
                            arg_talker_entity_id,
                            arg_talker_unique_id,
                            arg_listener_entity_id,
                            arg_listener_unique_id ) == 0 )
        {
            if ( raw_send( net, frame->dest_address.value, frame->payload, frame->length ) > 0 )
            {
                struct jdksavdecc_acmpdu acmpdu;
                bzero( &acmpdu, sizeof( acmpdu ) );
                if ( jdksavdecc_acmpdu_read( &acmpdu, frame->payload, 0, frame->length ) > 0 )
                {
                    if ( arg_verbose > 0 )
                    {
                        fprintf( stdout, "Sent:\n" );
                        acmp_print( stdout, frame, &acmpdu );
                        if ( arg_verbose > 1 )
                        {
                            avdecc_cmd_print_frame_payload( stdout, frame );
                        }
                    }
                    r = 0;
                }
                avdecc_cmd_process_incoming_raw( &acmpdu, net, arg_time_in_ms_to_wait, acmp_process );
            }
        }
        else
        {
            fprintf( stderr, "avdecc: unable to form acmp message\n" );
        }
    }
    else
    {
        struct jdksavdecc_uint16_name *name = jdksavdecc_acmpdu_print_message_type;
        fprintf( stdout, "acmp message type options:\n" );
        while ( name->name )
        {
            fprintf( stdout, "\t0x%04x %s\n", name->value, name->name );
            name++;
        }
    }
    return r;
}
