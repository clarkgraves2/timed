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
#define MAX_FORMAT_LENGTH      (1000)
#define FORMAT_TRUNCATE_LENGTH (255)
#define TRUNCATED_BUFFER_SIZE  (256)
#define FORMAT_DISPLAY_LENGTH  (100)
#define MAX_BUFFER_SIZE (4096)

/*************************************************************************
* Module State Structure
*************************************************************************/
typedef struct {
    struct pollfd *poll_fds;
    size_t num_fds;
    const socket_descriptor_t *socket_desc;
    const server_config_t *server_cfg;
    bool initialized;
} poll_state_t;

/*************************************************************************
* Justification for clang-tidy global suppression:
* This variable implements a singleton module pattern in C. The poll subsystem
* is designed as a singleton with internal state that must persist between
* calls to poll_init, poll_run, and poll_cleanup. 
*
* The variable is:
* 1. Static - restricting scope to this module only
* 2. Encapsulated - only accessed through public API functions
* 3. Protected - initialization state is checked before use
* 
* No thread-safety issues exist since the server is single-threaded.
* Making this const would require complex and potentially unsafe const-casting.
**************************************************************************/
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static poll_state_t g_poll_state = {0};
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

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
    
    // Check if already initialized to prevent double initialization
    if (g_poll_state.initialized) 
    {
        syslog_write(ERROR, "Poll subsystem already initialized");
        return false;
    }
    
    // Store config and descriptors
    g_poll_state.server_cfg = p_config;
    g_poll_state.socket_desc = p_descriptors;
    
    // Allocate poll fds array
    g_poll_state.num_fds = 2; // TCP and UDP sockets
    g_poll_state.poll_fds = calloc(g_poll_state.num_fds, sizeof(struct pollfd));
    if (NULL == g_poll_state.poll_fds)
    {
        syslog_write(CRITICAL, "Failed to allocate memory for poll fds");
        return false;
    }
    
    // Set up TCP socket for polling
    g_poll_state.poll_fds[0].fd = p_descriptors->tcp_socket;
    g_poll_state.poll_fds[0].events = POLLIN;
    
    // Set up UDP socket for polling
    g_poll_state.poll_fds[1].fd = p_descriptors->udp_socket;
    g_poll_state.poll_fds[1].events = POLLIN;
    
    // Mark as initialized
    g_poll_state.initialized = true;
    
    syslog_write(INFO, "Poll subsystem initialized with %zu fds", g_poll_state.num_fds);
    return true;
}

bool 
poll_run(void)
{
    if (!g_poll_state.initialized || NULL == g_poll_state.poll_fds)
    {
        syslog_write(ERROR, "Poll subsystem not initialized");
        return false;
    }
    
    syslog_write(INFO, "Starting main server loop");
    
    while (!signal_handler_shutdown_requested())
    {
        // Wait for events
        int poll_result = poll(g_poll_state.poll_fds, 
                              (nfds_t)g_poll_state.num_fds, 
                              g_poll_state.server_cfg->poll_timeout);
        
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
        if (g_poll_state.poll_fds[0].revents & POLLIN)
        {
            handle_tcp_connection(g_poll_state.socket_desc->tcp_socket);
        }
        
        // Check for UDP events
        if (g_poll_state.poll_fds[1].revents & POLLIN)
        {
            handle_udp_datagram(g_poll_state.socket_desc->udp_socket);
        }
    }
    
    syslog_write(INFO, "Server loop terminated");
    return true;
}

bool 
poll_cleanup(void)
{
    // We can still call cleanup on an uninitialized module, but log a warning
    if (!g_poll_state.initialized) 
    {
        syslog_write(WARNING, "Attempt to clean up uninitialized poll subsystem");
    }

    // Free resources if allocated
    if (NULL != g_poll_state.poll_fds)
    {
        free(g_poll_state.poll_fds);
        g_poll_state.poll_fds = NULL;
    }
    
    // Reset all state
    g_poll_state.num_fds = 0;
    g_poll_state.socket_desc = NULL;
    g_poll_state.server_cfg = NULL;
    g_poll_state.initialized = false;
    
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
      syslog_write(INFO, "Received format string from TCP client: '%s', length: %ld", buffer, bytes_read);
   }
   else
   {
      syslog_write(INFO, "Received empty format string from TCP client (bytes_read=%ld)", bytes_read);
   }

   // Format time according to request (or use default if empty)
   if (!format_time(bytes_read > 0 ? buffer : NULL, time_buffer, sizeof(time_buffer)))
   {
      syslog_write(ERROR, "Failed to format time for TCP client");
      close(client_fd);
      return;
   }

   // Debug: Log the formatted time
   syslog_write(INFO, "Sending formatted time to TCP client: '%s'", time_buffer);

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
   ssize_t bytes_read = 0;
   
   // Receive datagram - make sure we can handle up to MTU size
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
       // Log only the first 100 chars for very large format strings
       if (bytes_read > FORMAT_DISPLAY_LENGTH) {
           char truncated[FORMAT_DISPLAY_LENGTH];
           strncpy(truncated, buffer, FORMAT_DISPLAY_LENGTH);
           truncated[FORMAT_DISPLAY_LENGTH] = '\0';
           syslog_write(INFO, "Received long format string from UDP client: '%s...' (truncated), length: %ld", 
                       truncated, bytes_read);
       } else {
           syslog_write(INFO, "Received format string from UDP client: '%s', length: %ld", buffer, bytes_read);
       }
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
    const char *p_format = NULL; // Initialize to NULL
    char truncated_format[TRUNCATED_BUFFER_SIZE] = {0}; // Fixed size with constant
    
    // Validate our module state and parameters
    if (!g_poll_state.initialized || NULL == g_poll_state.server_cfg)
    {
        syslog_write(ERROR, "Poll subsystem not initialized properly");
        return false;
    }
    
    if (NULL == p_output || output_size == 0 || now == (time_t)-1)
    {
        return false;
    }
    
    // Get current time
    if (NULL == localtime_r(&now, &time_info))
    {
        return false;
    }
    
    // Check if format string is just a newline or only whitespace
    bool is_only_whitespace = true;
    if (p_format_str != NULL) 
    {
        size_t idx = 0;
        while (p_format_str[idx] != '\0') 
        {
            if (p_format_str[idx] != ' ' && p_format_str[idx] != '\t' && 
                p_format_str[idx] != '\n' && p_format_str[idx] != '\r')
            {
                is_only_whitespace = false;
                break;
            }
            idx++;
        }
    }
    
    // Use provided format or default
    p_format = (NULL != p_format_str && p_format_str[0] != '\0' && !is_only_whitespace) 
                ? p_format_str 
                : g_poll_state.server_cfg->time_format;
    
    // Check if format string is too long to display in logs
    if (p_format_str != NULL && !is_only_whitespace) {
        size_t format_len = strlen(p_format_str);
        
        // Truncate very long format strings for safety
        if (format_len > MAX_FORMAT_LENGTH)
        {
            // Truncate format string to prevent buffer overflow
            strncpy(truncated_format, p_format_str, FORMAT_TRUNCATE_LENGTH);
            truncated_format[FORMAT_TRUNCATE_LENGTH] = '\0';
            p_format = truncated_format;
            
            syslog_write(WARNING, "Format string too long (%zu bytes), truncated", format_len);
        }
        
        // For logging, truncate display if it's too long
        if (format_len > FORMAT_DISPLAY_LENGTH)
        {
            char display_format[FORMAT_DISPLAY_LENGTH + 1];
            strncpy(display_format, p_format_str, FORMAT_DISPLAY_LENGTH);
            display_format[FORMAT_DISPLAY_LENGTH] = '\0';
            syslog_write(INFO, "Using provided format (truncated): '%s...'", display_format);
        }
        else
        {
            syslog_write(INFO, "Using provided format: '%s'", p_format_str);
        }
    }
    else if (p_format_str == NULL)
    {
        syslog_write(INFO, "Format string is NULL, using default format: '%s'", g_poll_state.server_cfg->time_format);
    }
    else
    {
        syslog_write(INFO, "Format string is empty or whitespace, using default format: '%s'", g_poll_state.server_cfg->time_format);
    }
    
    // Format time
    size_t result = strftime(p_output, output_size, p_format, &time_info);
    
    if (result > 0)
    {
        syslog_write(INFO, "Formatted time result: '%s'", p_output);
        return true;
    }
    
    syslog_write(ERROR, "Failed to format time with format: '%s'", p_format);
    return false;
}

/*** end of file ***/
