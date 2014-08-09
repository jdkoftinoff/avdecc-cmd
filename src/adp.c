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
        struct jdksavdecc_adpdu adp;
        bzero( &adp, sizeof( adp ) );
        uint16_t message_type_code;
        if ( jdksavdecc_get_uint16_value_for_name( jdksavdecc_adpdu_print_message_type, message_type, &message_type_code ) )
        {
            adp.header.cd = 1;
            adp.header.subtype = JDKSAVDECC_SUBTYPE_ADP;
            adp.header.version = 0;
            adp.header.valid_time = 0;
            adp.header.sv = 0;
            adp.header.control_data_length = JDKSAVDECC_ADPDU_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
            adp.header.message_type = message_type_code;

            if ( target_entity )
            {
                if ( !jdksavdecc_eui64_init_from_cstr( &adp.header.entity_id, target_entity ) )
                {
                    fprintf( stderr, "ADP: invalid entity_id: '%s'\n", target_entity );
                    return r;
                }
            }

            frame->length = jdksavdecc_adpdu_write( &adp, frame->payload, 0, sizeof( frame->payload ) );
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

int adp_check( const struct jdksavdecc_frame *frame, struct jdksavdecc_adpdu *adp, const char *target_entity )
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

        bzero( adp, sizeof( *adp ) );
        if ( jdksavdecc_adpdu_read( adp, frame->payload, 0, frame->length ) > 0 )
        {
            if ( target_entity )
            {
                if ( memcmp( adp->header.entity_id.value, target_entity_id.value, 6 ) == 0 )
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

void adp_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_adpdu *adp )
{
    struct jdksavdecc_printer p;
    char buf[10240];
    jdksavdecc_printer_init( &p, buf, sizeof( buf ) );
    jdksavdecc_printer_print_label( &p, "DA" );
    jdksavdecc_printer_print_eui48( &p, frame->dest_address );
    jdksavdecc_printer_print_eol( &p );
    jdksavdecc_printer_print_label( &p, "SA" );
    jdksavdecc_printer_print_eui48( &p, frame->src_address );
    jdksavdecc_printer_print_eol( &p );
    jdksavdecc_printer_print_label( &p, "EtherType" );
    jdksavdecc_printer_print_uint16( &p, frame->ethertype );
    jdksavdecc_printer_print_eol( &p );
    jdksavdecc_printer_print_label( &p, "Payload Length" );
    jdksavdecc_printer_print_uint16( &p, frame->length );
    jdksavdecc_printer_print_eol( &p );

    jdksavdecc_adpdu_print( &p, adp );
    fprintf( s, "%s", buf );
}
