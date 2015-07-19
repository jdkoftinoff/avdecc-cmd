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

#include "descriptors.h"

void descriptor_key_init( struct descriptor_key *self,
                          struct jdksavdecc_eui64 entity_model_id,
                          struct jdksavdecc_eui64 entity_id,
                          uint16_t configuration_number,
                          uint16_t descriptor_type,
                          uint16_t descriptor_index )
{
    self->m_entity_model_id = entity_model_id;
    self->m_entity_id = entity_id;
    self->m_configuration_number = configuration_number;
    self->m_descriptor_type = descriptor_type;
    self->m_descriptor_index = descriptor_index;
}

int descriptor_key_compare( const struct descriptor_key *lhs, const struct descriptor_key *rhs )
{
    int r = 0;
    r = jdksavdecc_eui64_compare( &lhs->m_entity_model_id, &rhs->m_entity_model_id );
    if ( r == 0 )
    {
        r = jdksavdecc_eui64_compare( &lhs->m_entity_id, &rhs->m_entity_id );
        if ( r == 0 )
        {
            if ( lhs->m_configuration_number < rhs->m_configuration_number )
            {
                r = -1;
            }
            else if ( lhs->m_configuration_number > rhs->m_configuration_number )
            {
                r = 1;
            }
            else
            {
                if ( lhs->m_descriptor_type < rhs->m_descriptor_type )
                {
                    r = -1;
                }
                else if ( lhs->m_descriptor_type > rhs->m_descriptor_type )
                {
                    r = 1;
                }
                else
                {
                    if ( lhs->m_descriptor_index < rhs->m_descriptor_index )
                    {
                        r = -1;
                    }
                    else if ( lhs->m_descriptor_index > rhs->m_descriptor_index )
                    {
                        r = 1;
                    }
                    else
                    {
                        r = 0;
                    }
                }
            }
        }
    }
    return r;
}

int descriptor_key_compare_indirect( const void *lhs_, const void *rhs_ )
{
    const struct descriptor_key **lhs = (const struct descriptor_key **)lhs_;
    const struct descriptor_key **rhs = (const struct descriptor_key **)rhs_;
    int r = 0;
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
        r = descriptor_key_compare( *lhs, *rhs );
    }
    return r;
}

void descriptor_item_init( struct descriptor_item *self,
                           jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds,
                           struct jdksavdecc_eui64 entity_id,
                           struct jdksavdecc_eui64 entity_model_id,
                           uint16_t configuration_number,
                           uint16_t descriptor_type,
                           uint16_t descriptor_index,
                           uint16_t payload_length,
                           const void *payload_data,
                           void *additional_data )
{
    descriptor_key_init( &self->m_key, entity_model_id, entity_id, configuration_number, descriptor_type, descriptor_index );
    self->m_last_update_time_in_milliseconds = current_time_in_milliseconds;
    self->m_last_request_time_in_milliseconds = current_time_in_milliseconds;
    self->m_payload_length = payload_length;
    bzero( self->m_payload_data, sizeof( self->m_payload_data ) );
    memcpy( self->m_payload_data, payload_data, payload_length );
    self->m_additional_data = additional_data;
}

bool descriptors_init( struct descriptors *self, size_t max_entries )
{
    bool r = false;

    self->m_max_entries = max_entries;
    self->m_num_entries = 0;
    self->m_storage = (struct descriptor_item **)calloc( max_entries, sizeof( struct descriptor_item * ) );
    if ( self->m_storage != 0 )
    {
        r = true;
    }
    return r;
}

bool descriptors_resize( struct descriptors *self, size_t new_max_entries )
{
    bool r = false;

    if ( self->m_num_entries < new_max_entries )
    {
        struct descriptor_item **new_storage = 0;
        if ( self->m_needs_sort )
        {
            descriptors_sort( self );
        }

        new_storage = realloc( self->m_storage, new_max_entries * sizeof( struct descriptor_item * ) );
        if ( new_storage )
        {
            self->m_storage = new_storage;
            self->m_max_entries = new_max_entries;
            r = true;
        }
    }
    return r;
}

void descriptors_free( struct descriptors *self )
{
    if ( self->m_storage )
    {
        descriptors_clear( self );
        free( self->m_storage );
        self->m_storage = 0;
    }
}

void descriptors_clear( struct descriptors *self )
{
    size_t i;
    for ( i = 0; i < self->m_num_entries; ++i )
    {
        struct descriptor_item *item = self->m_storage[i];
        if ( item )
        {
            if ( jdksavdecc_eui64_is_set( item->m_key.m_entity_id ) )
            {
                jdksavdecc_eui64_init( &item->m_key.m_entity_id );
            }
            if ( item->m_additional_data )
            {
                free( item->m_additional_data );
                item->m_additional_data = 0;
            }
            item->m_payload_length = 0;
            free( item );
            self->m_storage[i] = 0;
        }
    }
    self->m_num_entries = 0;
}

void descriptors_clear_entity_id( struct descriptors *self, struct jdksavdecc_eui64 entity_id )
{
    size_t i;
    size_t new_item_count = self->m_num_entries;
    struct descriptor_item *item = 0;

    if ( self->m_needs_sort )
    {
        descriptors_sort( self );
    }

    /* find the first item */
    for ( i = 0; i < self->m_num_entries; ++i )
    {
        item = self->m_storage[i];

        if ( item && jdksavdecc_eui64_compare( &entity_id, &item->m_key.m_entity_id ) == 0 )
        {
            break;
        }
    }

    /* clear all until the last item for this entity_id */
    if ( item && jdksavdecc_eui64_compare( &entity_id, &item->m_key.m_entity_id ) == 0 )
    {
        for ( ; i < self->m_num_entries; ++i )
        {
            item = self->m_storage[i];

            if ( item )
            {
                if ( jdksavdecc_eui64_compare( &entity_id, &item->m_key.m_entity_id ) != 0 )
                {
                    break;
                }
                if ( item->m_additional_data )
                {
                    free( item->m_additional_data );
                    item->m_additional_data = 0;
                }
                item->m_payload_length = 0;
                free( item );
                self->m_storage[i] = 0;
                --new_item_count;
            }
            else
            {
                break;
            }
        }
    }

    if ( new_item_count != self->m_num_entries )
    {
        self->m_needs_sort = true;
        descriptors_sort( self );
        self->m_num_entries = new_item_count;
    }
}

void descriptors_clear_expired( struct descriptors *self,
                                jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds,
                                jdksavdecc_timestamp_in_milliseconds max_age_in_milliseconds )
{
    size_t i;
    size_t new_item_count = self->m_num_entries;
    struct descriptor_item *item = 0;

    for ( i = 0; i < self->m_num_entries; ++i )
    {
        item = self->m_storage[i];

        if ( item )
        {
            if ( ( current_time_in_milliseconds - item->m_last_update_time_in_milliseconds ) > max_age_in_milliseconds )
            {
                // the descriptor information has expired, remove it
                if ( item->m_additional_data )
                {
                    free( item->m_additional_data );
                    item->m_additional_data = 0;
                }
                item->m_payload_length = 0;
                free( item );
                self->m_storage[i] = 0;
                --new_item_count;
            }
        }
    }

    if ( new_item_count != self->m_num_entries )
    {
        self->m_needs_sort = true;
        descriptors_sort( self );
        self->m_num_entries = new_item_count;
    }
}

bool descriptors_is_full( struct descriptors *self ) { return self->m_num_entries == self->m_max_entries; }

bool descriptors_insert( struct descriptors *self,
                         jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds,
                         struct jdksavdecc_eui64 entity_model_id,
                         struct jdksavdecc_eui64 entity_id,
                         uint16_t configuration_number,
                         uint16_t descriptor_type,
                         uint16_t descriptor_index,
                         uint16_t payload_length,
                         const void *payload_data,
                         void *additional_data )
{
    bool r = false;
    struct descriptor_item *item;

    /* see if the descriptor is already known */
    item = descriptors_find( self, entity_model_id, entity_id, configuration_number, descriptor_type, descriptor_index );

    if ( !item && descriptors_is_full( self ) )
    {
        // It is not already known, AND the storage is full
        // Try grow the storage area by 50 %
        descriptors_resize( self, self->m_max_entries * 3 / 2 );
    }

    if ( !item && !descriptors_is_full( self ) )
    {
        // It is not already known and the storage is available

        // allocate space for the item
        item = (struct descriptor_item *)calloc( 1, sizeof( struct descriptor_item ) );

        if ( item )
        {
            // Allocation succeeded, initialize the item
            descriptor_item_init( item,
                                  current_time_in_milliseconds,
                                  entity_model_id,
                                  entity_id,
                                  configuration_number,
                                  descriptor_type,
                                  descriptor_index,
                                  payload_length,
                                  payload_data,
                                  additional_data );
            self->m_storage[self->m_num_entries] = item;

            ++self->m_num_entries;

            // The storage now needs sorting
            self->m_needs_sort = true;
            r = true;
        }
    }
    else if ( item )
    {
        // the item already exists, so we can just update it in place
        item->m_last_update_time_in_milliseconds = current_time_in_milliseconds;
        item->m_last_request_time_in_milliseconds = current_time_in_milliseconds;
        memcpy( item->m_payload_data, payload_data, payload_length );
        item->m_payload_length = payload_length;
        if ( item->m_additional_data )
        {
            // free any old additional_data
            free( item->m_additional_data );
        }
        item->m_additional_data = additional_data;
        r = true;
    }

    if ( !r )
    {
        // we were unable to insert
        if ( additional_data )
        {
            // free the passed in additional_data before returning false
            free( additional_data );
        }
    }
    return r;
}

void descriptors_sort( struct descriptors *self )
{
    if ( self->m_needs_sort )
    {
        qsort( self->m_storage, self->m_num_entries, sizeof( struct descriptor_item * ), descriptor_key_compare_indirect );
        self->m_needs_sort = false;
    }
}

struct descriptor_item *descriptors_find( struct descriptors *self,
                                          struct jdksavdecc_eui64 entity_model_id,
                                          struct jdksavdecc_eui64 entity_id,
                                          uint16_t configuration_number,
                                          uint16_t descriptor_type,
                                          uint16_t descriptor_index )
{
    struct descriptor_item *item;
    struct descriptor_key key;

    descriptors_sort( self );

    descriptor_key_init( &key, entity_model_id, entity_id, configuration_number, descriptor_type, descriptor_index );
    item = bsearch(
        &key, self->m_storage, self->m_num_entries, sizeof( struct descriptor_item * ), descriptor_key_compare_indirect );
    return item;
}

void descriptors_dispatch_requests( struct descriptors *self,
                                    jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds,
                                    jdksavdecc_timestamp_in_milliseconds min_time_since_last_request_in_milliseconds,
                                    void *callback_data,
                                    bool ( *callback )( void *, struct descriptor_item * ) )
{
    size_t i;
    struct descriptor_item *item = 0;

    for ( i = 0; i < self->m_num_entries; ++i )
    {
        item = self->m_storage[i];

        if ( item )
        {
            if ( ( current_time_in_milliseconds - item->m_last_request_time_in_milliseconds )
                 > min_time_since_last_request_in_milliseconds )
            {
                if ( callback( callback_data, item ) )
                {
                    item->m_last_request_time_in_milliseconds = current_time_in_milliseconds;
                }
                else
                {
                    break;
                }
            }
        }
    }
}
