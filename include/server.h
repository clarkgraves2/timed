/** @file server.h
 *
 * @brief Server lifecycle management API
 *
 * This header defines the public interface for the server component,
 * which manages the complete lifecycle of the time server including
 * initialization, event processing, and shutdown.
 */

 #ifndef SERVER_H
 #define SERVER_H
 
 #include <stdbool.h>
 #include "./config.h"
 
 /**
  * @brief Initialize the server with the provided configuration
  *
  * This function initializes all server subsystems including logging, 
  * signal handling, sockets, and the polling mechanism.
  * 
  * @param[in] config  Pointer to server configuration structure
  * 
  * @return True if initialization successful, false otherwise
  */
 bool server_init(const server_config_t * config);
 
 /**
 * @brief Run the server main event loop
 *
 * This function starts the server main loop, which processes incoming
 * connections and handles client requests until a shutdown is requested.
 * 
 * @return True if server terminated normally, false if an error occurred
 */
bool server_run(void);

/**
 * @brief Perform a graceful server shutdown
 *
 * This function cleans up all server resources and subsystems in the
 * correct order to ensure a clean shutdown.
 * 
 * @return True if shutdown successful, false otherwise
 */
bool server_shutdown(void);

#endif /* SERVER_H */