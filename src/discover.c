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

#include "avdecc-cmd.h"
#include "discover.h"

void discovered_entity_init( struct discovered_entity *self,
                             struct jdksavdecc_eui64 entity_id,
                             struct jdksavdecc_eui64 entity_model_id,
                             struct jdksavdecc_eui48 mac_address,
                             const struct jdksavdecc_adpdu *most_recent_adpdu,
                             jdksavdecc_timestamp_in_milliseconds time_of_last_adpdu_in_milliseconds,
                             void *data )
{
    self->entity_id = entity_id;
    self->entity_model_id = entity_model_id;
    self->mac_address = mac_address;
    self->most_recent_adpdu = *most_recent_adpdu;
    self->time_of_last_adpdu_in_milliseconds = time_of_last_adpdu_in_milliseconds;
    self->data = data;
}

void discovered_entity_free( struct discovered_entity *self )
{
    if ( self->data )
    {
        free( self->data );
        self->data = 0;
    }
}

int discovered_entity_compare( const struct discovered_entity *lhs, const struct discovered_entity *rhs )
{
    int r;

    r = jdksavdecc_eui64_compare( &lhs->entity_id, &rhs->entity_id );

    if ( r == 0 )
    {
        r = jdksavdecc_eui64_compare( &lhs->entity_model_id, &rhs->entity_model_id );
    }
    return r;
}
