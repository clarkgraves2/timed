/** @file syslog.h
 *
 * @brief Thread-safe simplified logging system
 *
 * This module implements a thread-safe logging system that writes
 * timestamped log entries. It supports multiple message types
 * and ensures thread safety through mutex synchronization.
 *
 */

 #ifndef SYSLOG_H
 #define SYSLOG_H
 
 #include <stdbool.h>
 #include <stddef.h>
 #include "./config.h"
 
 /**
  * Log message severity types
  */
 typedef enum 
 {
     INFO = 0,    // Informational message 
     WARNING,     // Warning condition 
     ERROR,       // Error condition 
     DEBUG_LOG,       // Debug-level message 
     CRITICAL     // Critical condition 
 } syslog_type_t;
 
 
 /**
  * @brief Initialize the system logging facility
  *
  * Opens the log file specified in the configuration.
  * Must be called before any other syslog functions.
  *
  * @param[in] p_config Configuration parameters for the server
  *
  * @return true if initialization successful
  * @retval false if already initialized, configuration invalid, or file open fails
  */
 bool syslog_init(server_config_t * p_config);
 
 /**
  * @brief Writes a formatted log message with timestamp and type
  *
  * Takes a message type, format string and variable arguments.
  * Formats and writes a timestamped log entry.
  * Thread-safe via mutex protection.
  *
  * @param[in] type Log message type (INFO, ERROR, etc)
  * @param[in] p_format Printf-style format string
  * @param[in] ... Variable arguments for format string
  *
  * @return true if write successful
  * @retval false if format string is NULL, message type invalid, logger not initialized
  */
 bool syslog_write(syslog_type_t type, const char * p_format, ...);
 
 /**
  * @brief Shuts down the logging system
  *
  * Closes the log file and cleans up resources.
  *
  * @return true if shutdown successful
  * @retval false if logger not initialized
  */
 bool syslog_shutdown(void);
 
 #endif /* SYSLOG_H */
 
 /*** end of file ***/