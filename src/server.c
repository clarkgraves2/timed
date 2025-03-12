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

    if(poll)


}