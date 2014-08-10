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
#include "acmp.h"

int acmp_form_msg( struct jdksavdecc_frame *frame,
                   const char *message_type,
                   const char *sequence_id,
                   const char *talker_entity_id,
                   const char *talker_unique_id,
                   const char *listener_entity_id,
                   const char *listener_unique_id )
{
    int r = -1;
    if ( message_type )
    {
        struct jdksavdecc_acmpdu acmpdu;
        bzero( &acmpdu, sizeof( acmpdu ) );
        uint16_t message_type_code;
        if ( jdksavdecc_get_uint16_value_for_name( jdksavdecc_acmpdu_print_message_type, message_type, &message_type_code ) )
        {
            acmpdu.header.cd = 1;
            acmpdu.header.subtype = JDKSAVDECC_SUBTYPE_ACMP;
            acmpdu.header.version = 0;
            acmpdu.header.status = 0;
            acmpdu.header.sv = 0;
            acmpdu.header.control_data_length = JDKSAVDECC_ACMPDU_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
            acmpdu.header.message_type = message_type_code;

            acmpdu.controller_entity_id.value[0] = frame->src_address.value[0];
            acmpdu.controller_entity_id.value[1] = frame->src_address.value[1];
            acmpdu.controller_entity_id.value[2] = frame->src_address.value[2];
            acmpdu.controller_entity_id.value[3] = 0xff;
            acmpdu.controller_entity_id.value[4] = 0xfe;
            acmpdu.controller_entity_id.value[5] = frame->src_address.value[5];
            acmpdu.controller_entity_id.value[6] = frame->src_address.value[6];
            acmpdu.controller_entity_id.value[7] = frame->src_address.value[7];

            if ( sequence_id )
            {
                acmpdu.sequence_id = atoi( sequence_id );
            }

            if ( talker_entity_id )
            {
                if ( !jdksavdecc_eui64_init_from_cstr( &acmpdu.talker_entity_id, talker_entity_id ) )
                {
                    fprintf( stderr, "acmp: invalid talker_entity_id: '%s'\n", talker_entity_id );
                    return r;
                }
            }

            if ( talker_unique_id )
            {
                acmpdu.talker_unique_id = atoi( talker_unique_id );
            }

            if ( listener_entity_id )
            {
                if ( !jdksavdecc_eui64_init_from_cstr( &acmpdu.listener_entity_id, listener_entity_id ) )
                {
                    fprintf( stderr, "acmp: invalid listener_entity_id: '%s'\n", listener_entity_id );
                    return r;
                }
            }

            if ( listener_unique_id )
            {
                acmpdu.listener_unique_id = atoi( listener_unique_id );
            }

            frame->length = jdksavdecc_acmpdu_write( &acmpdu, frame->payload, 0, sizeof( frame->payload ) );
            frame->dest_address = jdksavdecc_multicast_adp_acmp;
            frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;
            r = 0;
        }
        else
        {
            fprintf( stderr, "acmp: invalid message_type: '%s'\n", message_type );
        }
    }
    return r;
}

int acmp_check_listener( const struct jdksavdecc_frame *frame,
                         struct jdksavdecc_acmpdu *acmpdu,
                         const struct jdksavdecc_eui64 *controller_entity_id,
                         uint16_t sequence_id,
                         const struct jdksavdecc_eui64 *listener_entity_id,
                         uint16_t listener_unique_id )
{
    int r = -1;

    if ( frame->ethertype == JDKSAVDECC_AVTP_ETHERTYPE && memcmp( &frame->dest_address, &jdksavdecc_multicast_adp_acmp, 6 ) == 0
         && frame->payload[0] == JDKSAVDECC_1722A_SUBTYPE_ACMP )
    {
        bzero( acmpdu, sizeof( *acmpdu ) );
        if ( jdksavdecc_acmpdu_read( acmpdu, frame->payload, 0, frame->length ) > 0 )
        {
            struct jdksavdecc_eui64 zero;
            bzero( &zero, sizeof( zero ) );

            if ( acmpdu->header.message_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_CONNECT_RX_RESPONSE
                 || acmpdu->header.message_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_DISCONNECT_RX_RESPONSE
                 || acmpdu->header.message_type == JDKSAVDECC_ACMP_MESSAGE_TYPE_GET_RX_STATE_RESPONSE )
            {

                if ( controller_entity_id && jdksavdecc_eui64_compare( &zero, controller_entity_id ) != 0 )
                {
                    if ( jdksavdecc_eui64_compare( &acmpdu->controller_entity_id, controller_entity_id ) == 0 )
                    {
                        /* If we care about controller_entity_id then we are caring about sequence_id */
                        if ( acmpdu->sequence_id == sequence_id )
                        {
                            r = 0;
                        }
                        else
                        {
                            r = -1;
                        }
                    }
                    else
                    {
                        r = -1;
                    }
                }
                else
                {
                    r = 0;
                }

                if ( r == 0 )
                {
                    if ( listener_entity_id && jdksavdecc_eui64_compare( &zero, listener_entity_id ) != 0 )
                    {
                        if ( jdksavdecc_eui64_compare( &acmpdu->listener_entity_id, listener_entity_id ) == 0 )
                        {
                            /* If we care about listener_entity_id then we are caring about listener_unique_id */
                            if ( acmpdu->listener_unique_id == listener_unique_id )
                            {
                                r = 0;
                            }
                            else
                            {
                                r = -1;
                            }
                        }
                        else
                        {
                            r = -1;
                        }
                    }
                    else
                    {
                        r = 0;
                    }
                }
            }
        }
    }
    return r;
}

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
                    fprintf( stdout, "Sent:\n" );
                    acmp_print( stdout, frame, &acmpdu );

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
            fprintf( stdout, "\t%s (0x%x)\n", name->name, name->value );
            name++;
        }
    }
    return r;
}
