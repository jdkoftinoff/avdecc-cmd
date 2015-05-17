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

#include "aecp.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The descriptor_key struct
 *
 * The key which defines the location of a descriptor. Based on the
 * following fields in order:
 *
 * @item entity_model_id
 * @item entity_id
 * @item configuration_number
 * @item descriptor_type
 * @item descriptor_index
 */
struct descriptor_key
{
    struct jdksavdecc_eui64 entity_model_id;
    struct jdksavdecc_eui64 entity_id;
    uint16_t configuration_number;
    uint16_t descriptor_type;
    uint16_t descriptor_index;
};

/**
 * @brief descriptor_key_init
 *
 * Initialize a descriptor_key structure
 *
 * @param self
 * @param entity_model_id
 * @param entity_id
 * @param configuration_number
 * @param descriptor_type
 * @param descriptor_index
 */
void descriptor_key_init( struct descriptor_key *self,
                          struct jdksavdecc_eui64 entity_model_id,
                          struct jdksavdecc_eui64 entity_id,
                          uint16_t configuration_number,
                          uint16_t descriptor_type,
                          uint16_t descriptor_index );

/**
 * @brief descriptor_key_compare
 *
 * Compare two descrriptor_keys via pointers
 *
 * @param lhs
 * @param rhs
 * @return -1 if *lhs < *rhs, 0 if *lhs == *rhs, or 1 if *lhs > *rhs
 */
int descriptor_key_compare( const struct descriptor_key *lhs, const struct descriptor_key *rhs );

/**
 * @brief descriptor_key_compare_indirect
 *
 * Compare two descriptor_keys via indirect pointers,
 * suitable for bsearch() and sort()
 *
 * @param lhs_
 * @param rhs_
 * @return -1 if **lhs < **rhs, 0 if **lhs == **rhs, or 1 if **lhs > **rhs
 */
int descriptor_key_compare_indirect( const void *lhs_, const void *rhs_ );

/**
 * @brief The descriptor_item struct
 *
 * The descriptor_item structure contains the key and associated data
 * for one descriptor.  The associated payload_data may contain
 * any data up to JDKSAVDECC_AECP_MAX_CONTROL_DATA_LENGTH octets in length,
 * and additional_data may be allocated via malloc/calloc if needed
 */
struct descriptor_item
{
    struct descriptor_key key;
    uint16_t payload_length;
    uint8_t payload_data[JDKSAVDECC_AECP_MAX_CONTROL_DATA_LENGTH];
    void *additional_data;
};

/**
 * @brief descriptor_item_init
 *
 * Initialize a descriptor_item structure.
 *
 * Copies the full payload_length of payload_data into descriptor_item.
 * Copies the additional_data pointer into the descriptor_item.
 *
 * @param self
 * @param entity_model_id
 * @param entity_id
 * @param configuration_number
 * @param descriptor_type
 * @param descriptor_index
 * @param payload_length
 * @param payload_data
 * @param additional_data
 */
void descriptor_item_init( struct descriptor_item *self,
                           struct jdksavdecc_eui64 entity_model_id,
                           struct jdksavdecc_eui64 entity_id,
                           uint16_t configuration_number,
                           uint16_t descriptor_type,
                           uint16_t descriptor_index,
                           uint16_t payload_length,
                           const void *payload_data,
                           void *additional_data );

/**
 * @brief The descriptors struct
 *
 * Effectively contains a map of descriptor_keys with the payload and additional data
 */
struct descriptors
{
    struct descriptor_item **storage;
    size_t num_entries;
    size_t max_entries;
    bool needs_sort;
};

/**
 * @brief descriptors_init
 *
 * Initializes a descriptors struct, allocating enough space for max_entries descriptors
 * and data
 *
 * @param self
 * @param max_entries
 * @return false if allocation fails
 */
bool descriptors_init( struct descriptors *self, size_t max_entries );

/**
 * @brief descriptors_resize
 *
 * Shrink or Enlarge the allocated space for a new max_entries size
 *
 * @param self
 * @param new_max_entries
 * @return false if allocation fails or if the new size is too small
 */
bool descriptors_resize( struct descriptors *self, size_t new_max_entries );

/**
 * @brief descriptors_free
 *
 * Free all data for the allocated descriptors and top array. Also frees
 * any additional_data for any descriptors
 *
 * @param self
 */
void descriptors_free( struct descriptors *self );

/**
 * @brief descriptors_clear
 *
 * Clear all descriptor entries and free them, set num_entries to 0
 *
 * @param self
 */
void descriptors_clear( struct descriptors *self );

/**
 * @brief descriptors_clear_entity_id
 *
 * Clear and free all descriptor entries associated with a specific entity_id
 *
 * @param self
 * @param entity_id
 */
void descriptors_clear_entity_id( struct descriptors *self, struct jdksavdecc_eui64 entity_id );

/**
 * @brief descriptors_is_full
 *
 * Return true if the descriptors structure is full
 *
 * @param self
 * @return true if full
 */
bool descriptors_is_full( struct descriptors *self );

/**
 * @brief descriptors_insert
 *
 * Insert the descriptor data into the descriptors structure.
 *
 * @param self
 * @param entity_model_id
 * @param entity_id
 * @param configuration_number
 * @param descriptor_type
 * @param descriptor_index
 * @param descriptor_data_length
 * @param descriptor_data
 * @param additional_data
 * @return true on success, false on memory allocation failure
 */
bool descriptors_insert( struct descriptors *self,
                         struct jdksavdecc_eui64 entity_model_id,
                         struct jdksavdecc_eui64 entity_id,
                         uint16_t configuration_number,
                         uint16_t descriptor_type,
                         uint16_t descriptor_index,
                         uint16_t descriptor_data_length,
                         const void *descriptor_data,
                         void *additional_data );

/**
 * @brief descriptors_sort
 *
 * Sort all descriptor key data to allow find/bsearch to function
 *
 * @param self
 */
void descriptors_sort( struct descriptors *self );

/**
 * @brief descriptors_find
 *
 * Find the descriptor data by entity_model_id, entity_id, configuration_number, descriptor_type,
 * and descriptor_index
 *
 * @param self
 * @param entity_model_id
 * @param entity_id
 * @param configuration_number
 * @param descriptor_type
 * @param descriptor_index
 * @return pointer to the found descriptor_item or 0 if not found
 */
struct descriptor_item *descriptors_find( struct descriptors *self,
                                          struct jdksavdecc_eui64 entity_model_id,
                                          struct jdksavdecc_eui64 entity_id,
                                          uint16_t configuration_number,
                                          uint16_t descriptor_type,
                                          uint16_t descriptor_index );

#ifdef __cplusplus
}
#endif
