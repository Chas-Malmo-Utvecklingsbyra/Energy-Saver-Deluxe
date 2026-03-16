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

#include "cli/cli.h"
#include "process_manager/process_manager.h"
#include "parent_supervisor.h"
#include "process_lifecycle.h"
#include "benchmark/benchmark.h"
#include "Datatypes/Unordered_List.h"

/**
 * @brief CLI help callback
 */
static void help_callback(void)
{
    printf("Energy Saver Deluxe\n");
    printf("Usage: EnergySaverDeluxe [options]\n\n");
    printf("Options:\n");
    printf("  --port, -p <integer>  Server port (not yet implemented)\n");
    printf("  --help, -h            Show this help message\n");
    exit(0);
}

// TODO Add restart of processes when api error etc, logger macro?

int main(int argc, char **argv)
{
    // Parse command line arguments
    CLI cli = {0};
    int port_argument_data = 0;

    CLI_Argument_Add(&cli, "--port", "-p", Argument_Option_Integer, &port_argument_data);
    CLI_Argument_Add_Callback(&cli, "--help", "-h", help_callback);

    if (!CLI_Parse(&cli, argc, argv))
    {
        printf("Failed to parse CLI arguments!\n");
        return -1;
    }

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
