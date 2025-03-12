/** @file poll.c
 *
 * @brief Implementation of poll-based event handling for the time server.
 *
 * This module implements the polling mechanism to monitor TCP and UDP
 * socket activity and dispatch to appropriate handlers when events occur.
 */

// NOLINTNEXTLINE(bugprone-reserved-identifier,cert-dcl37-c,cert-dcl51-cpp)
 #define _POSIX_C_SOURCE 200809L

 #include <poll.h>
 #include <stdlib.h>
 #include <string.h>
 #include <errno.h>
 #include <unistd.h>
 #include <time.h>
 #include <sys/socket.h>
 #include <sys/types.h>
 #include <sys/poll.h>
 #include <netinet/in.h>
 #include <arpa/inet.h>
 
 #include "../include/poll.h"
 #include "../include/socket.h"
 #include "../include/syslog.h"
 #include "../include/signal_handler.h"
 #include "../include/config.h"
 
 /*************************************************************************
 * Constants and Macros
 *************************************************************************/
 
 // Buffer size for client requests and responses
 #define MAX_BUFFER_SIZE (1024)
 
 /*************************************************************************
 * Static Variables
 *************************************************************************/
 
 // Poll file descriptors array
 static struct pollfd *poll_fds = NULL;
 
 // Number of file descriptors being monitored
 static size_t num_fds = 0;
 
 // Socket descriptors reference
 static const socket_descriptor_t *socket_desc = NULL;
 
 // Server configuration reference
 static const server_config_t *server_cfg = NULL;
 
 /*************************************************************************
 * Static Function Prototypes
 *************************************************************************/
 
 /**
  * @brief Handle incoming TCP connection
  *
  * Accepts new connection, processes request, and sends time response
  *
  * @param[in] tcp_socket  TCP socket file descriptor
  */
 static void
 handle_tcp_connection(int tcp_socket);
 
 /**
  * @brief Handle incoming UDP datagram
  *
  * Receives datagram, processes request, and sends time response
  *
  * @param[in] udp_socket  UDP socket file descriptor
  */
 static void
 handle_udp_datagram(int udp_socket);
 
 /**
  * @brief Format current time based on requested format
  *
  * @param[in]  p_format_str  Format string (or NULL for default)
  * @param[out] p_output      Buffer to store formatted time
  * @param[in]  output_size   Size of output buffer
  *
  * @return True if formatting successful, false otherwise
  */
 static bool
 format_time(const char *p_format_str, char *p_output, size_t output_size);
 
 /*************************************************************************
 * Function Implementations
 *************************************************************************/
 
 bool 
 poll_init(const server_config_t *p_config, const socket_descriptor_t *p_descriptors)
 {
     if (NULL == p_config || NULL == p_descriptors || 
         !socket_is_valid(p_descriptors))
     {
         syslog_write(ERROR, "Invalid parameters for poll initialization");
         return false;
     }
     
     // Store config and descriptors
     server_cfg = p_config;
     socket_desc = p_descriptors;
     
     // Allocate poll fds array
     num_fds = 2; // TCP and UDP sockets
     poll_fds = calloc(num_fds, sizeof(struct pollfd));
     if (NULL == poll_fds)
     {
         syslog_write(CRITICAL, "Failed to allocate memory for poll fds");
         return false;
     }
     
     // Set up TCP socket for polling
     poll_fds[0].fd = p_descriptors->tcp_socket;
     poll_fds[0].events = POLLIN;
     
     // Set up UDP socket for polling
     poll_fds[1].fd = p_descriptors->udp_socket;
     poll_fds[1].events = POLLIN;
     
     syslog_write(INFO, "Poll subsystem initialized with %zu fds", num_fds);
     return true;
 }
 
 bool 
 poll_run(void)
 {
     if (NULL == poll_fds || NULL == server_cfg || NULL == socket_desc)
     {
         syslog_write(ERROR, "Poll subsystem not initialized");
         return false;
     }
     
     syslog_write(INFO, "Starting main server loop");
     
     while (!signal_handler_shutdown_requested())
     {
         // Wait for events
         int poll_result = poll(poll_fds, (nfds_t)num_fds, server_cfg->poll_timeout);
         
         // Check for errors
         if (poll_result < 0)
         {
             if (errno == EINTR)
             {
                 // Interrupted by signal, check if shutdown requested
                 continue;
             }
             syslog_write(ERROR, "Poll error: %s", strerror(errno));
             return false;
         }
         
         // Check for timeout
         if (poll_result == 0)
         {
             // No events, just continue
             continue;
         }
         
         // Check for TCP events
         if (poll_fds[0].revents & POLLIN)
         {
             handle_tcp_connection(socket_desc->tcp_socket);
         }
         
         // Check for UDP events
         if (poll_fds[1].revents & POLLIN)
         {
             handle_udp_datagram(socket_desc->udp_socket);
         }
     }
     
     syslog_write(INFO, "Server loop terminated");
     return true;
 }
 
 bool 
 poll_cleanup(void)
 {
     if (NULL != poll_fds)
     {
         free(poll_fds);
         poll_fds = NULL;
     }
     
     num_fds = 0;
     socket_desc = NULL;
     server_cfg = NULL;
     
     syslog_write(INFO, "Poll subsystem cleaned up");
     return true;
 }
 
 /*************************************************************************
 * Private Functions
 *************************************************************************/
 
 static void
 handle_tcp_connection(int tcp_socket)
 {
     struct sockaddr_in client_addr;
     socklen_t addr_len = sizeof(client_addr);
     char buffer[MAX_BUFFER_SIZE] = {0};
     char time_buffer[MAX_BUFFER_SIZE] = {0};
     int client_fd = -1;
     ssize_t bytes_read = 0;
 
     // Accept new connection
     client_fd = accept(tcp_socket, (struct sockaddr *)&client_addr, &addr_len);
     if (client_fd < 0)
     {
         syslog_write(ERROR, "Failed to accept TCP connection: %s", strerror(errno));
         return;
     }
 
     // Log client connection
     syslog_write(INFO, "TCP connection from %s:%d", 
                 inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
 
     // Read client request (format string)
     bytes_read = recv(client_fd, buffer, sizeof(buffer) - 1, 0);
     
     // Handle various recv results
     if (bytes_read < 0) {
         if (errno == EAGAIN || errno == EWOULDBLOCK) {
             // Timeout occurred - use default format
             syslog_write(INFO, "Recv timeout - using default format");
             bytes_read = 0;  // Treat as empty request
         } else {
             syslog_write(ERROR, "Error reading from TCP client: %s", strerror(errno));
             close(client_fd);
             return;
         }
     }
 
     // Null-terminate the request string
     if (bytes_read > 0) {
         buffer[bytes_read] = '\0';
         syslog_write(INFO, "Received format string from TCP client: '%s', length: %ld", buffer, bytes_read);
     } else {
         syslog_write(INFO, "Using default format (empty request)");
         buffer[0] = '\0';  // Ensure empty string for default format
     }
 
     // Format time according to request (or use default if empty)
     if (!format_time(bytes_read > 0 ? buffer : NULL, time_buffer, sizeof(time_buffer))) {
         syslog_write(ERROR, "Failed to format time for TCP client");
         close(client_fd);
         return;
     }
 
     // Debug: Log the formatted time
     syslog_write(INFO, "Sending formatted time to TCP client: '%s'", time_buffer);
 
     // Ensure there's a proper newline at the end if not already present
     size_t time_len = strlen(time_buffer);
     if (time_len > 0 && time_len < sizeof(time_buffer) - 2) {
         // Check if the last characters are \r\n
         if (!(time_buffer[time_len-2] == '\r' && time_buffer[time_len-1] == '\n')) {
             // Check if the last character is \n
             if (time_buffer[time_len-1] != '\n') {
                 // Add \r\n to the end
                 time_buffer[time_len] = '\r';
                 time_buffer[time_len+1] = '\n';
                 time_buffer[time_len+2] = '\0';
             }
         }
     }
 
     // Send formatted time to client
     if (send(client_fd, time_buffer, strlen(time_buffer), 0) < 0) {
         syslog_write(ERROR, "Failed to send response to TCP client: %s", strerror(errno));
     }
 
     // Close client connection
     close(client_fd);
 }
 
static void
handle_udp_datagram(int udp_socket)
{
    struct sockaddr_in client_addr;
    socklen_t addr_len = sizeof(client_addr);
    char buffer[MAX_BUFFER_SIZE] = {0};
    char time_buffer[MAX_BUFFER_SIZE] = {0};
    ssize_t bytes_read = 0;
    
    // Receive datagram
    bytes_read = recvfrom(udp_socket, buffer, sizeof(buffer) - 1, 0,
                        (struct sockaddr *)&client_addr, &addr_len);
    
    if (bytes_read < 0)
    {
        syslog_write(ERROR, "Error receiving UDP datagram: %s", strerror(errno));
        return;
    }
    
    // Log client request
    syslog_write(INFO, "UDP request from %s:%d", 
                inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    // Null-terminate the request string
    if (bytes_read > 0)
    {
        buffer[bytes_read] = '\0';
        syslog_write(INFO, "Received format string from UDP client: '%s', length: %ld", buffer, bytes_read);
    }
    else
    {
        syslog_write(INFO, "Received empty format string from UDP client (bytes_read=%ld)", bytes_read);
    }
    
    // Format time according to request (or use default if empty)
    if (!format_time(bytes_read > 0 ? buffer : NULL, time_buffer, sizeof(time_buffer)))
    {
        syslog_write(ERROR, "Failed to format time for UDP client");
        return;
    }
    
    // Debug: Log the formatted time
    syslog_write(INFO, "Sending formatted time to UDP client: '%s'", time_buffer);
    
    // Send formatted time to client
    if (sendto(udp_socket, time_buffer, strlen(time_buffer), 0,
            (struct sockaddr *)&client_addr, addr_len) < 0)
    {
        syslog_write(ERROR, "Failed to send response to UDP client: %s", strerror(errno));
    }
}
 
static bool
format_time(const char *p_format_str, char *p_output, size_t output_size)
{
    time_t now = time(NULL);
    struct tm time_info;
    
    if (NULL == p_output || output_size == 0 || now == (time_t)-1)
    {
        return false;
    }
    
    // Get current time
    if (NULL == localtime_r(&now, &time_info))
    {
        return false;
    }
    
    // Check if format string is NULL or empty
    if (p_format_str == NULL || p_format_str[0] == '\0')
    {
        syslog_write(INFO, "Using default format: '%s'", server_cfg->time_format);
        size_t result = strftime(p_output, output_size, server_cfg->time_format, &time_info);
        if (result == 0)
        {
            syslog_write(ERROR, "Failed to format time with default format");
            return false;
        }
        return true;
    }
    
    // Check if format string contains only whitespace
    bool is_only_whitespace = true;
    for (size_t i = 0; p_format_str[i] != '\0'; i++) 
    {
        if (p_format_str[i] != ' ' && p_format_str[i] != '\t' && 
            p_format_str[i] != '\n' && p_format_str[i] != '\r')
        {
            is_only_whitespace = false;
            break;
        }
    }
    
    // Use provided format or default
    const char *p_format = is_only_whitespace ? server_cfg->time_format : p_format_str;
    
    syslog_write(INFO, "Formatting time with format: '%s'", p_format);
    
    // Format time
    size_t result = strftime(p_output, output_size, p_format, &time_info);
    
    if (result > 0)
    {
        syslog_write(INFO, "Formatted time result: '%s'", p_output);
        return true;
    }
    else
    {
        syslog_write(ERROR, "Failed to format time with format: '%s'", p_format);
        return false;
    }
}
 
 /*** end of file ***/