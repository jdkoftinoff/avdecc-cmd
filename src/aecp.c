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

#include "aecp.h"

int aecp_aem_form_msg( struct jdksavdecc_frame *frame,
                       struct jdksavdecc_aecpdu_aem *aemdu,
                       uint16_t message_type_code,
                       uint16_t command_code,
                       uint16_t sequence_id,
                       struct jdksavdecc_eui48 destination_mac,
                       struct jdksavdecc_eui64 target_entity_id,
                       const uint8_t *command_payload,
                       size_t command_payload_length )
{
    int r = -1;
    frame->dest_address = destination_mac;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;

    aemdu->aecpdu_header.header.cd = 1;
    aemdu->aecpdu_header.header.subtype = JDKSAVDECC_SUBTYPE_AECP;
    aemdu->aecpdu_header.header.version = 0;
    aemdu->aecpdu_header.header.status = 0;
    aemdu->aecpdu_header.header.sv = 0;
    aemdu->aecpdu_header.header.control_data_length = JDKSAVDECC_AECPDU_AEM_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN
                                                      + command_payload_length;
    aemdu->aecpdu_header.header.message_type = (uint8_t)message_type_code;

    aemdu->aecpdu_header.controller_entity_id.value[0] = frame->src_address.value[0];
    aemdu->aecpdu_header.controller_entity_id.value[1] = frame->src_address.value[1];
    aemdu->aecpdu_header.controller_entity_id.value[2] = frame->src_address.value[2];
    aemdu->aecpdu_header.controller_entity_id.value[3] = 0xff;
    aemdu->aecpdu_header.controller_entity_id.value[4] = 0xfe;
    aemdu->aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[3];
    aemdu->aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[4];
    aemdu->aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[5];

    aemdu->command_type = command_code;
    aemdu->aecpdu_header.sequence_id = sequence_id;
    aemdu->aecpdu_header.header.target_entity_id = target_entity_id;

    frame->length = jdksavdecc_aecpdu_aem_write( aemdu, frame->payload, 0, sizeof( frame->payload ) );
    if ( frame->length + command_payload_length < (int)sizeof( frame->payload ) )
    {
        memcpy( frame->payload + frame->length, command_payload, command_payload_length );
        frame->length += command_payload_length;
        r = 0;
    }
    else
    {
        r = 1;
    }
    return r;
}

int aecp_aem_form_read_descriptor_command( struct jdksavdecc_frame *frame,
                                           struct jdksavdecc_aem_command_read_descriptor *pdu,
                                           uint16_t sequence_id,
                                           struct jdksavdecc_eui48 destination_mac,
                                           struct jdksavdecc_eui64 target_entity_id,
                                           uint16_t descriptor_type,
                                           uint16_t descriptor_index )
{
    int r = -1;
    frame->dest_address = destination_mac;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;

    pdu->aem_header.aecpdu_header.header.cd = 1;
    pdu->aem_header.aecpdu_header.header.subtype = JDKSAVDECC_SUBTYPE_AECP;
    pdu->aem_header.aecpdu_header.header.version = 0;
    pdu->aem_header.aecpdu_header.header.status = 0;
    pdu->aem_header.aecpdu_header.header.sv = 0;
    pdu->aem_header.aecpdu_header.header.control_data_length = JDKSAVDECC_AEM_COMMAND_READ_DESCRIPTOR_COMMAND_LEN
                                                               - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
    pdu->aem_header.aecpdu_header.header.message_type = JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND;

    pdu->aem_header.aecpdu_header.controller_entity_id.value[0] = frame->src_address.value[0];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[1] = frame->src_address.value[1];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[2] = frame->src_address.value[2];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[3] = 0xff;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[4] = 0xfe;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[3];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[4];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[5];

    pdu->aem_header.command_type = JDKSAVDECC_AEM_COMMAND_READ_DESCRIPTOR;

    pdu->aem_header.aecpdu_header.sequence_id = sequence_id;
    pdu->aem_header.aecpdu_header.header.target_entity_id = target_entity_id;

    pdu->configuration_index = 0;
    pdu->descriptor_type = descriptor_type;
    pdu->descriptor_index = descriptor_index;

    frame->length = jdksavdecc_aem_command_read_descriptor_write( pdu, frame->payload, 0, sizeof( frame->payload ) );
    if ( frame->length > 0 )
    {
        r = 0;
    }
    else
    {
        r = 1;
    }
    return r;
}

int aecp_aem_form_get_control_command( struct jdksavdecc_frame *frame,
                                       struct jdksavdecc_aem_command_get_control *pdu,
                                       uint16_t sequence_id,
                                       struct jdksavdecc_eui48 destination_mac,
                                       struct jdksavdecc_eui64 target_entity_id,
                                       uint16_t descriptor_index )
{
    int r = -1;
    frame->dest_address = destination_mac;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;

    pdu->aem_header.aecpdu_header.header.cd = 1;
    pdu->aem_header.aecpdu_header.header.subtype = JDKSAVDECC_SUBTYPE_AECP;
    pdu->aem_header.aecpdu_header.header.version = 0;
    pdu->aem_header.aecpdu_header.header.status = 0;
    pdu->aem_header.aecpdu_header.header.sv = 0;
    pdu->aem_header.aecpdu_header.header.control_data_length = JDKSAVDECC_AEM_COMMAND_GET_CONTROL_COMMAND_LEN
                                                               - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
    pdu->aem_header.aecpdu_header.header.message_type = JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND;

    pdu->aem_header.aecpdu_header.controller_entity_id.value[0] = frame->src_address.value[0];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[1] = frame->src_address.value[1];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[2] = frame->src_address.value[2];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[3] = 0xff;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[4] = 0xfe;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[3];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[4];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[5];

    pdu->aem_header.command_type = JDKSAVDECC_AEM_COMMAND_GET_CONTROL;

    pdu->aem_header.aecpdu_header.sequence_id = sequence_id;
    pdu->aem_header.aecpdu_header.header.target_entity_id = target_entity_id;

    pdu->descriptor_type = JDKSAVDECC_DESCRIPTOR_CONTROL;
    pdu->descriptor_index = descriptor_index;

    frame->length = jdksavdecc_aem_command_get_control_write( pdu, frame->payload, 0, sizeof( frame->payload ) );
    if ( frame->length > 0 )
    {
        r = 0;
    }
    else
    {
        r = 1;
    }
    return r;
}

int aecp_aem_form_d18_get_control_command( struct jdksavdecc_frame *frame,
                                       struct jdksavdecc_aem_command_get_control *pdu,
                                       uint16_t sequence_id,
                                       struct jdksavdecc_eui48 destination_mac,
                                       struct jdksavdecc_eui64 target_entity_id,
                                       uint16_t descriptor_index )
{
    int r = -1;
    frame->dest_address = destination_mac;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;

    pdu->aem_header.aecpdu_header.header.cd = 1;
    pdu->aem_header.aecpdu_header.header.subtype = JDKSAVDECC_SUBTYPE_AECP;
    pdu->aem_header.aecpdu_header.header.version = 0;
    pdu->aem_header.aecpdu_header.header.status = 0;
    pdu->aem_header.aecpdu_header.header.sv = 0;
    pdu->aem_header.aecpdu_header.header.control_data_length = JDKSAVDECC_AEM_COMMAND_GET_CONTROL_COMMAND_LEN
                                                               - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
    pdu->aem_header.aecpdu_header.header.message_type = JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND;

    pdu->aem_header.aecpdu_header.controller_entity_id.value[0] = frame->src_address.value[0];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[1] = frame->src_address.value[1];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[2] = frame->src_address.value[2];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[3] = 0xff;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[4] = 0xfe;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[3];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[4];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[5];

    pdu->aem_header.command_type = 0x0c; // Draft 18 GET_CONTROL_VALUE

    pdu->aem_header.aecpdu_header.sequence_id = sequence_id;
    pdu->aem_header.aecpdu_header.header.target_entity_id = target_entity_id;

    pdu->descriptor_type = JDKSAVDECC_DESCRIPTOR_CONTROL;
    pdu->descriptor_index = descriptor_index;

    frame->length = jdksavdecc_aem_command_get_control_write( pdu, frame->payload, 0, sizeof( frame->payload ) );
    if ( frame->length > 0 )
    {
        r = 0;
    }
    else
    {
        r = 1;
    }
    return r;
}

int aecp_aem_form_set_control_command( struct jdksavdecc_frame *frame,
                                       struct jdksavdecc_aem_command_set_control *pdu,
                                       uint16_t sequence_id,
                                       struct jdksavdecc_eui48 destination_mac,
                                       struct jdksavdecc_eui64 target_entity_id,
                                       uint16_t descriptor_index,
                                       const uint8_t *control_data_payload,
                                       size_t control_data_payload_length )
{
    int r = -1;
    frame->dest_address = destination_mac;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;

    pdu->aem_header.aecpdu_header.header.cd = 1;
    pdu->aem_header.aecpdu_header.header.subtype = JDKSAVDECC_SUBTYPE_AECP;
    pdu->aem_header.aecpdu_header.header.version = 0;
    pdu->aem_header.aecpdu_header.header.status = 0;
    pdu->aem_header.aecpdu_header.header.sv = 0;
    pdu->aem_header.aecpdu_header.header.control_data_length
        = JDKSAVDECC_AEM_COMMAND_SET_CONTROL_COMMAND_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + control_data_payload_length;

    pdu->aem_header.aecpdu_header.header.message_type = JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND;

    pdu->aem_header.aecpdu_header.controller_entity_id.value[0] = frame->src_address.value[0];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[1] = frame->src_address.value[1];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[2] = frame->src_address.value[2];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[3] = 0xff;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[4] = 0xfe;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[3];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[4];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[5];

    pdu->aem_header.command_type = JDKSAVDECC_AEM_COMMAND_SET_CONTROL;

    pdu->aem_header.aecpdu_header.sequence_id = sequence_id;
    pdu->aem_header.aecpdu_header.header.target_entity_id = target_entity_id;

    pdu->descriptor_type = JDKSAVDECC_DESCRIPTOR_CONTROL;
    pdu->descriptor_index = descriptor_index;

    frame->length = jdksavdecc_aem_command_set_control_write( pdu, frame->payload, 0, sizeof( frame->payload ) );

    if ( frame->length + control_data_payload_length < (int)sizeof( frame->payload ) )
    {
        memcpy( frame->payload + frame->length, control_data_payload, control_data_payload_length );
        frame->length += control_data_payload_length;
        r = 0;
    }
    else
    {
        r = 1;
    }
    return r;
}


int aecp_aem_form_d18_set_control_command( struct jdksavdecc_frame *frame,
                                       struct jdksavdecc_aem_command_set_control *pdu,
                                       uint16_t sequence_id,
                                       struct jdksavdecc_eui48 destination_mac,
                                       struct jdksavdecc_eui64 target_entity_id,
                                       uint16_t descriptor_index,
                                       const uint8_t *control_data_payload,
                                       size_t control_data_payload_length )
{
    int r = -1;
    frame->dest_address = destination_mac;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;

    pdu->aem_header.aecpdu_header.header.cd = 1;
    pdu->aem_header.aecpdu_header.header.subtype = JDKSAVDECC_SUBTYPE_AECP;
    pdu->aem_header.aecpdu_header.header.version = 0;
    pdu->aem_header.aecpdu_header.header.status = 0;
    pdu->aem_header.aecpdu_header.header.sv = 0;
    pdu->aem_header.aecpdu_header.header.control_data_length
        = JDKSAVDECC_AEM_COMMAND_SET_CONTROL_COMMAND_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN + control_data_payload_length;

    pdu->aem_header.aecpdu_header.header.message_type = JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND;

    pdu->aem_header.aecpdu_header.controller_entity_id.value[0] = frame->src_address.value[0];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[1] = frame->src_address.value[1];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[2] = frame->src_address.value[2];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[3] = 0xff;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[4] = 0xfe;
    pdu->aem_header.aecpdu_header.controller_entity_id.value[5] = frame->src_address.value[3];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[6] = frame->src_address.value[4];
    pdu->aem_header.aecpdu_header.controller_entity_id.value[7] = frame->src_address.value[5];

    pdu->aem_header.command_type = 0x0b; // D18 SET_CONTROL_VALUE

    pdu->aem_header.aecpdu_header.sequence_id = sequence_id;
    pdu->aem_header.aecpdu_header.header.target_entity_id = target_entity_id;

    pdu->descriptor_type = JDKSAVDECC_DESCRIPTOR_CONTROL;
    pdu->descriptor_index = descriptor_index;

    frame->length = jdksavdecc_aem_command_set_control_write( pdu, frame->payload, 0, sizeof( frame->payload ) );

    if ( frame->length + control_data_payload_length < (int)sizeof( frame->payload ) )
    {
        memcpy( frame->payload + frame->length, control_data_payload, control_data_payload_length );
        frame->length += control_data_payload_length;
        r = 0;
    }
    else
    {
        r = 1;
    }
    return r;
}

int aecp_aem_check( const struct jdksavdecc_frame *frame,
                    struct jdksavdecc_aecpdu_aem *aem,
                    const struct jdksavdecc_eui64 controller_entity_id,
                    const struct jdksavdecc_eui64 target_entity_id,
                    uint16_t sequence_id )
{
    int r = -1;
    ssize_t pos = jdksavdecc_aecpdu_aem_read( aem, frame->payload, 0, frame->length );
    if ( pos > 0 )
    {
        struct jdksavdecc_aecpdu_common_control_header *h = &aem->aecpdu_header.header;
        if ( h->version == 0 && h->subtype == JDKSAVDECC_SUBTYPE_AECP && h->cd == 1 )
        {
            if ( h->message_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE )
            {
                if ( h->control_data_length >= JDKSAVDECC_AECPDU_AEM_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN )
                {
                    if ( jdksavdecc_eui64_compare( &aem->aecpdu_header.controller_entity_id, &controller_entity_id ) == 0 )
                    {
                        if ( jdksavdecc_eui64_compare( &aem->aecpdu_header.header.target_entity_id, &target_entity_id ) == 0 )
                        {
                            if ( aem->aecpdu_header.sequence_id == sequence_id )
                            {
                                r = 0;
                            }
                        }
                    }
                }
            }
        }
    }

    return r;
}
