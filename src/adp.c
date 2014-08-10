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

int adp_form_msg( struct jdksavdecc_frame *frame,
                  struct jdksavdecc_adpdu *adpdu,
                  uint16_t message_type,
                  struct jdksavdecc_eui64 target_entity )
{
    int r = -1;
    adpdu->header.cd = 1;
    adpdu->header.subtype = JDKSAVDECC_SUBTYPE_ADP;
    adpdu->header.version = 0;
    adpdu->header.sv = 0;
    adpdu->header.control_data_length = JDKSAVDECC_ADPDU_LEN - JDKSAVDECC_COMMON_CONTROL_HEADER_LEN;
    adpdu->header.message_type = message_type;
    adpdu->header.entity_id = target_entity;
    frame->length = jdksavdecc_adpdu_write( adpdu, frame->payload, 0, sizeof( frame->payload ) );
    frame->dest_address = jdksavdecc_multicast_adp_acmp;
    frame->ethertype = JDKSAVDECC_AVTP_ETHERTYPE;
    r = 0;
    return r;
}

int adp_check( const struct jdksavdecc_frame *frame,
               struct jdksavdecc_adpdu *adpdu,
               const struct jdksavdecc_eui64 *target_entity_id )
{
    int r = -1;

    if ( frame->ethertype == JDKSAVDECC_AVTP_ETHERTYPE && memcmp( &frame->dest_address, &jdksavdecc_multicast_adp_acmp, 6 ) == 0
         && frame->payload[0] == JDKSAVDECC_1722A_SUBTYPE_ADP )
    {
        bzero( adpdu, sizeof( *adpdu ) );
        if ( jdksavdecc_adpdu_read( adpdu, frame->payload, 0, frame->length ) > 0 )
        {
            struct jdksavdecc_eui64 zero;
            bzero( &zero, sizeof( zero ) );
            if ( target_entity_id && jdksavdecc_eui64_compare( &zero, target_entity_id ) != 0 )
            {
                if ( jdksavdecc_eui64_compare( &adpdu->header.entity_id, target_entity_id ) == 0 )
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
