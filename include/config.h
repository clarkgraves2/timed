/** @file config.h
 *
 * @brief Centralized server configuration definitions.
 *
 * This module provides a unified configuration structure and default values
 * for all server subsystems, including networking, logging, database paths,
 * and thread management.
 */

#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

/*************************************************************************
 * Type Definitions
 *************************************************************************/

/**
 * @brief Server Configuration Structure
 */
typedef struct server_config 
{
    // Network settings 
    int port;                // Server listening port 
        
    // Logging configuration 
    const char * log_file;    // Path to log file

    // Poll configurations
    int poll_fds;           // Number of file descriptors for poll to monitor 
    int poll_timeout;       // Poll timeout in milliseconds 
    
    // Time format
    const char * time_format;    // Default time format 

} server_config_t;

/*************************************************************************
 * Constants
 *************************************************************************/

// Default server configuration 
#define CONFIG_SERVER_PORT             (13)
#define CONFIG_SERVER_LOG_FILE         "server.log"
#define CONFIG_MAX_POLL_FDS            (1024)
#define CONFIG_POLL_TIMEOUT            (1000)
#define CONFIG_TIME_FORMAT             "%a %b %d %H:%M:%S %Y\r\n"

/**
 * @brief Initialize a server configuration with default values
 *
 * @param[out] p_config  Pointer to configuration structure to initialize
 *
 * @return True if initialization successful, false otherwise
 */
bool config_init(server_config_t *p_config);

#endif /* CONFIG_H */

/*** end of file ***/
