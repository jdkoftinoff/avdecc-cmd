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

struct entitycontext
{
    struct discovered_entity *entity;
    struct descriptors *descriptors;
    uint16_t current_sequence_id;

    void ( *state )( struct entitycontext *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );
    jdksavdecc_timestamp_in_milliseconds last_request_sent_time;
    void *data;
};

int entitycontext_init( struct entitycontext *self, struct discovered_entity *entity );

void entitycontext_free( struct entitycontext *self );

void entitycontext_tick( struct entitycontext *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming( void *self, struct raw_context *net, const struct jdksavdecc_frame *frame );

int entitycontext_process_incoming_controller_available( struct entitycontext *self,
                                                         struct raw_context *net,
                                                         const struct jdksavdecc_frame *frame );

int entitycontext_process_incoming_unsolicited_control_descriptor( struct entitycontext *self,
                                                                   struct raw_context *net,
                                                                   const struct jdksavdecc_frame *frame );

void entitycontextstate_wait( struct entitycontext *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontextstate_waiting( struct entitycontext *self,
                                 jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontextstate_send_register_unsolicited( struct entitycontext *self,
                                                   jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontextstate_sent_register_unsolicited( struct entitycontext *self,
                                                   jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_register_unsolicited( struct entitycontext *self,
                                                         struct raw_context *net,
                                                         const struct jdksavdecc_frame *frame );

void entitycontextstate_send_read_entity_descriptor( struct entitycontext *self,
                                                     jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontextstate_sent_readentitydescriptor( struct entitycontext *self,
                                                   jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_read_entity_descriptor( struct entitycontext *self,
                                                           struct raw_context *net,
                                                           const struct jdksavdecc_frame *frame );

void entitycontextstate_send_read_configuration_descriptor( struct entitycontext *self,
                                                            jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontextstate_sent_read_configuration_descriptor( struct entitycontext *self,
                                                            jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_read_configuration_descriptor( struct entitycontext *self,
                                                                  struct raw_context *net,
                                                                  const struct jdksavdecc_frame *frame );

void entitycontextstate_send_lazy_read_control_descriptor( struct entitycontext *self,
                                                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds,
                                                           uint16_t descriptor_type,
                                                           uint16_t descriptor_index );

void entitycontextstate_sent_lazy_read_control_descriptor( struct entitycontext *self,
                                                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_read_control_descriptor( struct entitycontext *self,
                                                            struct raw_context *net,
                                                            const struct jdksavdecc_frame *frame );

void entitycontextstate_send_lazy_read_strings_descriptor( struct entitycontext *self,
                                                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

void entitycontextstate_sent_lazy_read_strings_descriptor( struct entitycontext *self,
                                                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

int entitycontext_process_incoming_read_strings_descriptor( struct entitycontext *self,
                                                            struct raw_context *net,
                                                            const struct jdksavdecc_frame *frame );

#ifdef __cplusplus
}
#endif
