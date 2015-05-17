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
    self->entity_model_id = entity_model_id;
    self->entity_id = entity_id;
    self->configuration_number = configuration_number;
    self->descriptor_type = descriptor_type;
    self->descriptor_index = descriptor_index;
}

int descriptor_key_compare( const struct descriptor_key *lhs, const struct descriptor_key *rhs )
{
    int r = 0;
    r = jdksavdecc_eui64_compare( &lhs->entity_model_id, &rhs->entity_model_id );
    if ( r == 0 )
    {
        r = jdksavdecc_eui64_compare( &lhs->entity_id, &rhs->entity_id );
        if ( r == 0 )
        {
            if ( lhs->configuration_number < rhs->configuration_number )
            {
                r = -1;
            }
            else if ( lhs->configuration_number > rhs->configuration_number )
            {
                r = 1;
            }
            else
            {
                if ( lhs->descriptor_type < rhs->descriptor_type )
                {
                    r = -1;
                }
                else if ( lhs->descriptor_type > rhs->descriptor_type )
                {
                    r = 1;
                }
                else
                {
                    if ( lhs->descriptor_index < rhs->descriptor_index )
                    {
                        r = -1;
                    }
                    else if ( lhs->descriptor_index > rhs->descriptor_index )
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
    return 0;
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
                           struct jdksavdecc_eui64 entity_id,
                           struct jdksavdecc_eui64 entity_model_id,
                           uint16_t configuration_number,
                           uint16_t descriptor_type,
                           uint16_t descriptor_index,
                           uint16_t payload_length,
                           const void *payload_data,
                           void *additional_data )
{
    descriptor_key_init( &self->key, entity_model_id, entity_id, configuration_number, descriptor_type, descriptor_index );
    self->payload_length = payload_length;
    bzero( self->payload_data, sizeof( self->payload_data ) );
    memcpy( self->payload_data, payload_data, payload_length );
    self->additional_data = additional_data;
}

bool descriptors_init( struct descriptors *self, size_t max_entries )
{
    bool r = false;

    self->max_entries = max_entries;
    self->num_entries = 0;
    self->storage = (struct descriptor_item **)calloc( max_entries, sizeof( struct descriptor_item * ) );
    if ( self->storage != 0 )
    {
        r = true;
    }
    return r;
}

bool descriptors_resize( struct descriptors *self, size_t new_max_entries )
{
    bool r = false;

    if ( self->num_entries < new_max_entries )
    {
        struct descriptor_item **new_storage = 0;
        if ( self->needs_sort )
        {
            descriptors_sort( self );
        }

        new_storage = realloc( self->storage, new_max_entries * sizeof( struct descriptor_item * ) );
        if ( new_storage )
        {
            self->storage = new_storage;
            self->max_entries = new_max_entries;
            r = true;
        }
    }
    return r;
}

void descriptors_free( struct descriptors *self )
{
    if ( self->storage )
    {
        descriptors_clear( self );
        free( self->storage );
        self->storage = 0;
    }
}

void descriptors_clear( struct descriptors *self )
{
    size_t i;
    for ( i = 0; i < self->num_entries; ++i )
    {
        struct descriptor_item *item = self->storage[i];
        if ( item )
        {
            if ( jdksavdecc_eui64_is_set( item->key.entity_id ) )
            {
                jdksavdecc_eui64_init( &item->key.entity_id );
            }
            if ( item->additional_data )
            {
                free( item->additional_data );
                item->additional_data = 0;
            }
            item->payload_length = 0;
            free( item );
            self->storage[i] = 0;
        }
    }
    self->num_entries = 0;
}

void descriptors_clear_entity_id( struct descriptors *self, struct jdksavdecc_eui64 entity_id )
{
    size_t i;
    size_t new_item_count = self->num_entries;
    struct descriptor_item *item = 0;

    if ( self->needs_sort )
    {
        descriptors_sort( self );
    }

    /* find the first item */
    for ( i = 0; i < self->num_entries; ++i )
    {
        item = self->storage[i];

        if ( item && jdksavdecc_eui64_compare( &entity_id, &item->key.entity_id ) == 0 )
        {
            break;
        }
    }

    /* clear all until the last item for this entity_id */
    if ( item && jdksavdecc_eui64_compare( &entity_id, &item->key.entity_id ) == 0 )
    {
        for ( ; i < self->num_entries; ++i )
        {
            item = self->storage[i];

            if ( item )
            {
                if ( jdksavdecc_eui64_compare( &entity_id, &item->key.entity_id ) != 0 )
                {
                    break;
                }
                if ( item->additional_data )
                {
                    free( item->additional_data );
                    item->additional_data = 0;
                }
                item->payload_length = 0;
                free( item );
                self->storage[i] = 0;
                --new_item_count;
            }
            else
            {
                break;
            }
        }
    }

    if ( new_item_count != self->num_entries )
    {
        descriptors_sort( self );
        self->num_entries = new_item_count;
    }
}

bool descriptors_is_full( struct descriptors *self ) { return self->num_entries == self->max_entries; }

bool descriptors_insert( struct descriptors *self,
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

    item = descriptors_find( self, entity_model_id, entity_id, configuration_number, descriptor_type, descriptor_index );

    if ( !item && !descriptors_is_full( self ) )
    {
        item = (struct descriptor_item *)calloc( 1, sizeof( struct descriptor_item ) );

        if ( item )
        {
            descriptor_item_init( item,
                                  entity_model_id,
                                  entity_id,
                                  configuration_number,
                                  descriptor_type,
                                  descriptor_index,
                                  payload_length,
                                  payload_data,
                                  additional_data );
            self->storage[self->num_entries] = item;
            ++self->num_entries;
            self->needs_sort = true;
            r = true;
        }
    }

    if ( !r )
    {
        if ( additional_data )
        {
            free( additional_data );
        }
    }
    return r;
}

void descriptors_sort( struct descriptors *self )
{
    qsort( self->storage, self->num_entries, sizeof( struct descriptor_item * ), descriptor_key_compare_indirect );
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

    if ( self->needs_sort )
    {
        descriptors_sort( self );
    }

    descriptor_key_init( &key, entity_model_id, entity_id, configuration_number, descriptor_type, descriptor_index );
    item = bsearch(
        &key, self->storage, self->num_entries, sizeof( struct descriptor_item * ), descriptor_key_compare_indirect );
    return item;
}
