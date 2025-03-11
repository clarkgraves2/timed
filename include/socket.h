/** @file socket.h
*
* @brief Socket management interface for the time server
*
* This module handles initialization, configuration, and cleanup
* of the TCP and UDP sockets used by the time server.
*/

#ifndef SOCKET_H
#define SOCKET_H

#include <stdbool.h>
#include "../include/config.h"

/**
 * @brief Socket context structure to manage socket file descriptors
 */
typedef struct {
    int tcp_socket;   /* TCP socket file descriptor */
    int udp_socket;   /* UDP socket file descriptor */
} socket_context_t;

/**
 * @brief Initialize the socket context with TCP and UDP sockets
 *
 * Creates, configures, and binds both TCP and UDP sockets according
 * to the provided server configuration.
 *
 * @param[in] config Pointer to server configuration
 * @param[out] context Pointer to socket context to be initialized
 * 
 * @return true if initialization was successful, false otherwise
 */
bool socket_init(const server_config_t *config, socket_context_t *context);

/**
 * @brief Clean up socket resources
 *
 * Closes open socket file descriptors and resets the socket context.
 *
 * @param[in,out] context Pointer to socket context to be cleaned up
 * 
 * @return true if cleanup was successful, false otherwise
 */
bool socket_cleanup(socket_context_t *context);

/**
 * @brief Check if a socket context is valid
 *
 * @param[in] context Pointer to socket context to check
 * 
 * @return true if context is valid, false otherwise
 */
bool socket_is_valid(const socket_context_t *context);

#endif /* SOCKET_H */