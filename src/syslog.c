/** @file syslog.c
 *
 * @brief Implementation of simplified thread-safe logging
 *
 * This module implements a thread-safe logging system that writes
 * timestamped log entries. It supports multiple message types
 * and ensures thread safety through mutex synchronization.
 *
 */

#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "../include/syslog.h"

/*************************************************************************
 * Constants and Macros
 *************************************************************************/

// Maximum size of formatted message buffer
#define MESSAGE_BUFFER          (1024)

// Size of timestamp string buffer
#define SYSLOG_TIMESTAMP_SIZE   (32)

// Format string for timestamp
#define SYSLOG_TIMESTAMP_FORMAT ("%Y-%m-%d %H:%M:%S")

// For SYSLOG type validation
#define SYSLOG_MAX_TYPE         (CRITICAL)

// Log message types
static const char * const SYSLOG_TYPE_STRINGS[]
    = {"INFO", "WARNING", "ERROR", "DEBUG_LOG", "CRITICAL"};

/*************************************************************************
 * Static Variables
 *************************************************************************/
// Justification for clang-tidy suppression:
// All varibales are accessed withing this module and are used to 
// synchonize within the functions of the module.
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static FILE *          log_file    = NULL;
static bool            g_initialized = false;
static pthread_mutex_t g_log_mutex   = PTHREAD_MUTEX_INITIALIZER;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

/*************************************************************************
 * Private Function Prototypes
 *************************************************************************/

static bool            
syslog_format_message (char *        p_buffer,
                       size_t        buffer_size,
                       syslog_type_t type,
                       const char *  p_message);

/*************************************************************************
 * Function Definitions
 *************************************************************************/

/**
 * @brief Initialize the logging subsystem
 *
 * @param[in] p_config  Pointer to server configuration
 *
 * @return True if initialization successful, false otherwise
 */
bool
syslog_init (server_config_t * p_config)
{
    if (NULL == p_config)
    {
        return false;
    }

    pthread_mutex_lock (&g_log_mutex);

    if (g_initialized)
    {
        pthread_mutex_unlock (&g_log_mutex);
        return false;
    }

    // Initialize log file
    if (p_config->log_file != NULL)
    {
        log_file = fopen (p_config->log_file, "a");
        if (NULL == log_file)
        {
            pthread_mutex_unlock (&g_log_mutex);
            return false;
        }
    }
    else
    {
        // Default to stdout if no file path provided
        log_file = stdout;
    }

    g_initialized = true;
    pthread_mutex_unlock (&g_log_mutex);

    // Log initialization message
    syslog_write (INFO, "Logging system g_initialized");

    return true;
}

bool
syslog_write (syslog_type_t type, const char * p_format, ...)
{
    char    formatted_message[MESSAGE_BUFFER];
    char    display_message[MESSAGE_BUFFER];
    va_list args;
    bool    b_result         = false;
    int     vsnprintf_result = 0;

    // Basic validation
    if ((NULL == p_format) || (type > SYSLOG_MAX_TYPE) || !g_initialized)
    {
        return false;
    }

     /*
     * Justification for clang-tidy suppression 
     * This is a false positive.
     * It has been initialized.
     * 1. Initializing the va_list with va_start()
     * 2. Using it in vsnprintf()
     * 3. Cleaning up with va_end()
     */
    // Format message with variable arguments
    va_start (args, p_format);
    // NOLINTNEXTLINE(clang-analyzer-valist.Uninitialized)
    vsnprintf_result = vsnprintf (
        formatted_message, sizeof (formatted_message), p_format, args);
    va_end (args);

    // Check for formatting errors or truncation
    if (vsnprintf_result < 0
        || (size_t) vsnprintf_result >= sizeof (formatted_message))
    {
        return false;
    }

    pthread_mutex_lock (&g_log_mutex);

    // Format the message with timestamp and type
    if (!syslog_format_message (
            display_message, sizeof (display_message), type, formatted_message))
    {
        pthread_mutex_unlock (&g_log_mutex);
        return false;
    }

    // Write message to log file
    if (log_file != NULL)
    {
        if (fprintf (log_file, "%s", display_message) < 0)
        {
            pthread_mutex_unlock (&g_log_mutex);
            return false;
        }

        // Use void cast to silence warning about unused return value
        (void) fflush (log_file);
        b_result = true;
    }

    pthread_mutex_unlock (&g_log_mutex);
    return b_result;
}

bool
syslog_shutdown (void)
{
    pthread_mutex_lock (&g_log_mutex);

    if (!g_initialized)
    {
        pthread_mutex_unlock (&g_log_mutex);
        return false;
    }

    // Close file if open and not stdout
    if (log_file != NULL && log_file != stdout)
    {
        // Use void cast to silence warning about unused return value
        (void) fclose (log_file);
        log_file = NULL;
    }

    g_initialized = false;

    pthread_mutex_unlock (&g_log_mutex);
    return true;
}

/*************************************************************************
 * Private Function Definitions
 *************************************************************************/

/**
 * @brief Formats a log message with timestamp and type
 *
 * @param[out] p_buffer     Buffer to store formatted message
 * @param[in]  buffer_size  Size of buffer
 * @param[in]  type         Log message type
 * @param[in]  p_message    Message content
 *
 * @return True if formatting successful
 */
static bool
syslog_format_message (char *        p_buffer,
                       size_t        buffer_size,
                       syslog_type_t type,
                       const char *  p_message)
{
    struct tm time_info;
    time_t    now = time (NULL);
    char      timestamp[SYSLOG_TIMESTAMP_SIZE];
    int       written = 0;

    if (NULL == p_buffer || buffer_size == 0 || NULL == p_message
        || type > CRITICAL || now == (time_t) -1)
    {
        return false;
    }

    // Get current time
    if (NULL == localtime_r (&now, &time_info))
    {
        return false;
    }

    // Format timestamp
    if (0
        == strftime (
            timestamp, sizeof (timestamp), SYSLOG_TIMESTAMP_FORMAT, &time_info))
    {
        return false;
    }

    // Format message with timestamp and type
    written = snprintf (p_buffer,
                        buffer_size,
                        "[%s][%s] %s\n",
                        timestamp,
                        SYSLOG_TYPE_STRINGS[type],
                        p_message);

    return (written > 0 && (size_t) written < buffer_size);
}

/*** end of file ***/
