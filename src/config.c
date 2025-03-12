/** @file config.c
*
* @brief Implementation of server configuration function.
*
* Initializes the default configurations fields
* used by the servers subsystems making configuring the
* server more modulular.
*/

#include "../include/config.h"
#include <string.h>

/**
 * @brief Initialize a server configuration with default values
 *
 * @param[out] p_config  Pointer to configuration structure to initialize
 *
 * @return True if initialization successful, false otherwise
 */
bool config_init(server_config_t *p_config)
{
    if (NULL == p_config)
    {
        return false;
    }
    
    p_config->port = CONFIG_SERVER_PORT;
    p_config->log_file = CONFIG_SERVER_LOG_FILE;
    p_config->poll_fds = CONFIG_MAX_POLL_FDS;
    p_config->poll_timeout = CONFIG_POLL_TIMEOUT;
    p_config->time_format = CONFIG_TIME_FORMAT;
    
    return true;
}

/*** end of file ***/