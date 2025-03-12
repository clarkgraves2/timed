#ifndef POLL_H
#define POLL_H

#include <stdbool.h>
#include "socket.h"
#include "config.h"

/**
 * @brief Initialize poll subsystem
 * 
 * @param[in] config Server configuration
 * @param[in] descriptors Socket descriptors to monitor
 * @return bool True if initialization successful
 */
bool poll_init(const server_config_t *config, const socket_descriptor_t *descriptors);

/**
 * @brief Run the main poll loop
 * 
 * @return bool True if poll loop exited normally
 */
bool poll_run(void);

/**
 * @brief Clean up poll resources
 * 
 * @return bool True if cleanup successful
 */
bool poll_cleanup(void);

#endif /* POLL_H */