#define _POSIX_C_SOURCE 200809L

#include "parent_supervisor.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>

int run_parent_loop(pid_t process_manager_pid)
{
    char buffer[32] = {0};
    bool should_quit = false;
    
    while (should_quit == false)
    {
        if (fgets(buffer, sizeof(buffer), stdin) == NULL)
            continue;
            
        if (strncmp(buffer, "q", 1) == 0)
        {
            // Send SIGTERM to child process - let child handle cleanup
            if (kill(process_manager_pid, SIGTERM) == -1)
            {
                printf("Error: Did not correctly kill the process: %d\n", errno);
                return -4;
            }

            int stat_loc;
            waitpid(process_manager_pid, &stat_loc, 0);
            printf("stat loc: %d\n", stat_loc);
            should_quit = true;
        }
    }
    
    printf("Goodbye, process done.\n");
    return 0;
}
