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
                   struct jdksavdecc_acmpdu *acmpdu,
                   uint16_t message_type,
                   uint16_t sequence_id,
                   struct jdksavdecc_eui64 talker_entity_id,
                   uint16_t talker_unique_id,
                   struct jdksavdecc_eui64 listener_entity_id,
                   uint16_t listener_unique_id,
                   uint16_t connection_count )
{
    int r = -1;

    acmpdu->header.cd = 1;
    acmpdu->header.subtype = JDKSAVDECC_SUBTYPE_ACMP;
    acmpdu->header.version = 0;
    acmpdu->header.status = 0;
    acmpdu->header.sv = 0;
    acmpdu->header.control_data_length = JDKSAVDECC_ACMPDU_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
    acmpdu->header.message_type = message_type;

    acmpdu->controller_entity_id.value[0] = frame->src_address.value[0];
    acmpdu->controller_entity_id.value[1] = frame->src_address.value[1];
    acmpdu->controller_entity_id.value[2] = frame->src_address.value[2];
    acmpdu->controller_entity_id.value[3] = 0xff;
    acmpdu->controller_entity_id.value[4] = 0xfe;
    acmpdu->controller_entity_id.value[5] = frame->src_address.value[3];
    acmpdu->controller_entity_id.value[6] = frame->src_address.value[4];
    acmpdu->controller_entity_id.value[7] = frame->src_address.value[5];

    acmpdu->sequence_id = sequence_id;

    acmpdu->talker_entity_id = talker_entity_id;

    acmpdu->talker_unique_id = talker_unique_id;
    acmpdu->listener_entity_id = listener_entity_id;
    acmpdu->listener_unique_id = listener_unique_id;
    acmpdu->connection_count = connection_count;

    frame->length = jdksavdecc_acmpdu_write( acmpdu, frame->payload, 0, sizeof( frame->payload ) );
    frame->dest_address = jdksavdecc_multicast_adp_acmp;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;
    r = 0;

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
