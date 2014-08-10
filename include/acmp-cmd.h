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

#include "avdecc-cmd.h"
#include "acmp.h"
#include "jdksavdecc_acmp_print.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief acmp_print
 *
 * Print the details of the acmp message in the ethernet frame
 *
 * @param s The output stream to print the ascii to
 * @param frame The ethernet frame which contains an acmp message
 * @param acmpdu The parsed acmp message
 */
void acmp_print( FILE *s, const struct jdksavdecc_frame *frame, const struct jdksavdecc_acmpdu *acmpdu );

/**
 * @brief acmp_process
 * @param request_
 * @param net
 * @param frame
 * @return
 */
int acmp_process( const void *request_, struct raw_context *net, const struct jdksavdecc_frame *frame );

/**
 * @brief handle acmp command line request
 *
 * command line arguments form:
 *
 * @param net raw network port to use
 * @param frame Ethernet Frame to use to send
 * @param verbose 1 for verbose information about operations done
 * @param time_ms_to_wait time in milliseconds to wait for responses, or 0 for none
 * @param argc count of arguments including "acmp"
 * @param argv array of arguments starting at "acmp"
 * @return 0 on success
 */
int acmp( struct raw_context *net, struct jdksavdecc_frame *frame, int argc, char **argv );

#ifdef __cplusplus
}
#endif
