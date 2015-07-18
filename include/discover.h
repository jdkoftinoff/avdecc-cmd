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

#include "adp.h"
#include "raw.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief The discovered_entity struct
 *
 * Contains the entity_id, entity_model_id, mac_address, most_recent_adpdu, and the timestamp
 * of when the most receive adpdu was received.
 *
 * Also contains a data pointer to additional allocated data
 */
struct discovered_entity
{
    struct jdksavdecc_eui64 entity_id;
    struct jdksavdecc_eui64 entity_model_id;
    struct jdksavdecc_eui48 mac_address;
    struct jdksavdecc_adpdu most_recent_adpdu;
    jdksavdecc_timestamp_in_milliseconds time_of_last_adpdu_in_milliseconds;
    void *data;
};

/**
 * @brief discovered_entity_init
 *
 * Initialize the discovered_entity structure
 *
 * @param self
 * @param entity_id
 * @param entity_model_id
 * @param mac_address
 * @param most_recent_adpdu
 * @param time_of_last_adpdu_in_milliseconds
 * @param data
 */
void discovered_entity_init( struct discovered_entity *self,
                             struct jdksavdecc_eui64 entity_id,
                             struct jdksavdecc_eui64 entity_model_id,
                             struct jdksavdecc_eui48 mac_address,
                             const struct jdksavdecc_adpdu *most_recent_adpdu,
                             jdksavdecc_timestamp_in_milliseconds time_of_last_adpdu_in_milliseconds,
                             void *data );

/**
 * @brief discovered_entity_free
 *
 * Free the discovered_entity data
 *
 * @param self
 */
void discovered_entity_free( struct discovered_entity *self );

/**
 * @brief discovered_entity_compare
 *
 * Compares only the entity_id and entity_model_id fields
 *
 * @param lhs
 * @param rhs
 * @return -1 if lhs < rhs, 0 if lhs == rhs, 1 if lhs > rhs
 */
int discovered_entity_compare( const struct discovered_entity *lhs, const struct discovered_entity *rhs );

int discovered_entity_compare_indirect( const void *lhs, const void *rhs );

/**
 * @brief The discover struct
 *
 * Manages the discovery state machine
 */
struct discover
{
    struct jdksavdecc_eui64 controller_entity_id;
    size_t num_items;
    size_t max_items;
    struct discovered_entity **items;
    struct raw_context *network;
    jdksavdecc_timestamp_in_milliseconds last_tick_time;
    bool request_do_discover;
    void *additional_data;

    void ( *discovered_entity_callback )( struct discover *self, struct discovered_entity *entity );
    void ( *removed_entity_callback )( struct discover *self, struct discovered_entity *entity );
    ssize_t ( *raw_send )( struct raw_context *self, const struct jdksavdecc_frame *frame );
};

/**
 * @brief discover_init
 * @param self
 * @param controller_entity_id
 * @param max_items
 * @param network
 * @param additional_data
 * @return
 */
bool discover_init( struct discover *self,
                    struct jdksavdecc_eui64 controller_entity_id,
                    size_t max_items,
                    struct raw_context *network,
                    void *additional_data,
                    void ( *discovered_entity_callback )( struct discover *self, struct discovered_entity *entity ),
                    void ( *removed_entity_callback )( struct discover *self, struct discovered_entity *entity ),
                    ssize_t ( *raw_send )( struct raw_context *self, const struct jdksavdecc_frame *frame ) );

/**
 * @brief discover_realloc
 * @param self
 * @param new_max_items
 * @return
 */
bool discover_resize( struct discover *self, size_t new_max_items );

/**
 * @brief discover_free
 * @param self
 */
void discover_free( struct discover *self );

/**
 * @brief discover_clear
 * @param self
 */
void discover_clear( struct discover *self );

/**
 * @brief discover_is_full
 * @param self
 * @return
 */
bool discover_is_full( struct discover *self );

/**
 * @brief discover_insert
 * @param self
 * @param mac_address
 * @param most_recent_adpdu
 * @param current_time_in_milliseconds
 * @param data
 * @return
 */
struct discovered_entity **discover_insert( struct discover *self,
                                            struct jdksavdecc_eui48 mac_address,
                                            const struct jdksavdecc_adpdu *most_recent_adpdu,
                                            jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds,
                                            void *data );

/**
 * @brief discover_remove_entity
 * @param self
 * @param entity_id
 */
void discover_remove_entity( struct discover *self, struct jdksavdecc_eui64 entity_id );

/**
 * @brief discover_remove_with_model_id
 * @param self
 * @param entity_id
 * @param entity_model_id
 */
void discover_remove_with_model_id( struct discover *self,
                                    struct jdksavdecc_eui64 entity_id,
                                    struct jdksavdecc_eui64 entity_model_id );

/**
 * @brief discover_process_incoming
 * @param self
 * @param net
 * @param frame
 * @return
 */
int discover_process_incoming( const void *self, struct raw_context *net, const struct jdksavdecc_frame *frame );

/**
 * @brief discover_sort
 * @param self
 */
void discover_sort( struct discover *self );

/**
 * @brief discover_removed_expired
 * @param self
 * @param current_time_in_milliseconds
 */
void discover_removed_expired( struct discover *self, jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds );

/**
 * @brief discover_find
 * @param self
 * @param entity_id
 */
struct discovered_entity **discover_find( struct discover *self, struct jdksavdecc_eui64 entity_id );

/**
 * @brief discover_find_with_model
 * @param self
 * @param entity_id
 * @param entity_model_id
 * @return
 */
struct discovered_entity **discover_find_with_model( struct discover *self,
                                                     struct jdksavdecc_eui64 entity_id,
                                                     struct jdksavdecc_eui64 entity_model_id );

/**
 * @brief discover_tick
 * @param self
 * @param current_time
 */
void discover_tick( struct discover *self, jdksavdecc_timestamp_in_milliseconds current_time );

/**
 * @brief discover_send_discover
 * @param self
 */
void discover_send_discover( struct discover *self );

#ifdef __cplusplus
}
#endif
