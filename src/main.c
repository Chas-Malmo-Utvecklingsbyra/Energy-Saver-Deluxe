#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include "http/server/http_server.h"
#include "logger/logger.h"
#include "routes/root_handler.h"
#include "cli/cli.h"
#include "process_manager/process_manager.h"
#include "config/config.h"
#include "file_helper/file_helper.h"

bool http_server_should_quit = false;
bool process_manager_should_quit = false;

typedef struct
{
    Logger *logger;
} HTTP_Cool_Context;

void check_signal(int signal)
{
    printf("Got signal: %d\n", signal);
    http_server_should_quit = true;
}

void process_manager_signal_handler(int signal)
{
    printf("Process Manager received signal: %d\n", signal);
    process_manager_should_quit = true;
}

void help_callback(void)
{
    printf("Current arguments:\n--port(-h) <integer>\n--help(-h) for information\n--test(t) <string> to print self out\n");
    exit(0);
}

static int parse_command_args(const char *cmd_string, char ***args_out)
{
    if (!cmd_string || !args_out)
        return 0;

    // Copy string since we'll modify it
    char *str = strdup(cmd_string);
    if (!str)
        return 0;

    // Count args first (rough estimate)
    int max_args = 1; // Start with program name
    for (const char *p = cmd_string; *p; p++)
    {
        if (*p == ' ' || *p == '\t')
            max_args++;
    }
    max_args += 2; // Extra space + NULL terminator

    char **args = calloc(max_args, sizeof(char *));
    if (!args)
    {
        free(str);
        return 0;
    }

    int argc = 0;
    char *p = str;

    while (*p)
    {
        // Skip whitespace
        while (*p == ' ' || *p == '\t')
            p++;
        if (!*p)
            break;

        char *start = p;

        // Handle quoted strings
        if (*p == '\'' || *p == '"')
        {
            char quote = *p++;
            start = p;
            while (*p && *p != quote)
                p++;
            if (*p)
            {
                *p++ = '\0';
            }
        }
        else
        {
            // Regular token
            while (*p && *p != ' ' && *p != '\t')
                p++;
            if (*p)
                *p++ = '\0';
        }

        args[argc++] = strdup(start);
    }

    args[argc] = NULL;
    free(str);

    *args_out = args;
    return argc;
}

// Function to free the args array
static void free_args(char **args)
{
    if (!args)
        return;
    for (int i = 0; args[i]; i++)
    {
        free(args[i]);
    }
    free(args);
}

// HTTP Server process entry point
int http_server_process(void *context)
{
    HTTP_Cool_Context *cool_context = (HTTP_Cool_Context *)context;

    Logger logger = {0};
    Logger_Init(&logger, "HTTP-Server", NULL, NULL, LOGGER_OUTPUT_TYPE_CONSOLE);

    signal(SIGQUIT, check_signal);
    signal(SIGTERM, check_signal);
    
    HTTP_Server http_server;

    if (HTTP_Server_Initialize(&http_server, 1024, cool_context) == false)
    {
        Logger_Write(&logger, "Server failed to initialize");
        return 1;
    }

    HTTP_Server_Register_Route(&http_server, "/", HTTP_METHOD_GET, root_handler_handle);

    if (HTTP_Server_Start(&http_server, 8080) == false)
    {
        Logger_Write(&logger, "Server failed to start");
        return 2;
    }

    while (http_server_should_quit == false)
    {
        HTTP_Server_Work(&http_server);
    }

    Logger_Write(&logger, "Disposing HTTP Server");
    HTTP_Server_Dispose(&http_server);

    return 0;
}

// int process_manager_process(void *context)
// {
//     Logger logger = {0};
//     Logger_Init(&logger, "Process Manager", NULL, NULL, LOGGER_OUTPUT_TYPE_CONSOLE);
//     Logger_Write(&logger, "%s", "Process Manager started");

//     ProcessManager process_manager;
//     if (!ProcessManager_Init(&process_manager, &logger))
//     {
//         Logger_Write(&logger, "Failed to initialize Process Manager");
//         return -1;
//     }
    
//     HTTP_Cool_Context cool_context = { .logger = &logger };
//     pid_t server_pid = ProcessManager_Spawn(&process_manager, "HTTP Server", http_server_process, &cool_context, false);

//     if (server_pid < 0)
//     {
//         Logger_Write(&logger, "Failed to spawn HTTP Server process");
//         return -1;
//     }
//     else
//     {
//         Logger_Write(&logger, "Spawned HTTP Server process with PID %d", server_pid);
//     }



//     ProcessManager_Destroy(&process_manager);
//     Logger_Dispose(&logger);
    
//     return 0;
// }

//TODO Test with execve to run a different binary as child process, http request service with args
int main(int argc, char **argv)
{
    CLI cli;
    int port_argument_data = 0;
    char test_string[128];
    memset(test_string, 0, sizeof(test_string));

    CLI_Argument_Add(&cli, "--port", "-p", Argument_Option_Integer, &port_argument_data);
    CLI_Argument_Add(&cli, "--test", "-t", Argument_Option_String, test_string);
    CLI_Argument_Add_Callback(&cli, "--help", "-h", help_callback);

    if (!CLI_Parse(&cli, argc, argv))
    {
        printf("Failed to parse CLI arguments!\n");
        return -1;
    }

    char cwd[255];
    memset(cwd, 0, sizeof(cwd));

    if (getcwd(cwd, sizeof(cwd)) == NULL)
    {
        printf("Could not get working directory!\r\n");
        return -1;
    }

    char full_path[260];
    memset(full_path, 0, sizeof(full_path));
    snprintf(full_path, sizeof(full_path), "%s/data", cwd);

    if (!File_Helper_Dir_Exists(full_path))
    {
        File_Helper_Create_Dir(full_path);
    }

    ProcessManager process_manager;
    pid_t process_manager_pid;
    
    process_manager_pid = fork();
    if (process_manager_pid > 0)
    {
        // Parent-case
        char buffer[32] = {0};
        bool should_quit = false;
        while (should_quit == false)
        {
            fgets(buffer, sizeof(buffer), stdin);
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
    }
    else if (process_manager_pid == 0)
    {
        // Child-case
        signal(SIGTERM, process_manager_signal_handler);
        signal(SIGINT, process_manager_signal_handler);
        
        Logger logger_process = {0};
        Logger_Init(&logger_process, "Process Manager", NULL, NULL, LOGGER_OUTPUT_TYPE_CONSOLE);
        Logger_Write(&logger_process, "%s", "Process Manager started");

        if (!ProcessManager_Init(&process_manager, &logger_process))
        {
            Logger_Write(&logger_process, "Failed to initialize Process Manager");
            return -1;
        }
    
        HTTP_Cool_Context cool_context = { .logger = &logger_process };
        pid_t server_pid = ProcessManager_Spawn(&process_manager, "HTTP Server", http_server_process, &cool_context, false);

        if (server_pid < 0)
        {
            Logger_Write(&logger_process, "Failed to spawn HTTP Server process");
            return -1;
        }
        else
        {
            Logger_Write(&logger_process, "Spawned HTTP Server process with PID %d", server_pid);
        }

        Config_t *cfg = Config_Get_Instance("settings.json");
        if (cfg == NULL)
        {
            printf("Failed to load configuration!\n");
            exit(-1);
        }

        char *fetcher_exec_path = Config_Get_Field_Value_String(cfg, "fetcher_exec_path");
        size_t fetcher_command_count = Config_Get_Field_Value_Integer(cfg, "fetchers_commands_count", NULL);
        
        for (size_t i = 0; i < fetcher_command_count; i++)
        {
            char *cmd_args_string = Config_Get_Field_Value_From_String_Array(cfg, "fetchers_commands_args", i);
        
            char **args = NULL;
            parse_command_args(cmd_args_string, &args);
        
            pid_t fetcher_pid = ProcessManager_SpawnByExecutable(&process_manager, "fetchers_commands_args", fetcher_exec_path, args, false);
        
            if (fetcher_pid < 0)
            {
                Logger_Write(&logger_process, "Failed to spawn fetcher process");
                return -1;
            }
            else
            {
                Logger_Write(&logger_process, "Spawned fetcher process with PID %d", fetcher_pid);
            }
            
            if (args != NULL)
                free_args(args);
        }

        // Wait for child processes to finish or termination signal
        while (!process_manager_should_quit)
        {
            sleep(1);
        }

        Logger_Write(&logger_process, "Process Manager shutting down...");
        ProcessManager_TerminateAll(&process_manager);
        
        // Wait for all child processes to terminate
        Logger_Write(&logger_process, "Waiting for child processes to exit...");
        int status;
        while (wait(&status) > 0)
        {
            // Reap all child processes
        }
        
        ProcessManager_Destroy(&process_manager);
        Logger_Dispose(&logger_process);
    }
    return 0;
}
//pid_t http_server_pid;
//http_server_pid = fork();
//     if(http_server_pid > 0)
//     {
//         // Parent-case        
//         Logger logger = {0};

//         Logger_Init(&logger, "Parent", "logfolder", LOGGER_OUTPUT_TYPE_FILE_TEXT);
//         Logger_Write(&logger, "%s", "Hello");

//         char buffer[32] = {0};
//         bool should_quit = false;
//         while(should_quit == false)
//         {
//             fgets(buffer, sizeof(buffer), stdin);
//             if(strncmp(buffer, "q", 1) == 0)
//             {
//                 if(kill(http_server_pid, SIGTERM) == -1)
//                 {
//                     Logger_Write(&logger, "Error: Did not correctly kill the process: %d", errno);
//                     return -4;
//                 }              

//                 int stat_loc;
//                 waitpid(http_server_pid, &stat_loc, 0);
//                 printf("stat loc: %d\n", stat_loc);

//                 should_quit = true;
//             } 
//         }
//         Logger_Write(&logger, "Goodbye, process done.");
//         Logger_Dispose(&logger);
//     }
//     else if(http_server_pid == 0)
//     {
//         // Child-case
//         Logger logger = {0};    

//         Logger_Init(&logger, "Child", NULL, LOGGER_OUTPUT_TYPE_CONSOLE);
//         Logger_Write(&logger, "%s", "Hello");
        
//         signal(SIGQUIT, check_signal);
//         signal(SIGTERM, check_signal);
//         signal(SIGKILL, check_signal);

//         HTTP_Server http_server;

//         HTTP_Cool_Context cool_context;
//         cool_context.logger = &logger;
//         if(HTTP_Server_Initialize(&http_server, max_connections, &cool_context) == false)
//         {
//             Logger_Write(&logger, "Server failed to initialize");
//             return -1;
//         }

//         /* Register valid routes */
//         HTTP_Server_Register_Route(&http_server, "/", HTTP_METHOD_GET, root_handler_handle);

//         if(HTTP_Server_Start(&http_server, port) == false)
//         {
//             Logger_Write(&logger, "Server failed to start");
//             return -2;
//         }

//         while(http_server_should_quit == false)
//         {
//             HTTP_Server_Work(&http_server);
//         }

//         Logger_Write(&logger, "Disposing HTTP Server");
//         Logger_Dispose(&logger);
//         HTTP_Server_Dispose(&http_server);
//     }
//     else
//     {
//         // TODO:
//         // Error: negative value of pid
//         // Check errno
//     }
//     Logger_Dispose(&logger_main);
//     return 0;
// }