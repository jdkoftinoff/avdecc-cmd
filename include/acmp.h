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

#include "jdksavdecc_acmp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief acmp_form_msg
 *
 * Create an ACMPDU message ethernet frame with the specified message_type,
 * possibly directed to the specified target_entity (as ascii)
 *
 * @param frame Pointer to the ethernet frame that will be filled in except for SA
 * @param acmpdu Pointer to the ACMPDU data structure that will be filled in with the high level representation
 * @param message_type the message type
 * @param sequence_id sequence id
 * @param talker_entity_id talker's entity id
 * @param talker_unique_id talker's unique id
 * @param listener_entity_id listener's entity id
 * @param listener_unique_id listener's unique id
 * @param target_entity Pointer to ascii string of the target entity id to use, or 0 for none
 * @return 0 success
 */
int acmp_form_msg( struct jdksavdecc_frame *frame,
                   struct jdksavdecc_acmpdu *acmpdu,
                   uint16_t message_type,
                   uint16_t sequence_id,
                   struct jdksavdecc_eui64 talker_entity_id,
                   uint16_t talker_unique_id,
                   struct jdksavdecc_eui64 listener_entity_id,
                   uint16_t listener_unique_id,
                   uint16_t connection_count );

/**
 * @brief acmp_check_listener
 *
 * Validate an ethernet frame to see if it contains an acmp message, potentially from the target entity
 *
 * @param frame The ethernet frame to validate
 * @param acmpdu The ACMPDU structure that will be filled in if the frame is matching
 * @param target_entity_id The target entity_id to expect, or 0 for any
 * @return 0 on success
 */
int acmp_check_listener( const struct jdksavdecc_frame *frame,
                         struct jdksavdecc_acmpdu *acmpdu,
                         const struct jdksavdecc_eui64 *controller_entity_id,
                         uint16_t sequence_id,
                         const struct jdksavdecc_eui64 *listener_entity_id,
                         uint16_t listener_unique_id );

#ifdef __cplusplus
}
#endif
