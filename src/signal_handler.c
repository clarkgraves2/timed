/** @file signal_handler.c
 *
 * @brief Implementation of simple signal handling for the time server.
 *
 * Implements basic signal handling to allow for graceful shutdown
 * when termination signals (SIGINT, SIGTERM) are received.
 */

#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <string.h>

#include "../include/signal_handler.h"
#include "../include/syslog.h"

/*************************************************************************
 * Static Variables
 *************************************************************************/

/**
 * Justification for clang-tidy suppression:
 * Both variables are only accessed and used withing this module.
 * They are used to coordinate withing the functions of the module.
 */
// NOLINTBEGIN(cppcoreguidelines-avoid-non-const-global-variables)
static volatile sig_atomic_t shutdown_requested = 0;
static volatile sig_atomic_t initialized        = 0;
// NOLINTEND(cppcoreguidelines-avoid-non-const-global-variables)

/*************************************************************************
 * Static Function Prototypes
 *************************************************************************/

/**
 * @brief Signal handler function
 *
 * Sets the shutdown flag when a termination signal is received.
 * This function only performs async-signal-safe operations.
 *
 * @param[in] signo Signal number received
 */
static void 
signal_handler (int signo);

/*************************************************************************
 * Function Implementations
 *************************************************************************/

bool
signal_handler_init (void)
{
    struct sigaction sig_action;

    if (initialized)
    {
        syslog_write (
            WARNING,
            "Signal handler already initialized - skipping initialization");
        return false;
    }

    syslog_write (INFO, "Initializing signal handlers for SIGINT and SIGTERM");

    // Configure signal handler
    memset (&sig_action, 0, sizeof (sig_action));
    sig_action.sa_handler = signal_handler;
    sigemptyset (&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    // Install handlers for termination signals
    if (sigaction (SIGINT, &sig_action, NULL) == -1)
    {
        syslog_write (
            ERROR, "Failed to register SIGINT handler: %s", strerror (errno));
        return false;
    }

    if (sigaction (SIGTERM, &sig_action, NULL) == -1)
    {
        syslog_write (
            ERROR, "Failed to register SIGTERM handler: %s", strerror (errno));
        return false;
    }

    initialized = 1;
    syslog_write (
        INFO, "Signal handlers successfully initialized for graceful shutdown");

    return true;
}

bool
signal_handler_shutdown_requested (void)
{
    // Log first time detecting a shutdown request
    if (shutdown_requested && initialized)
    {
        static volatile sig_atomic_t shutdown_logged = 0;

        if (!shutdown_logged)
        {
            syslog_write (
                INFO,
                "Shutdown requested via signal, initiating graceful shutdown");
            shutdown_logged = 1;
        }
    }

    return (shutdown_requested != 0);
}

bool
signal_handler_cleanup (void)
{
    struct sigaction sig_action;

    if (!initialized)
    {
        syslog_write (WARNING,
                      "Attempting to clean up uninitialized signal handler");
        return false;
    }

    syslog_write (INFO, "Cleaning up signal handlers");

    // Reset to default all signals we handle
    memset (&sig_action, 0, sizeof (sig_action));
    sig_action.sa_handler = SIG_DFL;
    sigemptyset (&sig_action.sa_mask);
    sig_action.sa_flags = 0;

    // Restore default handlers
    if (sigaction (SIGINT, &sig_action, NULL) == -1)
    {
        syslog_write (WARNING,
                      "Failed to restore default SIGINT handler: %s",
                      strerror (errno));
    }

    if (sigaction (SIGTERM, &sig_action, NULL) == -1)
    {
        syslog_write (WARNING,
                      "Failed to restore default SIGTERM handler: %s",
                      strerror (errno));
    }

    initialized = 0;
    syslog_write (INFO, "Signal handlers successfully reset to default state");

    return true;
}

/*************************************************************************
 * Private Functions
 *************************************************************************/

static void
signal_handler (int signo)
{
    shutdown_requested = 1;

    // The variable for the signal is not used but required for sigaction
    // function.
    (void) signo;
}

/*** end of file ***/
