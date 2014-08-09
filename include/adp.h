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

#include "avdecc-cmd.h"

#include "jdksavdecc_adp.h"
#include "jdksavdecc_adp_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief adp_form_msg
 *
 * Create an ADPDU message ethernet frame with the specified message_type,
 * possibly directed to the specified target_entity (as ascii)
 *
 * @param frame Pointer to the ethernet frame that will be filled in except for SA
 * @param message_type Pointer to ascii string "ENTITY_DISCOVER"
 * @param target_entity Pointer to ascii string of the target entity id to use, or 0 for none
 * @return 0 success
 */
int adp_form_msg( struct jdksavdecc_frame *frame, const char *message_type, const char *target_entity );

/**
 * @brief adp_check
 *
 * Validate an ethernet frame to see if it contains an ADP message, potentially from the target entity
 *
 * @param frame The ethernet frame to validate
 * @param adp The ADPDU structure that will be filled in if the frame is matching
 * @param target_entity The target entity_id to expect, or 0 for any
 * @return 0 on success
 */
int adp_check( const struct jdksavdecc_frame *frame, struct jdksavdecc_adpdu *adp, const char *target_entity );

/**
 * @brief adp_print
 *
 * Print the details of the ADP message in the ethernet frame
 *
 * @param s The output stream to print the ascii to
 * @param frame The ethernet frame which contains an ADP message
 * @param adp The parsed ADP message
 */
void adp_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_adpdu *adp );

#ifdef __cplusplus
}
#endif
