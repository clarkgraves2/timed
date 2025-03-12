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
     server_config_t config;
      
     // Initialize config with default values
     if (!config_init(&config))
     {
         // Cast to void to silence warning about unused return value
         (void)fprintf(stderr, "Failed to initialize server configuration\n");
         return EXIT_FAILURE;
     }
      
     // Initialize logging system
     if (!syslog_init(&config))
     {
         // Cast to void to silence warning about unused return value
         (void)fprintf(stderr, "Failed to initialize logging system\n");
         return EXIT_FAILURE;
     }
      
     // Log server startup
     syslog_write(INFO, "Timed server starting up...");
      
     // Initialize server subsystems with config fields
     if (!server_init(&config))
     {
         syslog_write(CRITICAL, "Failed to initialize server");
         syslog_shutdown();
         return EXIT_FAILURE;
     }
  
     if (!server_run())
     {
         syslog_write(ERROR, "Server terminated with errors");
         server_shutdown();
         return EXIT_FAILURE;
     }
      
     if (!server_shutdown())
     {
         syslog_write(ERROR, "Failed to cleanly shut down server");
         return EXIT_FAILURE;
     }
      
     syslog_write(INFO, "Server shutdown complete");
     syslog_shutdown();
      
     return EXIT_SUCCESS;
 }
  
 /*** end of file ***/