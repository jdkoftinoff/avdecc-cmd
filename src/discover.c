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
        // Only compare entity_model_id if both sides have entity_model_id non zero and non FF:FF:FF:FF:FF:FF:FF:FF

        if ( jdksavdecc_eui64_is_not_zero( lhs->entity_model_id ) || jdksavdecc_eui64_is_unset( lhs->entity_model_id )
             || jdksavdecc_eui64_is_not_zero( rhs->entity_model_id ) || jdksavdecc_eui64_is_unset( rhs->entity_model_id ) )
        {
            r = jdksavdecc_eui64_compare( &lhs->entity_model_id, &rhs->entity_model_id );
        }
    }
    return r;
}

int discovered_entity_compare_indirect( const void *lhs_, const void *rhs_ )
{
    int r = 0;
    const struct discovered_entity **lhs = (const struct discovered_entity **)lhs_;
    const struct discovered_entity **rhs = (const struct discovered_entity **)rhs_;

    if ( !lhs )
    {
        r = 1;
    }
    else if ( !rhs )
    {
        r = -1;
    }
    else
    {
        r = discovered_entity_compare( *lhs, *rhs );
    }
    return r;
}

bool discover_init( struct discover *self,
                    struct jdksavdecc_eui64 controller_entity_id,
                    size_t max_items,
                    struct raw_context *network,
                    void *additional_data,
                    void ( *discovered_entity_callback )( struct discover *self, const struct discovered_entity *entity ),
                    void ( *removed_entity_callback )( struct discover *self, const struct discovered_entity *entity ),
                    ssize_t ( *raw_send )( struct raw_context *self, const struct jdksavdecc_frame *frame ) )
{
    bool r = false;

    self->items = (struct discovered_entity **)calloc( max_items, sizeof( struct discovered_entity * ) );

    if ( self->items )
    {
        self->controller_entity_id = controller_entity_id;
        self->num_items = 0;
        self->max_items = max_items;
        self->network = network;
        self->additional_data = additional_data;
        self->discovered_entity_callback = discovered_entity_callback;
        self->removed_entity_callback = removed_entity_callback;
        self->last_tick_time = 0;
        self->request_do_discover = true;
        self->raw_send = raw_send;
        r = true;
    }
    return r;
}

bool discover_resize( struct discover *self, size_t new_max_items )
{
    bool r = false;

    if ( self->num_items < new_max_items )
    {
        struct discovered_entity **new_items = 0;
        new_items = (struct discovered_entity **)realloc( self->items, new_max_items * sizeof( struct discovered_entity * ) );
        if ( new_items )
        {
            self->items = new_items;
            self->max_items = new_max_items;
            r = true;
        }
    }
    return r;
}

void discover_free( struct discover *self )
{
    discover_clear( self );

    if ( self->items )
    {
        free( self->items );
        self->items = 0;
    }
    if ( self->additional_data )
    {
        free( self->additional_data );
        self->additional_data = 0;
    }
}

void discover_clear( struct discover *self )
{
    size_t i;
    for ( i = 0; i < self->num_items; ++i )
    {
        struct discovered_entity *entity = self->items[i];
        if ( entity )
        {
            if ( self->removed_entity_callback )
            {
                self->removed_entity_callback( self, entity );
            }
            discovered_entity_free( entity );
            free( entity );
            self->items[i] = 0;
        }
    }
    self->num_items = 0;
}

bool discover_is_full( struct discover *self ) { return self->num_items == self->max_items; }

struct discovered_entity **discover_insert( struct discover *self,
                                            struct jdksavdecc_eui48 mac_address,
                                            const struct jdksavdecc_adpdu *most_recent_adpdu,
                                            jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds,
                                            void *data )
{
    bool r = true;
    struct discovered_entity **entity_ptr = 0;

    if ( !discover_is_full( self ) )
    {
        // We need to enlarge our area
        size_t new_max_items = self->max_items;
        r = discover_resize( self, new_max_items * 3 / 2 );
    }

    // Make sure resize worked
    if ( r )
    {
        // Allocate one item
        entity_ptr = &self->items[self->num_items];

        *entity_ptr = calloc( 1, sizeof( struct discovered_entity ) );

        // Did the allocate work?
        if ( *entity_ptr )
        {
            // Yes, increase the number of items
            ++self->num_items;

            // Initialize the item
            discovered_entity_init( *entity_ptr,
                                    most_recent_adpdu->header.entity_id,
                                    most_recent_adpdu->entity_model_id,
                                    mac_address,
                                    most_recent_adpdu,
                                    current_time_in_milliseconds,
                                    data );

            // and sort the whole set so that they remain in order
            discover_sort( self );
        }
        else
        {
            // No, make sure we return 0
            entity_ptr = 0;
        }
    }

    return entity_ptr;
}

int discover_process_incoming( const void *self_, struct raw_context *net, const struct jdksavdecc_frame *frame )
{
    struct discover *self = (struct discover *)self_;
    struct discovered_entity **entity_ptr = 0;
    (void)net;

    if ( frame->ethertype == JDKSAVDECC_AVTP_ETHERTYPE )
    {
        struct jdksavdecc_adpdu adpdu;
        ssize_t pos = jdksavdecc_adpdu_read( &adpdu, frame->payload, 0, sizeof( frame->payload ) );
        if ( pos > 0 )
        {
            if ( adpdu.header.message_type == JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_AVAILABLE )
            {
                // This is an entity available message
                bool notify_new_info = false;

                // do we know about this specific entity_id with this entity_model_id ?

                entity_ptr = discover_find_with_model( self, adpdu.header.entity_id, adpdu.entity_model_id );

                // We know about it
                if ( entity_ptr )
                {
                    // Check to see if the available_index went backwards, indicating the
                    // entity rebooted

                    if ( adpdu.available_index < ( *entity_ptr )->most_recent_adpdu.available_index )
                    {
                        // it rebooted, so notify the upper layer that the original entity went away
                        if ( self->removed_entity_callback )
                        {
                            self->removed_entity_callback( self, *entity_ptr );
                        }

                        // and that it came back
                        notify_new_info = true;
                    }

                    // Check to see if something else interesting changed with this entity

                    if ( ( adpdu.gptp_domain_number != ( *entity_ptr )->most_recent_adpdu.gptp_domain_number )
                         || ( jdksavdecc_eui64_compare( &adpdu.gptp_grandmaster_id,
                                                        &( *entity_ptr )->most_recent_adpdu.gptp_grandmaster_id ) != 0 ) )
                    {
                        // The gptp grandmaster changed
                        notify_new_info = true;
                    }
                }
                else
                {
                    // Remove any stale entities in the list that have the same entity_id
                    // but a different entity_model_id before adding the new entry
                    discover_remove_entity( self, adpdu.header.entity_id );

                    // try insert the new entity info
                    entity_ptr = discover_insert( self, frame->src_address, &adpdu, frame->time / 1000, 0 );

                    if ( entity_ptr )
                    {
                        // The insert worked, mark that we need to notify the upper layer
                        notify_new_info = true;
                    }
                }

                // notify the upper layer that there is a new entity discovered
                if ( entity_ptr && notify_new_info && self->discovered_entity_callback )
                {
                    self->discovered_entity_callback( self, *entity_ptr );
                }
            }
            else if ( adpdu.header.message_type == JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DEPARTING )
            {
                entity_ptr = discover_find_with_model( self, adpdu.header.entity_id, adpdu.entity_model_id );

                if ( entity_ptr && self->removed_entity_callback )
                {
                    self->removed_entity_callback( self, *entity_ptr );
                }
            }
        }
    }

    return 0;
}

void discover_sort( struct discover *self )
{
    qsort( self->items, self->num_items, sizeof( self->items[0] ), discovered_entity_compare_indirect );
}

void discover_removed_expired( struct discover *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds )
{
    int num_removed = 0;
    size_t i;
    for ( i = 0; i < self->num_items; ++i )
    {
        struct discovered_entity *entity = self->items[i];
        if ( entity )
        {
            jdksavdecc_timestamp_in_milliseconds age = current_time_in_milliseconds
                                                       - entity->time_of_last_adpdu_in_milliseconds;
            jdksavdecc_timestamp_in_milliseconds max_age_in_milliseconds = entity->most_recent_adpdu.header.valid_time * 2000;
            if ( max_age_in_milliseconds == 0 )
            {
                max_age_in_milliseconds = 62000;
            }
            if ( age > max_age_in_milliseconds )
            {
                if ( self->removed_entity_callback )
                {
                    self->removed_entity_callback( self, entity );
                }
                discovered_entity_free( entity );
                free( entity );
                self->items[i] = 0;
                ++num_removed;
            }
        }
    }
    if ( num_removed > 0 )
    {
        discover_sort( self );
    }
}

struct discovered_entity **discover_find( struct discover *self, struct jdksavdecc_eui64 entity_id )
{
    struct discovered_entity **rp = 0;

    struct discovered_entity key;
    struct discovered_entity *keyp = &key;

    bzero( &key, sizeof( key ) );
    key.entity_id = entity_id;
    jdksavdecc_eui64_init( &key.entity_model_id );

    rp = (struct discovered_entity **)bsearch(
        &keyp, self->items, self->num_items, sizeof( self->items[0] ), discovered_entity_compare_indirect );
    return rp;
}

struct discovered_entity **discover_find_with_model( struct discover *self,
                                                     struct jdksavdecc_eui64 entity_id,
                                                     struct jdksavdecc_eui64 entity_model_id )
{
    struct discovered_entity **rp = 0;

    struct discovered_entity key;
    struct discovered_entity *keyp = &key;

    bzero( &key, sizeof( key ) );
    key.entity_id = entity_id;
    key.entity_model_id = entity_model_id;

    rp = (struct discovered_entity **)bsearch(
        &keyp, self->items, self->num_items, sizeof( self->items[0] ), discovered_entity_compare_indirect );
    return rp;
}

void discover_remove_entity( struct discover *self, struct jdksavdecc_eui64 entity_id )
{
    int num_removed = 0;
    struct discovered_entity **entity_ptr;

    while ( ( entity_ptr = discover_find( self, entity_id ) ) != 0 )
    {
        if ( self->removed_entity_callback )
        {
            self->removed_entity_callback( self, *entity_ptr );
        }
        discovered_entity_free( *entity_ptr );
        free( *entity_ptr );
        *entity_ptr = 0;
        ++num_removed;
    }
    if ( num_removed > 0 )
    {
        discover_sort( self );
    }
}

void discover_remove_with_model_id( struct discover *self,
                                    struct jdksavdecc_eui64 entity_id,
                                    struct jdksavdecc_eui64 entity_model_id )
{
    int num_removed = 0;
    struct discovered_entity **entity_ptr;

    while ( ( entity_ptr = discover_find_with_model( self, entity_id, entity_model_id ) ) != 0 )
    {
        if ( self->removed_entity_callback )
        {
            self->removed_entity_callback( self, *entity_ptr );
        }
        discovered_entity_free( *entity_ptr );
        free( *entity_ptr );
        *entity_ptr = 0;
        ++num_removed;
    }
    if ( num_removed > 0 )
    {
        discover_sort( self );
    }
}

void discover_tick( struct discover *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds )
{
    jdksavdecc_timestamp_in_milliseconds time_since_last_tick = current_time_in_milliseconds - self->last_tick_time;

    // Make sure work is only done at the most once per second
    if ( time_since_last_tick > 1000 )
    {
        // remember the current time
        self->last_tick_time = current_time_in_milliseconds;

        // remove any expired entities
        discover_removed_expired( self, current_time_in_milliseconds );

        // And send a discover message if there was a request to do so
        if ( self->request_do_discover )
        {
            discover_send_discover( self );

            // and clear the request to send a discover
            self->request_do_discover = false;
        }
    }
}

void discover_send_discover( struct discover *self )
{
    struct jdksavdecc_frame frame;
    struct jdksavdecc_adpdu adpdu;
    struct jdksavdecc_eui64 target_entity_id;

    // target entity id of zero means discover all
    jdksavdecc_eui64_zero( &target_entity_id );

    // form the ADPDU for ENTITY_DISCOVER
    if ( adp_form_msg( &frame, &adpdu, JDKSAVDECC_ADP_MESSAGE_TYPE_ENTITY_DISCOVER, target_entity_id ) == 0 )
    {
        // and send it to the raw network port
        self->raw_send( self->network, &frame );
    }
}
