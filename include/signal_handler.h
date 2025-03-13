/** @file signal_handler.h
 *
 * @brief Simple signal handling for the time server.
 *
 * Provides basic signal handling functionality for graceful
 * shutdown of the time server when receiving termination signals.
 */

#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <stdbool.h>

/**
 * @brief Initialize signal handlers for graceful shutdown
 *
 * Sets up handlers for SIGINT and SIGTERM to allow the server
 * to shut down gracefully when these signals are received.
 *
 * @return true if signal handlers were successfully installed, false otherwise
 */
bool signal_handler_init (void);

/**
 * @brief Check if a shutdown has been requested via signal
 *
 * This function should be called periodically from the main loop
 * to check if a termination signal has been received.
 *
 * @return true if shutdown has been requested, false otherwise
 */
bool signal_handler_shutdown_requested (void);

/**
 * @brief Reset signal handlers to their default state
 *
 * Should be called during server shutdown to restore default signal handlers.
 *
 * @return true if cleanup was successful, false otherwise
 */
bool signal_handler_cleanup (void);

#endif /* SIGNAL_HANDLER_H */