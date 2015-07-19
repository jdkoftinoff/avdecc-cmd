#pragma once
/*
Copyright (c) 2015, J.D. Koftinoff Software, Ltd.
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

#include "discover.h"
#include "raw.h"
#include "descriptors.h"
#include "aecp.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ENITYCONTEXT_MAX_DESCRIPTORS (4096)

/**
 * @brief The entitycontext struct
 *
 * Contains the communications context and state
 */
struct entitycontext
{
    struct raw_context *m_net;
    struct jdksavdecc_eui64 m_controller_entity_id;
    struct discovered_entity *m_entity;
    struct m_descriptors *m_descriptors;
    uint16_t m_current_sequence_id;

    void ( *current_state_tick )( struct entitycontext *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );
    int ( *current_state_process_incoming)( void *self,
                                            struct raw_context *m_net,
                                            const struct jdksavdecc_frame *frame,
                                            const struct jdksavdecc_aecpdu_aem *aem );
    jdksavdecc_timestamp_in_milliseconds m_last_request_sent_time;
    void *m_data;
};

bool entitycontext_init( struct entitycontext *self, struct raw_context *net, struct jdksavdecc_eui64 controller_entity_id, struct discovered_entity *entity );

void entitycontext_free( struct entitycontext *self );

void entitycontext_tick( struct entitycontext *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming( void *self, struct raw_context *net, const struct jdksavdecc_frame *frame );

int entitycontext_process_incoming_controller_available( struct entitycontext *self,
                                                         struct raw_context *net,
                                                         const struct jdksavdecc_frame *frame,
                                                         const struct jdksavdecc_aecpdu_aem *aem );

int entitycontext_process_incoming_entity_available( struct entitycontext *self,
                                                         struct raw_context *net,
                                                         const struct jdksavdecc_frame *frame,
                                                         const struct jdksavdecc_aecpdu_aem *aem );

int entitycontext_process_incoming_unsolicited( struct entitycontext *self,
                                                struct raw_context *net,
                                                const struct jdksavdecc_frame *frame,
                                                const struct jdksavdecc_aecpdu_aem *aem  );

void entitycontext_do_wait( struct entitycontext *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontext_state_waiting( struct entitycontext *self,
                                 jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontext_do_send_register_unsolicited( struct entitycontext *self,
                                                   jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontext_state_sent_register_unsolicited( struct entitycontext *self,
                                                   jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_register_unsolicited( struct entitycontext *self,
                                                         struct raw_context *net,
                                                         const struct jdksavdecc_frame *frame );

void entitycontext_do_send_read_entity_descriptor( struct entitycontext *self,
                                                     jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontext_state_sent_readentitydescriptor( struct entitycontext *self,
                                                   jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_read_entity_descriptor( struct entitycontext *self,
                                                           struct raw_context *net,
                                                           const struct jdksavdecc_frame *frame );

void entitycontext_do_send_read_configuration_descriptor( struct entitycontext *self,
                                                            jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontext_state_sent_read_configuration_descriptor( struct entitycontext *self,
                                                            jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_read_configuration_descriptor( struct entitycontext *self,
                                                                  struct raw_context *net,
                                                                  const struct jdksavdecc_frame *frame );

void entitycontext_do_send_lazy_read_control_descriptor( struct entitycontext *self,
                                                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds,
                                                           uint16_t descriptor_type,
                                                           uint16_t descriptor_index );

void entitycontext_state_sent_lazy_read_control_descriptor( struct entitycontext *self,
                                                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_read_control_descriptor( struct entitycontext *self,
                                                            struct raw_context *net,
                                                            const struct jdksavdecc_frame *frame );

void entitycontext_do_send_lazy_read_strings_descriptor( struct entitycontext *self,
                                                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontext_state_sent_lazy_read_strings_descriptor( struct entitycontext *self,
                                                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_read_strings_descriptor( struct entitycontext *self,
                                                            struct raw_context *net,
                                                            const struct jdksavdecc_frame *frame );

#ifdef __cplusplus
}
#endif
