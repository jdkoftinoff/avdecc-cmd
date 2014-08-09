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

int adp_form_msg( struct jdksavdecc_frame *frame, const char *message_type, const char *target_entity )
{
    int r = -1;
    if ( message_type )
    {
        struct jdksavdecc_adpdu adpdu;
        bzero( &adpdu, sizeof( adpdu ) );
        uint16_t message_type_code;
        if ( jdksavdecc_get_uint16_value_for_name( jdksavdecc_adpdu_print_message_type, message_type, &message_type_code ) )
        {
            adpdu.header.cd = 1;
            adpdu.header.subtype = JDKSAVDECC_SUBTYPE_ADP;
            adpdu.header.version = 0;
            adpdu.header.valid_time = 0;
            adpdu.header.sv = 0;
            adpdu.header.control_data_length = JDKSAVDECC_ADPDU_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
            adpdu.header.message_type = message_type_code;

            if ( target_entity )
            {
                if ( !jdksavdecc_eui64_init_from_cstr( &adpdu.header.entity_id, target_entity ) )
                {
                    fprintf( stderr, "ADP: invalid entity_id: '%s'\n", target_entity );
                    return r;
                }
            }

            frame->length = jdksavdecc_adpdu_write( &adpdu, frame->payload, 0, sizeof( frame->payload ) );
            frame->dest_address = jdksavdecc_multicast_adp_acmp;
            frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;
            r = 0;
        }
        else
        {
            fprintf( stderr, "ADP: invalid message_type: '%s'\n", message_type );
        }
    }
    return r;
}

int adp_check( const struct jdksavdecc_frame *frame, struct jdksavdecc_adpdu *adpdu, const char *target_entity )
{
    int r = -1;

    if ( frame->ethertype == JDKSAVDECC_AVTP_ETHERTYPE && memcmp( &frame->dest_address, &jdksavdecc_multicast_adp_acmp, 6 ) == 0
         && frame->payload[0] == JDKSAVDECC_1722A_SUBTYPE_ADP )
    {
        struct jdksavdecc_eui64 target_entity_id;
        jdksavdecc_eui64_init( &target_entity_id );

        if ( target_entity )
        {
            if ( !jdksavdecc_eui64_init_from_cstr( &target_entity_id, target_entity ) )
            {
                fprintf( stderr, "ADP: invalid entity_id: '%s'\n", target_entity );
                return r;
            }
        }

        bzero( adpdu, sizeof( *adpdu ) );
        if ( jdksavdecc_adpdu_read( adpdu, frame->payload, 0, frame->length ) > 0 )
        {
            if ( target_entity )
            {
                if ( memcmp( adpdu->header.entity_id.value, target_entity_id.value, 6 ) == 0 )
                {
                    r = 0;
                }
            }
            else
            {
                r = 0;
            }
        }
    }
    return r;
}

void adp_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_adpdu *adpdu )
{
    struct jdksavdecc_printer p;
    char buf[10240];
    jdksavdecc_printer_init( &p, buf, sizeof( buf ) );

    avdecc_cmd_print_frame_header( &p, frame );
    jdksavdecc_adpdu_print( &p, adpdu );
    fprintf( s, "%s", buf );
}

int adp_process( struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    struct jdksavdecc_adpdu adpdu;
    ssize_t len;

    (void)net;
    bzero( &adpdu, sizeof( adpdu ) );
    len = jdksavdecc_adpdu_read( &adpdu, frame->payload, 0, frame->length );
    if ( len > 0 )
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
        if ( adp_form_msg( frame, arg_message_type, arg_entity_id ) == 0 )
        {
            if ( raw_send( net, frame->dest_address.value, frame->payload, frame->length ) > 0 )
            {
                struct jdksavdecc_adpdu adpdu;
                bzero( &adpdu, sizeof( adpdu ) );
                if ( jdksavdecc_adpdu_read( &adpdu, frame->payload, 0, frame->length ) > 0 )
                {
                    fprintf( stdout, "Sent:\n" );
                    adp_print( stdout, frame, &adpdu );

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
                avdecc_cmd_process_incoming_raw( net, arg_time_in_ms_to_wait, adp_process );
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
