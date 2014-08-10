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
#include "raw.h"
#include "jdksavdecc_aem_command.h"
#include "jdksavdecc_aem_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief aem_form_msg
 *
 * Create an jdksavdecc_aecpdu_aem message ethernet frame with the specified message_type,
 * possibly directed to the specified target_entity (as ascii)
 *
 * @param frame Pointer to the ethernet frame that will be filled in except for SA
 * @param message_type Pointer to ascii string for the message type
 * @param sequence_id Pointer to ascii string for sequence id
 * @return 0 success
 */
int aem_form_msg( struct jdksavdecc_frame *frame, const char *message_type, const char *sequence_id );

/**
 * @brief aem_check
 *
 * Validate an ethernet frame to see if it contains an aem message, potentially from the target entity
 *
 * @param frame The ethernet frame to validate
 * @param aemdu The aemDU structure that will be filled in if the frame is matching
 * @param target_entity_id The target entity_id to expect, or 0 for any
 * @return 0 on success
 */
int aem_check( const struct jdksavdecc_frame *frame,
               struct jdksavdecc_aecpdu_aem *aecpdu,
               const struct jdksavdecc_eui64 *controller_entity_id,
               uint16_t sequence_id );

/**
 * @brief aem_print
 *
 * Print the details of the aem message in the ethernet frame
 *
 * @param s The output stream to print the ascii to
 * @param frame The ethernet frame which contains an aem message
 * @param aemdu The parsed aem message
 */
void aem_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_aecpdu_aem *aemdu );

/**
 * @brief aem_process
 * @param request_
 * @param net
 * @param frame
 * @return
 */
int aem_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame );

/**
 * @brief handle aem command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param verbose 1 for verbose information about operations done
 * @param time_ms_to_wait time in milliseconds to wait for responses, or 0 for none
 * @param argc count of arguments including "aem"
 * @param argv array of arguments starting at "aem"
 * @return 0 on success
 */
int aem( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv );

#ifdef __cplusplus
}
#endif
