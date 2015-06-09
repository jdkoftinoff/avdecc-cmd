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

ssize_t send_frame( struct raw_context *self, const struct jdksavdecc_frame *frame );
void discovered_callback( struct discover *self, const struct discovered_entity *entity );
void removed_callback( struct discover *self, const struct discovered_entity *entity );

ssize_t send_frame( struct raw_context *self, const struct jdksavdecc_frame *frame )
{
    return raw_send( self, frame->dest_address.value, frame->payload, frame->length );
}

void discovered_callback( struct discover *self, const struct discovered_entity *entity )
{
    char buf[1024];
    struct jdksavdecc_printer printer;
    jdksavdecc_printer_init( &printer, buf, sizeof( buf ) );
    jdksavdecc_printer_print_label( &printer, "Discovered " );
    jdksavdecc_printer_print_eui64( &printer, entity->entity_id );
    printf( "%s\n", buf );
}

void removed_callback( struct discover *self, const struct discovered_entity *entity )
{
    char buf[1024];
    struct jdksavdecc_printer printer;
    jdksavdecc_printer_init( &printer, buf, sizeof( buf ) );
    jdksavdecc_printer_print_label( &printer, "Removed: " );
    jdksavdecc_printer_print_eui64( &printer, entity->entity_id );
    printf( "%s\n", buf );
}

int main( int argc, char **argv )
{
    int r = 1;
    if ( argc > 1 && argv[1][0] == '-' && argv[1][1] == 'h' )
    {
        fprintf( stderr,
                 "avdecc-advanced-discover usage:\n"
                 "\tavdecc-advanced-discover [verbosity] [timeout_in_ms] [network_port]\n\n" );
        return 1;
    }

    arg_verbose = 2;
    arg_time_in_ms_to_wait = 50000;
    arg_network_port = "en4";

    if ( argc > 1 )
    {
        arg_verbose = atoi( argv[1] );
    }
    if ( argc > 2 )
    {
        arg_time_in_ms_to_wait = atoi( argv[2] );
    }
    if ( argc > 3 )
    {
        arg_network_port = argv[3];
    }

    struct raw_context net;
    int fd = raw_socket( &net, JDKSAVDECC_AVTP_ETHERTYPE, arg_network_port, jdksavdecc_multicast_adp_acmp.value );

    if ( fd >= 0 )
    {
        struct discover discover_engine;
        struct jdksavdecc_eui64 controller_entity_id = raw_generate_controller_entity_id( &net, 0x0000 );

        if ( discover_init(
                 &discover_engine, controller_entity_id, 512, &net, 0, discovered_callback, removed_callback, send_frame ) )
        {
            jdksavdecc_timestamp_in_milliseconds current_time_in_milliseconds = raw_get_time_of_day_in_milliseconds();
            jdksavdecc_timestamp_in_milliseconds end_time_in_milliseconds = current_time_in_milliseconds
                                                                            + arg_time_in_ms_to_wait;

            do
            {
                discover_tick( &discover_engine, current_time_in_milliseconds );
                raw_dispatch_one( &discover_engine, &net, 1000, discover_process_incoming, 0, 0 );
                current_time_in_milliseconds = raw_get_time_of_day_in_milliseconds();
            } while ( current_time_in_milliseconds < end_time_in_milliseconds );

            discover_free( &discover_engine );
        }

        raw_close( &net );
    }

    return 0;
}
