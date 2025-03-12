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

#include "../include/config.h"
#include "../include/syslog.h"
#include "../include/server.h"

int main(void) 
{  
    server_config_t config;
    
    if (!config_init(&config)) // Initialize configs with default values
    {
        fprintf(stderr, "Failed to initialize server configuration\n");
        return EXIT_FAILURE;
    }

    if (!syslog_init(&config))  // Initialize logging system
    {
        fprintf(stderr, "Failed to initialize logging system\n");
        return EXIT_FAILURE;
    }
    
    syslog_write(INFO, "Timed server starting up...");
    
    if (!server_init(&config)) // Initialize server subsystems with config fields
    {
        syslog_write(CRITICAL, "Failed to initialize server");
        syslog_shutdown();
        return EXIT_FAILURE;
    }

    if (!server_run()) // Running the server
    {
        syslog_write(ERROR, "Server terminated with errors");
        server_shutdown();
        return EXIT_FAILURE;
    }
    
    if (!server_shutdown()) // When Shutdown signal is receieved initiate shutdown sequence.
    {
        syslog_write(ERROR, "Failed to cleanly shut down server");
        return EXIT_FAILURE;
    }

    syslog_shutdown();
    syslog_write(INFO, "Server shutdown complete");
    
    return EXIT_SUCCESS;
}

/*** end of file ***/