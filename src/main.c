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
#include "energy_advisor/energy_advisor.h"
#include "thread_pool/thread_pool.h"

bool http_server_should_quit = false;

typedef struct
{
    Logger *logger;
} HTTP_Cool_Context;

void check_signal(int signal)
{
    printf("Got signal: %d\n", signal);
    http_server_should_quit = true;
}

void help_callback(void)
{
    printf("Current arguments:\n--port(-h) <integer>\n--help(-h) for information\n--test(t) <string> to print self out\n");
    exit(0);
}

void *hellothere(void *ptr)
{
    (void)ptr;
    printf("Hello there\n");
    return NULL;
}


int main(int argc, char **argv)
{
    Thread_Pool pools;
    memset(&pools, 0, sizeof(pools));
    Thread_Pool_Create(&pools);
    
    printf("IS THREAD IN USE: %d\r\n", pools.threads[0].in_use);
    Thread_Pool_Task_Add(&pools, hellothere, NULL);
    printf("IS THREAD IN USE: %d\r\n", pools.threads[0].in_use);

    Thread_Pool_Destroy(&pools);
    printf("IS THREAD IN USE: %d\r\n", pools.threads[0].in_use);

    printf("Ending the program gracefully\n");
    return 0;
    CLI cli;
    uint16_t port = 8080;
    Energy_Status plan; 

    int port_argument_data = 0;
    char test_string[128];
    memset(test_string, 0, sizeof(test_string));

    CLI_Argument_Add(&cli, "--port", "-p", Argument_Option_Integer, &port_argument_data);
    CLI_Argument_Add(&cli, "--test", "-t", Argument_Option_String, test_string);
    CLI_Argument_Add_Callback(&cli, "--help", "-h", help_callback);

    if (CLI_Parse(&cli, argc, argv))
    {
        if (port_argument_data != 0)
            port = port_argument_data;

        if (test_string[0] != 0)
            printf("TEST STRING IS: %s\n", test_string);
    }
    else
    {
        printf("Failed to parse CLI arguments!\n");
        return -1;
    }

    printf("Hello, Energy Saver Deluxe!\n");
    Energy_Advisor_Advice(&plan);
    size_t max_connections = 1024;

    Logger logger_main = {0};
    Logger_Init(&logger_main, "Main", "logfolder", "log.txt", LOGGER_OUTPUT_TYPE_FILE_TEXT);
    Logger_Write(&logger_main, "%s", "Hello");

    pid_t http_server_pid;
    http_server_pid = fork();
    if(http_server_pid > 0)
    {
        // Parent-case        
        Logger logger = {0};

        Logger_Init(&logger, "Parent", "logfolder", "log.txt", LOGGER_OUTPUT_TYPE_FILE_TEXT);
        Logger_Write(&logger, "%s", "Hello");

        char buffer[32] = {0};
        bool should_quit = false;
        while(should_quit == false)
        {
            fgets(buffer, sizeof(buffer), stdin);
            if(strncmp(buffer, "q", 1) == 0)
            {
                if(kill(http_server_pid, SIGTERM) == -1)
                {
                    Logger_Write(&logger, "Error: Did not correctly kill the process: %d", errno);
                    return -4;
                }              

                int stat_loc;
                waitpid(http_server_pid, &stat_loc, 0);
                printf("stat loc: %d\n", stat_loc);

                should_quit = true;
            } 
        }
        Logger_Write(&logger, "Goodbye, process done.");
        Logger_Dispose(&logger);
    }
    else if(http_server_pid == 0)
    {
        // Child-case
        Logger logger = {0};    

        Logger_Init(&logger, "Child", NULL, NULL, LOGGER_OUTPUT_TYPE_CONSOLE);
        Logger_Write(&logger, "%s", "Hello");
        
        signal(SIGQUIT, check_signal);
        signal(SIGTERM, check_signal);
        signal(SIGKILL, check_signal);

        HTTP_Server http_server;

        HTTP_Cool_Context cool_context;
        cool_context.logger = &logger;
        if(HTTP_Server_Initialize(&http_server, max_connections, &cool_context) == false)
        {
            Logger_Write(&logger, "Server failed to initialize");
            return -1;
        }

        /* Register valid routes */
        HTTP_Server_Register_Route(&http_server, "/", HTTP_METHOD_GET, root_handler_handle);

        if(HTTP_Server_Start(&http_server, port) == false)
        {
            Logger_Write(&logger, "Server failed to start");
            return -2;
        }

        while(http_server_should_quit == false)
        {
            HTTP_Server_Work(&http_server);
        }

        Logger_Write(&logger, "Disposing HTTP Server");
        Logger_Dispose(&logger);
        HTTP_Server_Dispose(&http_server);
    }
    else
    {
        // TODO:
        // Error: negative value of pid
        // Check errno
    }
    Logger_Dispose(&logger_main);
    return 0;
}