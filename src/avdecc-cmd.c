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

void avdecc_cmd_print_frame_header( struct jdksavdecc_printer *self, const struct jdksavdecc_frame *frame )
{
    jdksavdecc_printer_print_label( self, "DA" );
    jdksavdecc_printer_print_eui48( self, frame->dest_address );
    jdksavdecc_printer_print_eol( self );
    jdksavdecc_printer_print_label( self, "SA" );
    jdksavdecc_printer_print_eui48( self, frame->src_address );
    jdksavdecc_printer_print_eol( self );
    jdksavdecc_printer_print_label( self, "EtherType" );
    jdksavdecc_printer_print_uint16( self, frame->ethertype );
    jdksavdecc_printer_print_eol( self );
    jdksavdecc_printer_print_label( self, "Payload Length" );
    jdksavdecc_printer_print_uint16( self, frame->length );
    jdksavdecc_printer_print_eol( self );
}
