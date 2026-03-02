#ifndef PARENT_SUPERVISOR_H
#define PARENT_SUPERVISOR_H

#include <sys/types.h>

/**
 * @brief Run the parent process loop that supervises the process manager
 * @param process_manager_pid PID of the child process manager
 * @return Exit code (0 on success, negative on error)
 * 
 * Reads from stdin and waits for 'q' command to initiate shutdown.
 * Sends SIGTERM to the process manager and waits for it to exit.
 */
int run_parent_loop(pid_t process_manager_pid);

#endif /* PARENT_SUPERVISOR_H */
