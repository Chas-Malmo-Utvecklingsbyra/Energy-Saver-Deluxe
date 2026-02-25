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
#include "energy_advisor/energy_advisor.h"

#define ENERGY_ADVISOR_CHECK_INTERVAL_SECONDS 60

bool http_server_should_quit = false;
bool process_manager_should_quit = false;
bool energy_advisor_should_quit = false;

typedef struct
{
    Logger *logger;
} HTTP_Cool_Context;

void check_signal_http_server(int signal)
{
    (void)signal;
    //printf("Got signal: %d\n", signal);
    http_server_should_quit = true;
}

void check_signal_process_manager(int signal)
{
    (void)signal;
    //printf("Process Manager received signal: %d\n", signal);
    process_manager_should_quit = true;
}

void check_signal_energy_advisor(int signal)
{
    (void)signal;
    //printf("Energy Advisor received signal: %d\n", signal);
    energy_advisor_should_quit = true;
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

    signal(SIGQUIT, check_signal_http_server);
    signal(SIGTERM, check_signal_http_server);
    
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

    Logger_Dispose(&logger);
    return 0;
}

int energy_advisor_start(void *context)
{   
    (void)context;

    signal(SIGQUIT, check_signal_energy_advisor);
    signal(SIGTERM, check_signal_energy_advisor);

    Config_t *cfg = Config_Get_Instance(NULL);

    size_t fetcher_command_count = Config_Get_Field_Value_Integer(cfg, "fetchers_commands_count", NULL);
    char **args = NULL;

    bool first_file_exists = false;
    bool second_file_exists = false;

    while(energy_advisor_should_quit == false)
    {
        for (size_t i = 0; i < fetcher_command_count; i++)
        {
            char *directory;
            char *filename;
            char *cmd_args_string = Config_Get_Field_Value_From_String_Array(cfg, "fetchers_commands_args", i);
            parse_command_args(cmd_args_string, &args);

            for (int j = 0; args[j]; j++)
            {   
                if (strcmp(args[j], "-o") == 0 && args[j + 1])
                {
                    directory = args[j + 1];
                }

                if (strcmp(args[j], "-n") == 0 && args[j + 1])
                {
                    filename = args[j + 1];
                }
            }

            char full_path[128];
            snprintf(full_path, sizeof(full_path), "%s/%s", directory, filename);
            
            if (File_Helper_File_Exists(full_path))
            {
                if (i == 0)
                {
                    first_file_exists = true;
                }
                else 
                {
                    second_file_exists = true;
                }
            }

            if (args != NULL)
                free_args(args);
        }

        if (first_file_exists == true && second_file_exists == true)
        {
            Energy_Status status = Energy_Advisor_Advice();
            if (status != ENERGY_STATUS_OK)
            {
                printf("Energy Advice data is missing, error code: %d\n", status);
                return -1;
            }
        }
        
        first_file_exists = false;
        second_file_exists = false;

        sleep(ENERGY_ADVISOR_CHECK_INTERVAL_SECONDS);
    }
    
    return 0;
}

//TODO: FIX DATE IN ARGS FOR FETCHER, CURRENTLY HARDCODED TO TODAY, SHOULD BE DYNAMIC BASED ON REQUESTED DATE OR CURRENT DATE FOR TESTING
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

    ProcessManager process_manager;
    pid_t process_manager_pid = fork();

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
        signal(SIGTERM, check_signal_process_manager);
        signal(SIGINT, check_signal_process_manager);
        
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

        Config_t *cfg = Config_Get_Instance("settings.json");
        if (cfg == NULL)
        {
            Logger_Write(&logger_process,"Failed to load configuration!");
            exit(-1);
        }

        char *fetcher_exec_path = Config_Get_Field_Value_String(cfg, "fetcher_exec_path");
        size_t fetcher_command_count = Config_Get_Field_Value_Integer(cfg, "fetchers_commands_count", NULL);
        char **args = NULL;
        
        for (size_t i = 0; i < fetcher_command_count; i++)
        {
            char *cmd_args_string = Config_Get_Field_Value_From_String_Array(cfg, "fetchers_commands_args", i);
            parse_command_args(cmd_args_string, &args);

            // change date in args to current date for fetcher commands that need it
            for (int j = 0; args != NULL && args[j] != NULL; j++)
            {
                if (strcmp(args[j], "-u") == 0 && args[j + 1])
                {
                    if (strcmp(args[j + 1], "https://www.elprisetjustnu.se") != 0)
                    {
                        break;
                    }
                }
                if (strcmp(args[j], "-r") == 0 && args[j + 1])
                {
                    time_t t = time(NULL);
                    struct tm tm = *localtime(&t);
                    char date_buffer[64];
                    snprintf(date_buffer, sizeof(date_buffer), "/api/v1/prices/%04d/%02d-%02d_SE4.json", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
                    free(args[j + 1]);
                    args[j + 1] = strdup(date_buffer);
                }
            }

            pid_t fetcher_pid = ProcessManager_SpawnByExecutable(&process_manager, fetcher_exec_path, fetcher_exec_path, args, true);

            if (fetcher_pid < 0)
            {
                Logger_Write(&logger_process, "Failed to spawn fetcher process");
                return -1;
            }

            if (args != NULL)
                free_args(args);
        }

        
        pid_t energy_advisor_pid = ProcessManager_Spawn(&process_manager, "Energy Advisor", energy_advisor_start, NULL, false);

        if (energy_advisor_pid < 0)
        {
            Logger_Write(&logger_process, "Failed to spawn Energy Advisor process");
            return -1;
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
        Config_Instance_Dispose();
        ProcessManager_Destroy(&process_manager);
        Logger_Dispose(&logger_process);
    }
    return 0;
}