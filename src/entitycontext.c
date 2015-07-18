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

#include "entitycontext.h"


bool entitycontext_init( struct entitycontext *self, struct raw_context *net, struct jdksavdecc_eui64 controller_entity_id, struct discovered_entity *entity )
{
    bool r=false;
    self->net = net;
    self->entity = entity;
    self->descriptors = (struct descriptors *)calloc(1,sizeof(struct descriptors));
    if( descriptors_init(self->descriptors,ENITYCONTEXT_MAX_DESCRIPTORS) )
    {
        self->current_sequence_id = 0;
        self->current_state_tick = 0;
        self->current_state_process_incoming = 0;
        self->last_request_sent_time = 0;
        self->controller_entity_id = controller_entity_id;
        self->data = 0;
        r=true;
    }
    else
    {
        free(self->descriptors);
        r=false;
    }
    return r;
}

void entitycontext_free( struct entitycontext *self )
{
    if( self->descriptors )
    {
        descriptors_free(self->descriptors);
        free(self->descriptors);
    }
    if( self->data )
    {
        free( self->data );
    }
}

void entitycontext_tick( struct entitycontext *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds )
{
    if( self->current_state_tick )
    {
        self->current_state_tick(self,current_time_in_milliseconds);
    }
}

int entitycontext_process_incoming( void *self_, struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    int r=0;
    struct entitycontext *self = (struct entitycontext *)self_;

    if( frame->ethertype == JDKSAVDECC_AVTP_ETHERTYPE )
    {
        if( frame->payload[0] == JDKSAVDECC_1722A_SUBTYPE_AECP )
        {
            struct jdksavdecc_aecpdu_aem aem;
            ssize_t pos = jdksavdecc_aecpdu_aem_read( &aem, frame->payload, 0, frame->length );
            if( pos>0 )
            {
                if( jdksavdecc_eui64_compare( &aem.aecpdu_header.controller_entity_id, &self->controller_entity_id )==0 )
                {
                    if( jdksavdecc_eui64_compare( &aem.aecpdu_header.header.target_entity_id, &self->entity->entity_id )==0 )
                    {
                        if( aem.aecpdu_header.header.message_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND ||
                                aem.aecpdu_header.header.message_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE )
                        {
                            if( (aem.command_type & 0x8000) != 0 )
                            {
                                /* Unsolicited message */
                                r=entitycontext_process_incoming_unsolicited(self,net,frame,&aem);
                            }
                            else if( aem.aecpdu_header.header.message_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_COMMAND
                                     && aem.command_type == JDKSAVDECC_AEM_COMMAND_CONTROLLER_AVAILABLE )
                            {
                                r=entitycontext_process_incoming_controller_available(self,net,frame,&aem);
                            }
                            else if( aem.aecpdu_header.header.message_type == JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE
                                     && aem.command_type == JDKSAVDECC_AEM_COMMAND_ENTITY_AVAILABLE )
                            {
                                r=entitycontext_process_incoming_entity_available(self,net,frame,&aem);
                            }
                            else if( self->current_state_process_incoming )
                            {
                                r=self->current_state_process_incoming(self,net,frame,&aem);
                            }
                        }
                    }
                }
            }
        }
    }
    return r;
}

int entitycontext_process_incoming_controller_available( struct entitycontext *self,
                                                         struct raw_context *net,
                                                         const struct jdksavdecc_frame *frame,
                                                         const struct jdksavdecc_aecpdu_aem *aem )
{
    struct jdksavdecc_aecpdu_aem response;
    uint8_t payload[256];
    ssize_t payload_len;

    memset(&payload,0,sizeof(payload));
    response=*aem;
    response.aecpdu_header.header.message_type = JDKSAVDECC_AECP_MESSAGE_TYPE_AEM_RESPONSE;
    payload_len = jdksavdecc_aecpdu_aem_write(&response,payload,0,sizeof(payload));

    raw_send(net,frame->src_address.value,payload,payload_len);
    return 0;
}

int entitycontext_process_incoming_entity_available( struct entitycontext *self,
                                                         struct raw_context *net,
                                                         const struct jdksavdecc_frame *frame,
                                                         const struct jdksavdecc_aecpdu_aem *aem )
{
    return 0;
}

int entitycontext_process_incoming_unsolicited( struct entitycontext *self,
                                                struct raw_context *net,
                                                const struct jdksavdecc_frame *frame,
                                                const struct jdksavdecc_aecpdu_aem *aem  )
{
    uint16_t command = aem->command_type&0x7fff;

    if( command == JDKSAVDECC_AEM_COMMAND_SET_CONTROL
        || command == JDKSAVDECC_AEM_COMMAND_GET_CONTROL )
    {
        struct jdksavdecc_aem_command_set_control_response msg;
        ssize_t pos;
        pos = jdksavdecc_aem_command_set_control_response_read(&msg,frame->payload,0,frame->length);
        if( pos >0 )
        {
        /* TODO:
         *
         * Check if associated descriptors are loaded. If not, request them to be loaded lazily.
         *
         * If the descriptors are loaded, then forward the control values and control descriptor to the
         * handling function which can then either validate or print and log
         *
         */
        }
    }

    return 0;
}

void entitycontext_do_wait( struct entitycontext *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds )
{
    self->current_state_tick = entitycontext_state_waiting;
    self->current_state_process_incoming = 0;
}

void entitycontext_state_waiting( struct entitycontext *self,
                                 jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds )
{

}

void entitycontext_do_send_register_unsolicited( struct entitycontext *self,
                                                   jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds )
{

}

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
