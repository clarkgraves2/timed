/** @file server.c
 *
 * @brief Implementation of server lifecycle management with simplified cleanup
 *
 * @details
 * This module implements the complete server lifecycle including
 * initialization, event processing, and shutdown procedures.
 * Uses a stage-based initialization approach for simpler cleanup.
 */

 #include <stdio.h>
 #include <stdlib.h>
 #include <string.h>
 #include <signal.h>
 #include <unistd.h>
 #include <sys/socket.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 #include <errno.h>
 
 #include "../include/syslog.h"
 #include "../include/server.h"
 #include "../include/signal_handler.h"
 #include "../include/socket.h"
 #include "../include/poll.h"
 
 /*************************************************************************
 * Types and Enums
 *************************************************************************/
 
 /**
 * @brief Server initialization stages for controlled cleanup
 */
 typedef enum 
 {
     INIT_NONE = 0,    /* No initialization performed */
     INIT_SIGNALS = 1, /* Signal handlers initialized */
     INIT_SOCKET = 2,  /* Server socket created */
     INIT_POLL = 3,    /* Poll subsystem initialized */
     INIT_COMPLETE = 4 /* All initialization complete */
 } init_stage_t;
 
 /*************************************************************************
 * Static Variables
 *************************************************************************/
  
 /*************************************************************************
 * Justification: Clang-tidy warning suppression
 * These variables need to be mutable and maintain state throughout
 * the server lifecycle. They are protected by being static and are only 
 * accessed within this module. There are getters and setters to safely 
 * access and modify values.
 **************************************************************************/
static volatile bool server_running = false; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static init_stage_t cleanup_stage = INIT_NONE; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
static socket_descriptor_t socket_descriptors = {-1, -1}; // NOLINT(cppcoreguidelines-avoid-non-const-global-variables)
 
 /*************************************************************************
 * Static Function Prototypes
 *************************************************************************/
 
 /**
  * @brief Performs staged cleanup based on initialization progress
  *
  * @param[in] stage  The stage to clean up from
  *
  * @return True if cleanup successful, false otherwise
  */
 static bool
 cleanup_server(init_stage_t stage);
 
 /*************************************************************************
 * Public Functions
 *************************************************************************/
  
 bool 
 server_init(const server_config_t * config)
 {
     if (NULL == config) 
     {
         syslog_write(CRITICAL, "Server configs for initialization are NULL");
         cleanup_server(cleanup_stage);
         return false;
     }
     
     syslog_write(INFO, "Initializing server on port %d", config->port);
 
     if(!signal_handler_init())
     {
         syslog_write(CRITICAL, "Signal Handler failed to initialize.");
         cleanup_server(cleanup_stage);
         return false;
     }
 
     cleanup_stage = INIT_SIGNALS;
     syslog_write(INFO, "Signal handlers initialized successfully");
 
     if(!socket_init(config, &socket_descriptors))
     {
         syslog_write(CRITICAL, "Socket initialization failed");
         cleanup_server(cleanup_stage);
         return false;
     }
 
     cleanup_stage = INIT_SOCKET;
     syslog_write(INFO, "Sockets initialized successfully");
 
     if (!poll_init(config, &socket_descriptors))
     {
         syslog_write(CRITICAL, "Poll initialization failed");
         cleanup_server(cleanup_stage);
         return false;
     }
 
     cleanup_stage = INIT_POLL;
     syslog_write(INFO, "Poll subsystem initialized successfully");
     
     server_running = true;
     syslog_write(INFO, "Server initialization complete");
     cleanup_stage = INIT_COMPLETE;
     
     return true;
 }
 
 bool 
 server_run(void)
 {
     bool result = false;
     
     if (!server_running || cleanup_stage != INIT_COMPLETE)
     {
         syslog_write(ERROR, "Attempted to run server before complete initialization");
         return false;
     }
     
     syslog_write(INFO, "Server ready and waiting for connections");
     
     /* Run the poll event loop */
     result = poll_run();
     
     if (!result)
     {
         syslog_write(ERROR, "Server event loop terminated with error");
     }
     else
     {
         syslog_write(INFO, "Server event loop terminated normally");
     }
     
     return result;
 }
 
 bool 
 server_shutdown(void)
 {
     if (!server_running)
     {
         syslog_write(WARNING, "Attempted to shut down server that is not running");
         return false;
     }
     
     syslog_write(INFO, "Server shutting down...");
     server_running = false;
     
     /* Clean up from the current stage */
     return cleanup_server(cleanup_stage);
 }
 
 /*************************************************************************
 * Static Functions
 *************************************************************************/
 
 static bool
 cleanup_server(init_stage_t stage)
 {
     bool success = true;
     
     /* Perform cleanup in reverse order of initialization */
     switch (stage)
     {
         case INIT_COMPLETE:
         case INIT_POLL:
             if (!poll_cleanup())
             {
                 syslog_write(WARNING, "Failed to cleanly shut down poll subsystem");
                 success = false;
             }
             else
             {
                 syslog_write(INFO, "Poll subsystem shutdown successfully");
             }
             /* Fall through */
             
         case INIT_SOCKET:
             if (!socket_cleanup(&socket_descriptors))
             {
                 syslog_write(WARNING, "Failed to cleanly shut down sockets");
                 success = false;
             }
             else
             {
                 syslog_write(INFO, "Socket shutdown successfully");
             }
             /* Fall through */
             
         case INIT_SIGNALS:
             if (!signal_handler_cleanup())
             {
                 syslog_write(WARNING, "Failed to cleanly reset signal handlers");
                 success = false;
             }
             else
             {
                 syslog_write(INFO, "Signal handlers reset successfully");
             }
             /* Fall through */
             
         case INIT_NONE:
             /* Nothing to clean up at this stage */
             break;
             
         default:
             /* Unknown stage */
             syslog_write(ERROR, "Unknown initialization stage: %d", stage);
             success = false;
             break;
     }
     
     server_running = false;
     cleanup_stage = INIT_NONE;
     return success;
 }

/*** end of file ***/
