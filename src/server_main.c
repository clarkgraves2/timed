/** @file server_main.c
 *
 * @brief Main entry point for the time server.
 *
 * @details
 * Initializes the server with configuration file, system logger, and
 * runs the main loop. Once the server receives a signal 
 * it shuts down gracefully.
 */

#include <stdio.h>
#include <stdlib.h>

#include "../include/config.h"
#include "../include/syslog.h"
#include "../include/server.h"

int main(void) 
{  
    server_config_t config = {0}; 

    if (!config_init(&config)) // Set configs with default values.
    {
        (void)fprintf(stderr, "Failed to initialize server configuration\n");
        return EXIT_FAILURE;
    }

    syslog_write(INFO, "Server Configs Set Sucessfully");

    if (!syslog_init(&config))  // Initialize logging system.
    {
        (void)fprintf(stderr, "Failed to initialize logging system\n");
        return EXIT_FAILURE;
    }
    
    syslog_write(INFO, "Sylog Initialized Sucessfully");
    syslog_write(INFO, "Timed server starting up...");
    
    if (!server_init(&config)) // Initialize server subsystems with config fields.
    {
        syslog_write(CRITICAL, "Failed to initialize server");
        syslog_shutdown();
        return EXIT_FAILURE;
    }

    syslog_write(INFO, "Server Initialized Sucessfully");

    if (!server_run()) // Running the server.
    {
        syslog_write(ERROR, "Server terminated with errors");
        server_shutdown();
        return EXIT_FAILURE;
    }
    
    syslog_write(INFO, "Server Running...");

    if (!server_shutdown()) // When shutdown signal is received initiate shutdown sequence.
    {
        syslog_write(ERROR, "Failed to cleanly shut down server");
        return EXIT_FAILURE;
    }

    syslog_write(INFO, "Server shutdown complete");
    
    if (!syslog_shutdown()) // Shuts down syslog, the first initialized subsystem.
    {
        (void)fprintf(stderr, "Failed to shut down logging system\n");
        return EXIT_FAILURE;
    }
    
    return EXIT_SUCCESS;
}

/*** end of file ***/
