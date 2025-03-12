/** @file poll.h
 *
 * @brief Interface for poll-based event handling for the time server.
 *
 * @details
 * This module implements the polling mechanism to monitor socket activity
 * and dispatch to appropriate handlers for TCP and UDP connections.
 */

 #ifndef POLL_H
 #define POLL_H
 
 #include <stdbool.h>
 #include "../include/socket.h"
 #include "../include/config.h"
 
 /**
  * @brief Initialize poll subsystem
  * 
  * Allocates and configures resources for polling both TCP and UDP sockets
  * 
  * @param[in] p_config       Pointer to server configuration
  * @param[in] p_descriptors  Pointer to socket descriptors to monitor
  * 
  * @return True if initialization successful, false otherwise
  */
 bool
 poll_init(const server_config_t *p_config, const socket_descriptor_t *p_descriptors);
 
 /**
  * @brief Run the main poll loop
  * 
  * Monitors socket activity and dispatches to appropriate handlers
  * until shutdown is requested via signal handler.
  * 
  * @return True if poll loop exited normally, false otherwise
  */
 bool
 poll_run(void);
 
 /**
  * @brief Clean up poll resources
  * 
  * Frees all resources allocated during poll initialization
  * 
  * @return True if cleanup successful, false otherwise
  */
 bool
 poll_cleanup(void);
 
 #endif /* POLL_H */