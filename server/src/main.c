/**
 * @file main.c
 * @brief Energy Saver Deluxe - Main entry point
 * 
 * This application manages multiple child processes:
 * - HTTP Server: Serves web requests
 * - Fetchers: Retrieve energy price and weather data
 * - Energy Advisor: Analyzes data and generates advice
 * 
 * All process lifecycle management is delegated to dedicated modules.
 */

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "process_manager/process_manager.h"
#include "parent_supervisor.h"
#include "process_lifecycle.h"

// TODO Add restart of processes when api error etc, logger macro?

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;
    // Fork to create parent supervisor and process manager child
    ProcessManager process_manager;
    pid_t process_manager_pid = fork();

    if (process_manager_pid > 0)
    {
        // Parent process: supervise the process manager
        return run_parent_loop(process_manager_pid);
    }
    else if (process_manager_pid == 0)
    {
        // Child process: run the process manager
        return run_process_manager_child(&process_manager);
    }
    else
    {
        // Fork failed
        perror("fork");
        return -1;
    }
}
