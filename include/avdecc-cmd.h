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

#if defined( __linux__ )
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <errno.h>
#include <strings.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <linux/sockios.h>
#include <linux/if_packet.h>
#include <linux/if_ether.h>
#elif defined( __APPLE__ )
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#include <errno.h>
#include <strings.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <inttypes.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/select.h>
#include <net/if_dl.h>
#include <pcap.h>
#elif defined( _WIN32 )
#ifndef _WINSOCKAPI_
#define _WINSOCKAPI_
#endif
#include <Windows.h>
#include <iphlpapi.h>
#include <winsock2.h>
#include "jdksavdecc_ms.h"
#include <pcap.h>
#pragma comment( lib, "IPHLPAPI.lib" )
#pragma comment( lib, "wpcap.lib" )
#pragma comment( lib, "Ws2_32.lib" )
static inline void bzero(void *buf, size_t sz) { memset(buf, 0, sz); }
#endif

#include "jdksavdecc.h"
#include "jdksavdecc_util.h"
#include "jdksavdecc_frame.h"

#ifdef __cplusplus
extern "C" {
#endif

struct raw_context;

extern int arg_verbose;
extern int arg_time_in_ms_to_wait;
extern const char *arg_network_port;
extern const char *arg_protocol;
extern const char *arg_message_type;
extern const char *arg_sequence_id;
extern const char *arg_destination_mac;
extern const char *arg_entity_id;
extern const char *arg_talker_entity_id;
extern const char *arg_talker_unique_id;
extern const char *arg_listener_entity_id;
extern const char *arg_listener_unique_id;
extern const char *arg_connection_count;
extern const char *arg_target_entity_id;
extern const char *arg_command;
extern const char *arg_descriptor_type;
extern const char *arg_descriptor_index;
extern const char *arg_payload;

/**
 * @brief avdecc_cmd_print_frame_header
 *
 * Pretty-print the DA,SA,Ethertype, and Payload length of an ethernet frame
 *
 * @param self The jdksavdecc_printer to print to
 * @param frame The frame to print
 */
void avdecc_cmd_print_frame_header( struct jdksavdecc_printer *self, const struct jdksavdecc_frame *frame );

/**
 * @brief avdecc_cmd_print_frame_payload
 *
 * Print ascii bytes of payload of ethernet frame to FILE
 *
 * @param f FILE to print hex bytes to
 * @param frame pointer to ethernet frame
 */
void avdecc_cmd_print_frame_payload( FILE *f, const struct jdksavdecc_frame *frame );

/**
 * @brief avdecc_cmd_process_incoming_raw
 *
 * Process incoming AVTPDU's from net for up to max_time_in_ms milliseconds.
 * For each AVTPDU received, call process with the network and the frame that was received.
 * If process returns non-zero, avdecc_cmd_process_incoming_raw returns immediately.
 * Always puts the network port socket into non-blocking mode.
 *
 * @param request_ const void * to the context dependent request structure to be forwarded to process function.
 * @param net The network port to use
 * @param max_time_in_ms Max time to wait
 * @param process function pointer to function taking (const void *request, struct raw_context *, const struct
 *jdksavdecc_frame*) and returning int
 */
void avdecc_cmd_process_incoming_raw( const void *request_,
                                      struct raw_context *net,
                                      int max_time_in_ms,
                                      int ( *process )( const void *request_,
                                                        struct raw_context *net,
                                                        const struct jdksavdecc_frame *frame ) );

#ifdef __cplusplus
}
#endif
