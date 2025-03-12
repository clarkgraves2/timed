/** @file server_main.c
 *
 * @brief Main entry point for the time server.
 *
 * @details
 * Initializes the server with configuration file, system logger, and
 * runs the main loop. Once the server times out or recieves a signal 
 * it shutsdown gracefully.
 */

#include <stdio.h>
#include <stdlib.h>

#include "./include/config.h"
#include "./include/syslog.h"
#include "./include/server.h"

int main(void) 
{  
    server_config_t config = config_init(); // Set default server configuration fields.

    if (!server_init(&config)) // Initialize server subsystems with config fields.
    {
        fprintf(stderr, "Failed to initialize server\n");
        return EXIT_FAILURE;
    }

    if (!server_run())
    {
        fprintf(stderr, "Server terminated with errors\n");
        server_shutdown();
        return EXIT_FAILURE;
    }
    
    if (!server_shutdown())
    {
        fprintf(stderr, "Failed to cleanly shut down server\n");
        return EXIT_FAILURE;
    }
    
    printf("Server shutdown complete\n");
    return EXIT_SUCCESS;
}

/*** end of file ***/