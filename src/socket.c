/** @file socket.c
*
* @brief Implementation of socket management for the time server
*
* This module implements the initialization, configuration, and cleanup
* of TCP and UDP sockets used by the time server.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "../include/socket.h"
#include "../include/syslog.h"

/*************************************************************************
* Constants and Macros
*************************************************************************/

// Socket option values 
#define SOCKET_OPTION_ENABLE (1)

/*************************************************************************
* Public Functions
*************************************************************************/

bool 
socket_init(const server_config_t *config, socket_descriptor_t * descriptors)
{
    struct sockaddr_in server_addr;
    
    // Parameter validation 
    if (NULL == config || NULL == descriptors)
    {
        syslog_write(ERROR, "Invalid parameters to socket_init");
        return false;
    }
    
    // Initialize socket descriptors 
    descriptors->tcp_socket = -1;
    descriptors->udp_socket = -1;
    
    // Prepare server address structure 
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(config->port);
    
    syslog_write(INFO, "Initializing sockets on port %d", config->port);
    
    // Initialize TCP socket 
    descriptors->tcp_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (descriptors->tcp_socket < 0)
    {
        syslog_write(CRITICAL, "Failed to create TCP socket: %s", strerror(errno));
        socket_cleanup(descriptors);
        return false;
    }
    
    // Set socket options for address reuse
    if (setsockopt(descriptors->tcp_socket, SOL_SOCKET, SO_REUSEADDR, 
                  &(int){SOCKET_OPTION_ENABLE}, sizeof(int)) < 0)
    {
        syslog_write(ERROR, "Failed to set TCP socket options: %s", strerror(errno));
        socket_cleanup(descriptors);
        return false;
    }
    
    // Bind TCP socket 
    if (bind(descriptors->tcp_socket, (struct sockaddr *)&server_addr, 
            sizeof(server_addr)) < 0)
    {
        syslog_write(CRITICAL, "Failed to bind TCP socket: %s", strerror(errno));
        socket_cleanup(descriptors);
        return false;
    }
    
    // Listen on TCP socket 
    if (listen(descriptors->tcp_socket, SOMAXCONN) < 0)
    {
        syslog_write(CRITICAL, "Failed to listen on TCP socket: %s", strerror(errno));
        socket_cleanup(descriptors);
        return false;
    }
    
    // Initialize UDP socket 
    descriptors->udp_socket = socket(AF_INET, SOCK_DGRAM, 0);
    if (descriptors->udp_socket < 0)
    {
        syslog_write(CRITICAL, "Failed to create UDP socket: %s", strerror(errno));
        socket_cleanup(descriptors);
        return false;
    }
    
    // Set socket options for address reuse (UDP) 
    if (setsockopt(descriptors->udp_socket, SOL_SOCKET, SO_REUSEADDR, 
                  &(int){SOCKET_OPTION_ENABLE}, sizeof(int)) < 0)
    {
        syslog_write(ERROR, "Failed to set UDP socket options: %s", strerror(errno));
        socket_cleanup(descriptors);
        return false;
    }
    
    // Bind UDP socket 
    if (bind(descriptors->udp_socket, (struct sockaddr *)&server_addr, 
            sizeof(server_addr)) < 0)
    {
        syslog_write(CRITICAL, "Failed to bind UDP socket: %s", strerror(errno));
        socket_cleanup(descriptors);
        return false;
    }
    
    syslog_write(INFO, "Socket initialization successful (TCP: %d, UDP: %d)",
                descriptors->tcp_socket, descriptors->udp_socket);
    
    return true;
}

bool 
socket_cleanup(socket_descriptor_t *descriptors)
{
    bool success = true;
    
    // Parameter validation 
    if (NULL == descriptors)
    {
        return false;
    }
    
    // Close TCP socket if open 
    if (descriptors->tcp_socket >= 0)
    {
        if (close(descriptors->tcp_socket) < 0)
        {
            syslog_write(ERROR, "Error closing TCP socket: %s", strerror(errno));
            success = false;
        }
        else
        {
            syslog_write(INFO, "TCP socket closed successfully");
        }
        descriptors->tcp_socket = -1;
    }
    
    // Close UDP socket if open 
    if (descriptors->udp_socket >= 0)
    {
        if (close(descriptors->udp_socket) < 0)
        {
            syslog_write(ERROR, "Error closing UDP socket: %s", strerror(errno));
            success = false;
        }
        else
        {
            syslog_write(INFO, "UDP socket closed successfully");
        }
        descriptors->udp_socket = -1;
    }
    
    return success;
}

bool 
socket_is_valid(const socket_descriptor_t *descriptors)
{
    if (NULL == descriptors)
    {
        return false;
    }
    
    // Check if both sockets are valid file descriptors 
    return (descriptors->tcp_socket >= 0 && descriptors->udp_socket >= 0);
}

/*** end of file ***/