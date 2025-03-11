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
    
    // Thread management 
    int num_worker_threads;  // Number of worker threads for the thread pool 
    
    // Logging configuration 
    const char * log_file;    // Path to log file

} server_config_t;

/*************************************************************************
* Constants
*************************************************************************/
 
// Default server configuration 
#define CONFIG_SERVER_PORT             (8080)
#define CONFIG_SERVER_WORKER_THREADS   (4)
#define CONFIG_SERVER_LOG_FILE         "server.log"

/**
 * @brief Create a server configuration with default values
 *
 * @return Default server configuration
 */
server_config_t config_init(void);

#endif /* CONFIG_H */

/*** end of file ***/
