#ifndef PROCESS_LIFECYCLE_H
#define PROCESS_LIFECYCLE_H

#include "process_manager/process_manager.h"

/**
 * @brief HTTP Server process entry point
 * @param context Context pointer (currently unused)
 * @return Exit code (0 on success, positive on error)
 */
int http_server_process(void *context);

/**
 * @brief Energy Advisor process entry point
 * @param context Context pointer (currently unused)
 * @return Exit code (0 on success, negative on error)
 */
int energy_advisor_run(void *context);

/**
 * @brief Process Manager child process entry point
 * @param process_manager Pointer to uninitialized ProcessManager
 * @return Exit code (0 on success, negative on error)
 * 
 * This function:
 * - Initializes the process manager
 * - Spawns HTTP server, fetcher, and energy advisor processes
 * - Monitors fetcher output and sends acknowledgments
 * - Handles shutdown and cleanup
 */
int run_process_manager_child(ProcessManager *process_manager);

#endif /* PROCESS_LIFECYCLE_H */
