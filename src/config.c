/** @file config.c
*
* @brief Implementation of server configuration function.
*
* Initializes the default configurations fields
* used by the servers subsystems making configuring the
* server more modulular.
*/

#include "../include/config.h"

server_config_t config_init(void)
{
    server_config_t config = 
    {
        .port =                CONFIG_SERVER_PORT,
        .num_worker_threads =  CONFIG_SERVER_WORKER_THREADS,
        .log_file =            CONFIG_SERVER_LOG_FILE,
        .poll_fds =            CONFIG_MAX_POLL_FDS,
        .poll_timeout =        CONFIG_POLL_TIMEOUT
    };
    
    return config;
}

/*** end of file ***/