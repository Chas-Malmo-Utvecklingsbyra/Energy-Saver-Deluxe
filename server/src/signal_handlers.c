#define _POSIX_C_SOURCE 200809L

#include "signal_handlers.h"
#include <string.h>
#include <stddef.h>

// Global quit flags for each process type
volatile sig_atomic_t http_server_should_quit = 0;
volatile sig_atomic_t process_manager_should_quit = 0;
volatile sig_atomic_t energy_advisor_should_quit = 0;

// Active quit flag pointer - set by each process
static volatile sig_atomic_t *active_quit_flag = NULL;

/**
 * @brief Generic signal handler that sets the active quit flag
 */
static void generic_quit_handler(int signal)
{
    (void)signal;
    if (active_quit_flag != NULL)
        *active_quit_flag = 1;
}

/**
 * @brief Install a signal handler without SA_RESTART to allow interruption
 */
static void install_signal_handler(int signal, void (*handler)(int))
{
    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;  // Don't use SA_RESTART - allow interruption of system calls

    sigaction(signal, &sa, NULL);
}

void setup_http_server_signals(void)
{
    active_quit_flag = &http_server_should_quit;
    install_signal_handler(SIGQUIT, generic_quit_handler);
    install_signal_handler(SIGTERM, generic_quit_handler);
}

void setup_energy_advisor_signals(void)
{
    active_quit_flag = &energy_advisor_should_quit;
    install_signal_handler(SIGQUIT, generic_quit_handler);
    install_signal_handler(SIGTERM, generic_quit_handler);
}

void setup_process_manager_signals(void)
{
    active_quit_flag = &process_manager_should_quit;
    install_signal_handler(SIGTERM, generic_quit_handler);
    install_signal_handler(SIGINT, generic_quit_handler);
}
