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

#include "config.h"

/**
 * @brief Manage socket file descriptors
 */
typedef struct
{
    int tcp_socket; // TCP socket file descriptor
    int udp_socket; // UDP socket file descriptor
} socket_descriptor_t;

/**
 * @brief Initialize the socket descriptors with TCP and UDP sockets
 *
 * Creates, configures, and binds both TCP and UDP sockets according
 * to the provided server configuration.
 *
 * @param[in] config Pointer to server configuration
 * @param[out] descriptors Pointer to socket descriptors to be initialized
 *
 * @return true if initialization was successful, false otherwise
 */
bool socket_init (const server_config_t * config,
                  socket_descriptor_t *   descriptors);

/**
 * @brief Clean up socket resources
 *
 * Closes open socket file descriptors and resets the socket descriptors.
 *
 * @param[in,out] descriptors Pointer to socket descriptors to be cleaned up
 *
 * @return true if cleanup was successful, false otherwise
 */
bool socket_cleanup (socket_descriptor_t * descriptors);

/**
 * @brief Check if a socket descriptors is valid
 *
 * @param[in] descriptors Pointer to socket descriptors to check
 *
 * @return true if descriptors is valid, false otherwise
 */
bool socket_is_valid (const socket_descriptor_t * descriptors);

#endif /* SOCKET_H */
