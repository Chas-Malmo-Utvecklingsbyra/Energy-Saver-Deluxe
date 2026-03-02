#ifndef SIGNAL_HANDLERS_H
#define SIGNAL_HANDLERS_H

#include <signal.h>

// Quit flags for each process type
extern volatile sig_atomic_t http_server_should_quit;
extern volatile sig_atomic_t process_manager_should_quit;
extern volatile sig_atomic_t energy_advisor_should_quit;

/**
 * @brief Setup signal handlers for HTTP server process
 */
void setup_http_server_signals(void);

/**
 * @brief Setup signal handlers for Energy Advisor process
 */
void setup_energy_advisor_signals(void);

/**
 * @brief Setup signal handlers for Process Manager process
 */
void setup_process_manager_signals(void);

#endif /* SIGNAL_HANDLERS_H */
