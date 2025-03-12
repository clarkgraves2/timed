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

/*************************************************************************
* Types and Enums
*************************************************************************/

/**
* @brief Server initialization stages for controlled cleanup
*/
typedef enum 
{
    INIT_NONE,     /* No initialization performed */
    INIT_LOGGING,      /* Logging system initialized */
    INIT_SIGNALS,      /* Signal handlers initialized */
    INIT_SOCKET,       /* Server socket created */
    INIT_POLL,         /* Poll subsystem initialized */
    INIT_COMPLETE      /* All initialization complete */
} init_stage_t;

/*************************************************************************
* Static Variables
*************************************************************************/
 
 /* Server state */
 static volatile bool is_running = false;
 static server_config_t server_config = {0};
 static init_stage_t cleanup_stage = INIT_NONE;
 static socket_descriptor_t socket_descriptors = {-1, -1};

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
server_init(const server_config_t *config)
{
    if (NULL == config) 
    {
        syslog_write(CRITICAL, "Server configs for initialization are NULL");
        cleanup_server(cleanup_stage);
        return false;
    }
    
    syslog_write(INFO, "Configs Loaded, Server starting...");

    if(!signal_handler_init())
    {
        syslog_write(CRITICAL, "Signal Handler failed to initialize.");
        cleanup_server(cleanup_stage);
        return false;
    }

    cleanup_stage = INIT_SIGNALS;

    if(!socket_init(&config, &socket_descriptors))
    {
        syslog_write(CRITICAL, "Socket initialization failed");
        cleanup_server(cleanup_stage);
        return false;
    }

    cleanup_stage = INIT_SOCKET;

    if (!poll_init(&server_config, &socket_descriptors))
    {
        syslog_write(CRITICAL, "Poll initialization failed");
        cleanup_server(cleanup_stage);
        return false;
    }
    
    is_running = true;
    syslog_write(INFO, "Server initialization complete");
    cleanup_stage = INIT_COMPLETE;
    
    return true;
}

bool 
server_run(void)
{
    bool result = false;
    
    if (!is_running || cleanup_stage != INIT_COMPLETE)
    {
        syslog_write(ERROR, "Attempted to run server before complete initialization");
        return false;
    }
    
    syslog_write(INFO, "Server starting main event loop");
    
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
    if (!is_running)
    {
        return false;
    }
    
    syslog_write(INFO, "Server shutting down...");
    is_running = false;
    
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
            /* Fall through */
            
        case INIT_SOCKET:
            if (!socket_cleanup(&socket_descriptors))
            {
                syslog_write(WARNING, "Failed to cleanly shut down sockets");
                success = false;
            }
            /* Fall through */
            
        case INIT_SIGNALS:
            if (!signal_handler_cleanup())
            {
                syslog_write(WARNING, "Failed to cleanly reset signal handlers");
                success = false;
            }
            /* Fall through */
            
        case INIT_LOGGING:
            /* Clean up logging last so we can log other cleanup issues */
            if (!syslog_shutdown())
            {
                fprintf(stderr, "Failed to cleanly shut down logging system\n");
                success = false;
            }
            /* Fall through */
            
        case INIT_NONE:
            /* Nothing to clean up at this stage */
            break;
            
        default:
            /* Unknown stage */
            success = false;
            break;
    }
    
    is_running = false;
    cleanup_stage = INIT_NONE;
    return success;
}

/*** end of file ***/