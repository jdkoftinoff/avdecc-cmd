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
#include "aecp.h"
#include "jdksavdecc_aecp_print.h"
#include "jdksavdecc_aem_print.h"

int aecp_aem_form_msg( struct jdksavdecc_frame *frame, uint16_t message_type_code, const char *sequence_id )
{
    int r = -1;
    struct jdksavdecc_aecpdu_aem aemdu;
    bzero( &aemdu, sizeof( aemdu ) );
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
    aemdu.aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[3];
    aemdu.aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[4];
    aemdu.aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[5];

    if ( sequence_id )
    {
        aemdu.aecpdu_header.sequence_id = atoi( sequence_id );
    }

    aemdu.command_type = 0; // TODO: Fill in Command Type

    frame->length = jdksavdecc_aecpdu_aem_write( &aemdu, frame->payload, 0, sizeof( frame->payload ) );
    frame->dest_address = jdksavdecc_multicast_adp_acmp;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;
    r = 0;
    return r;
}

int aecp_aem_check( const struct jdksavdecc_frame *frame,
                    struct jdksavdecc_aecpdu_aem *aem,
                    const struct jdksavdecc_eui64 *controller_entity_id,
                    uint16_t sequence_id )
{
    // TODO:
    (void)frame;
    (void)aem;
    (void)controller_entity_id;
    (void)sequence_id;

    return 0;
}

void aecp_aem_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_aecpdu_aem *aemdu )
{
    struct jdksavdecc_printer p;
    char buf[10240];
    jdksavdecc_printer_init( &p, buf, sizeof( buf ) );

    avdecc_cmd_print_frame_header( &p, frame );
    jdksavdecc_aecp_print( &p, &aemdu->aecpdu_header, frame->payload, 0, frame->length );
    fprintf( s, "%s", buf );
}

int aecp_aem_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    // TODO:
    struct jdksavdecc_aecpdu_aem *request = (struct jdksavdecc_aecpdu_aem *)request_;
    (void)net;
    (void)frame;
    (void)request;
    return 0;
}

int aecp_aem( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
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

    if ( aecp_aem_form_msg( frame, message_type, arg_sequence_id ) == 0 )
    {
        if ( raw_send( net, frame->dest_address.value, frame->payload, frame->length ) > 0 )
        {
            struct jdksavdecc_aecpdu_aem aemdu;
            bzero( &aemdu, sizeof( aemdu ) );
            if ( jdksavdecc_aecpdu_aem_read( &aemdu, frame->payload, 0, frame->length ) > 0 )
            {
                if ( arg_verbose > 0 )
                {
                    fprintf( stdout, "Sent:\n" );
                    aecp_aem_print( stdout, frame, &aemdu );

                    if ( arg_verbose > 1 )
                    {
                        avdecc_cmd_print_frame_payload( stdout, frame );
                    }
                }
                r = 0;
            }
            avdecc_cmd_process_incoming_raw( &aemdu, net, arg_time_in_ms_to_wait, aecp_aem_process );
        }
    }
    else
    {
        fprintf( stderr, "avdecc: unable to form aem message\n" );
    }

    return r;
}

int aecp_aa( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp aa\n" );
    return 1;
}

int aecp_avc( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp avc\n" );
    return 1;
}

int aecp_hdcp_apm( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp hdcp apm\n" );
    return 1;
}

int aecp_vendor( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp vendor\n" );
    return 1;
}

int aecp_extended( struct raw_context *net, struct jdksavdecc_frame *frame, uint16_t message_type, int argc, char **argv )
{
    (void)net;
    (void)frame;
    (void)message_type;
    (void)argc;
    (void)argv;
    fprintf( stderr, "TODO: aecp extended\n" );
    return 1;
}

int aecp( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv )
{
    int r = 1;
    int parsed = 0;

    if ( arg_message_type )
    {
        uint16_t message_type_code = 0;
        if ( jdksavdecc_get_uint16_value_for_name( jdksavdecc_aecp_print_message_type, arg_message_type, &message_type_code ) )
        {
            switch ( message_type_code )
            {
            case JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_ADDRESS_ACCESS_RESPONSE:
                parsed = 1;
                r = aecp_aa( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE:
                parsed = 1;
                r = aecp_aem( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_AVC_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_AVC_RESPONSE:
                parsed = 1;
                r = aecp_avc( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_HDCP_APM_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_HDCP_APM_RESPONSE:
                parsed = 1;
                r = aecp_hdcp_apm( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_VENDOR_UNIQUE_RESPONSE:
                parsed = 1;
                r = aecp_vendor( net, frame, message_type_code, argc, argv );
                break;
            case JDKSAVDECC_AECP_MESSAGE_TYPE_EXTENDED_COMMAND:
            case JDKSAVDECC_AECP_MESSAGE_TYPE_EXTENDED_RESPONSE:
                parsed = 1;
                r = aecp_extended( net, frame, message_type_code, argc, argv );
                break;
            }
        }
    }

    if ( !parsed )
    {
        struct jdksavdecc_uint16_name *name = jdksavdecc_aecp_print_message_type;
        fprintf( stdout, "aecpdu message type options:\n" );
        while ( name->name )
        {
            fprintf( stdout, "\t%s (0x%x)\n", name->name, name->value );
            name++;
        }
    }
    return r;
}
