/** @file poll.c
 *
 * @brief Implementation of poll-based event handling for the time server.
 *
 * This module implements the polling mechanism to monitor TCP and UDP
 * socket activity and dispatch to appropriate handlers when events occur.
 */

 #include <poll.h>
 #include <stdlib.h>
 #include <string.h>
 #include <errno.h>
 #include <unistd.h>
 #include <time.h>
 #include <sys/socket.h>
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
 static struct pollfd *p_poll_fds = NULL;
 
 // Number of file descriptors being monitored
 static size_t num_fds = 0;
 
 // Socket descriptors reference
 static const socket_descriptor_t *p_socket_desc = NULL;
 
 // Server configuration reference
 static const server_config_t *p_server_cfg = NULL;
 
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
     p_server_cfg = p_config;
     p_socket_desc = p_descriptors;
     
     // Allocate poll fds array
     num_fds = 2; // TCP and UDP sockets
     p_poll_fds = calloc(num_fds, sizeof(struct pollfd));
     if (NULL == p_poll_fds)
     {
         syslog_write(CRITICAL, "Failed to allocate memory for poll fds");
         return false;
     }
     
     // Set up TCP socket for polling
     p_poll_fds[0].fd = p_descriptors->tcp_socket;
     p_poll_fds[0].events = POLLIN;
     
     // Set up UDP socket for polling
     p_poll_fds[1].fd = p_descriptors->udp_socket;
     p_poll_fds[1].events = POLLIN;
     
     syslog_write(INFO, "Poll subsystem initialized with %zu fds", num_fds);
     return true;
 }
 
 bool 
 poll_run(void)
 {
     if (NULL == p_poll_fds || NULL == p_server_cfg || NULL == p_socket_desc)
     {
         syslog_write(ERROR, "Poll subsystem not initialized");
         return false;
     }
     
     syslog_write(INFO, "Starting main server loop");
     
     while (!signal_handler_shutdown_requested())
     {
         // Wait for events
         int poll_result = poll(p_poll_fds, (nfds_t)num_fds, p_server_cfg->poll_timeout);
         
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
         if (p_poll_fds[0].revents & POLLIN)
         {
             handle_tcp_connection(p_socket_desc->tcp_socket);
         }
         
         // Check for UDP events
         if (p_poll_fds[1].revents & POLLIN)
         {
             handle_udp_datagram(p_socket_desc->udp_socket);
         }
     }
     
     syslog_write(INFO, "Server loop terminated");
     return true;
 }
 
 bool 
 poll_cleanup(void)
 {
     if (NULL != p_poll_fds)
     {
         free(p_poll_fds);
         p_poll_fds = NULL;
     }
     
     num_fds = 0;
     p_socket_desc = NULL;
     p_server_cfg = NULL;
     
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
     int client_fd;
     ssize_t bytes_read;
     
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
     if (bytes_read < 0)
     {
         syslog_write(ERROR, "Error reading from TCP client: %s", strerror(errno));
         close(client_fd);
         return;
     }
     
     // Null-terminate the request string
     if (bytes_read > 0)
     {
         buffer[bytes_read] = '\0';
     }
     
     // Format time according to request (or use default if empty)
     if (!format_time(bytes_read > 0 ? buffer : NULL, time_buffer, sizeof(time_buffer)))
     {
         syslog_write(ERROR, "Failed to format time for TCP client");
         close(client_fd);
         return;
     }
     
     // Send formatted time to client
     if (send(client_fd, time_buffer, strlen(time_buffer), 0) < 0)
     {
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
     ssize_t bytes_read;
     
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
     }
     
     // Format time according to request (or use default if empty)
     if (!format_time(bytes_read > 0 ? buffer : NULL, time_buffer, sizeof(time_buffer)))
     {
         syslog_write(ERROR, "Failed to format time for UDP client");
         return;
     }
     
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
     time_t now;
     struct tm time_info;
     
     if (NULL == p_output || output_size == 0)
     {
         return false;
     }
     
     // Get current time
     now = time(NULL);
     if (NULL == localtime_r(&now, &time_info))
     {
         return false;
     }
     
     // Use provided format or default
     const char *p_format = (NULL != p_format_str && p_format_str[0] != '\0') 
                           ? p_format_str 
                           : p_server_cfg->time_format;
     
     // Format time
     size_t result = strftime(p_output, output_size, p_format, &time_info);
     
     return (result > 0);
 }
 
 /*** end of file ***/