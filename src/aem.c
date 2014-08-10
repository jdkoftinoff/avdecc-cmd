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
#include "aem.h"
#include "jdksavdecc_aecp_print.h"
#include "jdksavdecc_aem_print.h"

int aem_form_msg( struct jdksavdecc_frame *frame, const char *message_type, const char *sequence_id )
{
    int r = -1;
    if ( message_type )
    {
        struct jdksavdecc_aecpdu_aem aemdu;
        bzero( &aemdu, sizeof( aemdu ) );
        uint16_t message_type_code;
        if ( jdksavdecc_get_uint16_value_for_name( jdksavdecc_aecp_print_message_type, message_type, &message_type_code ) )
        {
            aemdu.aecpdu_header.header.cd = 1;
            aemdu.aecpdu_header.header.subtype = JDKSAVDECC_SUBTYPE_AECP;
            aemdu.aecpdu_header.header.version = 0;
            aemdu.aecpdu_header.header.status = 0;
            aemdu.aecpdu_header.header.sv = 0;
            aemdu.aecpdu_header.header.control_data_length = JDKSAVDECC_AECPDU_AEM_LEN
                                                             - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN; // TODO: Add payload
            aemdu.aecpdu_header.header.message_type = message_type_code;

            aemdu.aecpdu_header.controller_entity_id.value[0] = frame->src_address.value[0];
            aemdu.aecpdu_header.controller_entity_id.value[1] = frame->src_address.value[1];
            aemdu.aecpdu_header.controller_entity_id.value[2] = frame->src_address.value[2];
            aemdu.aecpdu_header.controller_entity_id.value[3] = 0xff;
            aemdu.aecpdu_header.controller_entity_id.value[4] = 0xfe;
            aemdu.aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[5];
            aemdu.aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[6];
            aemdu.aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[7];

            if ( sequence_id )
            {
                aemdu.aecpdu_header.sequence_id = atoi( sequence_id );
            }

            aemdu.command_type = 0; // TODO: Fill in Command Type

            frame->length = jdksavdecc_aecpdu_aem_write( &aemdu, frame->payload, 0, sizeof( frame->payload ) );
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

int aem_check( const struct jdksavdecc_frame *frame,
               struct jdksavdecc_aecpdu_aem *aemdu,
               const struct jdksavdecc_eui64 *controller_entity_id,
               uint16_t sequence_id )
{
    return 0;
}

void aem_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_aecpdu_aem *aemdu )
{
    struct jdksavdecc_printer p;
    char buf[10240];
    jdksavdecc_printer_init( &p, buf, sizeof( buf ) );

    avdecc_cmd_print_frame_header( &p, frame );
    jdksavdecc_aecp_print( &p, &aemdu->aecpdu_header, frame->payload, 0, frame->length );
    fprintf( s, "%s", buf );
}

int aem_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    return 0;
}

int aem( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv )
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
        if ( aem_form_msg( frame, arg_message_type, arg_sequence_id ) == 0 )
        {
            if ( raw_send( net, frame->dest_address.value, frame->payload, frame->length ) > 0 )
            {
                struct jdksavdecc_aecpdu_aem aemdu;
                bzero( &aemdu, sizeof( aemdu ) );
                if ( jdksavdecc_aecpdu_aem_read( &aemdu, frame->payload, 0, frame->length ) > 0 )
                {
                    fprintf( stdout, "Sent:\n" );
                    aem_print( stdout, frame, &aemdu );

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
                avdecc_cmd_process_incoming_raw( &aemdu, net, arg_time_in_ms_to_wait, aem_process );
            }
        }
        else
        {
            fprintf( stderr, "avdecc: unable to form aem message\n" );
        }
    }
    else
    {
        struct jdksavdecc_uint16_name *name = jdksavdecc_aecp_print_message_type;
        fprintf( stdout, "aem message type options:\n" );
        while ( name->name )
        {
            fprintf( stdout, "\t%s (0x%x)\n", name->name, name->value );
            name++;
        }
    }
    return r;
}
