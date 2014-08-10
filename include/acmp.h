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
#include "jdksavdecc_acmp.h"
#include "jdksavdecc_acmp_print.h"

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
 * @param message_type Pointer to ascii string for the message type
 * @param sequence_id Pointer to ascii string for sequence id
 * @param talker_entity_id Pointer to ascii string for talker's entity id
 * @param talker_unique_id Pointer to ascii string for talker's unique id
 * @param listener_entity_id Pointer to ascii string for listener's entity id
 * @param listener_unique_id Pointer to ascii string for listener's unique id
 * @param target_entity Pointer to ascii string of the target entity id to use, or 0 for none
 * @return 0 success
 */
int acmp_form_msg( struct jdksavdecc_frame *frame,
                   const char *arg_message_type,
                   const char *sequence_id,
                   const char *talker_entity_id,
                   const char *talker_unique_id,
                   const char *listener_entity_id,
                   const char *listener_unique_id );

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

/**
 * @brief acmp_print
 *
 * Print the details of the acmp message in the ethernet frame
 *
 * @param s The output stream to print the ascii to
 * @param frame The ethernet frame which contains an acmp message
 * @param acmpdu The parsed acmp message
 */
void acmp_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_acmpdu *acmpdu );

/**
 * @brief acmp_process
 * @param request_
 * @param net
 * @param frame
 * @return
 */
int acmp_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame );

/**
 * @brief handle acmp command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param verbose 1 for verbose information about operations done
 * @param time_ms_to_wait time in milliseconds to wait for responses, or 0 for none
 * @param argc count of arguments including "acmp"
 * @param argv array of arguments starting at "acmp"
 * @return 0 on success
 */
int acmp( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv );

#ifdef __cplusplus
}
#endif
