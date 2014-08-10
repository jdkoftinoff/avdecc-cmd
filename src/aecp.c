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
                       int command_payload_length )
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
    aemdu->aecpdu_header.header.message_type = message_type_code;

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
        if ( h->version == 0 && h->subtype == JDKSAVDECC_SUBTYPE_AECP && h->cd == 1
             && h->message_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE
             && h->control_data_length >= JDKSAVDECC_AECPDU_AEM_LEN )
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

    return r;
}
