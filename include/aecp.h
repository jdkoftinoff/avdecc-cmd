#pragma once
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

#include "jdksavdecc_aecp.h"
#include "jdksavdecc_aecp_aa.h"
#include "jdksavdecc_aecp_aem.h"
#include "jdksavdecc_aecp_vendor.h"
#include "jdksavdecc_aecp_hdcp_apm.h"
#include "jdksavdecc_aecp_print.h"
#include "jdksavdecc_aem_command.h"
#include "jdksavdecc_aem_descriptor.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief aecp_aem_form_msg
 *
 * Create an jdksavdecc_aecpdu_aecpdu message ethernet frame with the specified message_type,
 * possibly directed to the specified target_entity (as ascii)
 *
 * @param frame Pointer to the ethernet frame that will be filled in except for SA
 * @param message_type Pointer to ascii string for the message type
 * @return 0 success
 */
int aecp_aem_form_msg( struct jdksavdecc_frame *frame,
                       struct jdksavdecc_aecpdu_aem *aemdu,
                       uint16_t message_type_code,
                       uint16_t command_code,
                       uint16_t sequence_id,
                       struct jdksavdecc_eui48 destination_mac,
                       struct jdksavdecc_eui64 target_entity_id,
                       const uint8_t *command_payload,
                       int command_payload_length );

int aecp_aem_form_read_descriptor_command( struct jdksavdecc_frame *frame,
                                           struct jdksavdecc_aem_command_read_descriptor *pdu,
                                           uint16_t sequence_id,
                                           struct jdksavdecc_eui48 destination_mac,
                                           struct jdksavdecc_eui64 target_entity_id,
                                           uint16_t descriptor_type,
                                           uint16_t descriptor_index );

int aecp_aem_form_get_control_command( struct jdksavdecc_frame *frame,
                                       struct jdksavdecc_aem_command_get_control *pdu,
                                       uint16_t sequence_id,
                                       struct jdksavdecc_eui48 destination_mac,
                                       struct jdksavdecc_eui64 target_entity_id,
                                       uint16_t descriptor_index );

int aecp_aem_form_set_control_command( struct jdksavdecc_frame *frame,
                                       struct jdksavdecc_aem_command_set_control *pdu,
                                       uint16_t sequence_id,
                                       struct jdksavdecc_eui48 destination_mac,
                                       struct jdksavdecc_eui64 target_entity_id,
                                       uint16_t descriptor_index,
                                       const uint8_t *control_data_payload,
                                       int control_data_payload_length );

/**
 * @brief aecp_aem_check
 *
 * Validate an ethernet frame to see if it contains an aecpdu message, potentially from the target entity
 *
 * @param frame The ethernet frame to validate
 * @param aecpdudu The aecpdu AEM structure that will be filled in if the frame is matching
 * @param controller_entity_id The target entity_id to expect
 * @param target_entity_id The target entity_id to expect, or 0 for any
 @ @param sequence_id The command sequence id to expect
 * @return 0 on success
 */
int aecp_aem_check( const struct jdksavdecc_frame *frame,
                    struct jdksavdecc_aecpdu_aem *aem,
                    const struct jdksavdecc_eui64 controller_entity_id,
                    const struct jdksavdecc_eui64 target_entity_id,
                    uint16_t sequence_id );

#ifdef __cplusplus
}
#endif
