#define _POSIX_C_SOURCE 200809L

#include "process_lifecycle.h"
#include "signal_handlers.h"
#include "utils/arg_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/wait.h>
#include <time.h>

#include "http/server/http_server.h"
#include "logger/logger.h"
#include "routes/root_handler.h"
#include "config/config.h"
#include "file_helper/file_helper.h"
#include "energy_advisor/energy_advisor.h"

#define ENERGY_ADVISOR_CHECK_INTERVAL_SECONDS 60
#define MINUTES_TO_SECONDS(x) (x*60)


/**
 * @brief Helper function to check if a given date string (YYYY-MM-DD) matches today's date.
 * @param logger Pointer to a Logger instance for logging.
 * @param date_str Date string to compare in the format "YYYY-MM-DD".
 * @return true if the date string matches today's date, false otherwise.
 */
static bool is_date_today(const char *date_str)
{
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    char today_str[16];
    strftime(today_str, sizeof(today_str), "%Y-%m-%d", &tm);
    
    return strcmp(date_str, today_str) == 0;
}

int http_server_process(void *context)
{
    (void)context;

    Logger logger = {0};
    // Logger_Init(&logger, "HTTP-Server", NULL, NULL, LOGGER_OUTPUT_TYPE_CONSOLE);
    Logger_Init(&logger, "HTTP-Server", "logfolder", "log.txt", LOGGER_OUTPUT_TYPE_FILE_TEXT);

    setup_http_server_signals();

    HTTP_Server http_server;

    LOG_WRITE(&logger, "Initializing HTTP server.");
    if (HTTP_Server_Initialize(&http_server, 1024, NULL) == false)
    {
        LOG_WRITE(&logger, "Server failed to initialize");
        return 1;
    }
    
    LOG_WRITE(&logger, "Registering routes.");
    HTTP_Server_Register_Route(&http_server, "/", HTTP_METHOD_GET, root_handler_handle, NULL);
    HTTP_Server_Register_Route(&http_server, "/advice", HTTP_METHOD_GET, advice_handler_handle, NULL);
    HTTP_Server_Register_Route(&http_server, "/weather", HTTP_METHOD_GET, weather_handler_handle, NULL);
    HTTP_Server_Register_Route(&http_server, "/summary", HTTP_METHOD_GET, summary_handler_handle, NULL);
    HTTP_Server_Register_Route(&http_server, "/summary.html", HTTP_METHOD_GET, summary_page_handler_handle, NULL);
    
    
    LOG_WRITE(&logger, "Server starting.");
    if (HTTP_Server_Start(&http_server, 8080) == false)
    {
        LOG_WRITE(&logger, "Server failed to start");
        return 2;
    }
    
    LOG_WRITE(&logger, "Server working.");
    while (http_server_should_quit == 0)
    {
        HTTP_Server_Work(&http_server);
    }

    LOG_WRITE(&logger, "Disposing HTTP Server.");
    HTTP_Server_Dispose(&http_server);

    LOG_WRITE(&logger, "Disposing logger.");
    Logger_Dispose(&logger);
    return 0;
}

int energy_advisor_process(void *context)
{
    (void)context;

    setup_energy_advisor_signals();

    Config_t *cfg = Config_Get_Instance(NULL);

    size_t fetcher_command_count = Config_Get_Field_Value_Integer(cfg, "fetchers_commands_count", NULL);
    char **args = NULL;

    bool first_file_exists = false;
    bool second_file_exists = false;

    sleep(15);

    while (energy_advisor_should_quit == 0)
    {
        for (size_t i = 0; i < fetcher_command_count; i++)
        {
            char *directory = NULL;
            char *filename = NULL;
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

            if (directory != NULL && filename != NULL)
            {
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

        time_t current_time = time(NULL);
        struct tm *tm_info = localtime(&current_time);

        int minutes = tm_info->tm_min;
        int seconds_to_sleep = 0;
        int minutes_in_seconds = MINUTES_TO_SECONDS(minutes);

        if (minutes == 0 || minutes == 15 || minutes == 30 || minutes == 45)
        {
            sleep(60);
            break;
        }

        if (minutes > 0 && minutes < 15)
        {
            seconds_to_sleep = MINUTES_TO_SECONDS(16) - minutes_in_seconds;
        }
        else if (minutes > 15 && minutes < 30)
        {
            seconds_to_sleep = MINUTES_TO_SECONDS(31) - minutes_in_seconds;
        }
        else if (minutes > 30 && minutes < 45)
        {
            seconds_to_sleep = MINUTES_TO_SECONDS(46) - minutes_in_seconds;
        }
        else if (minutes > 45 && minutes <= 59)
        {
            seconds_to_sleep = MINUTES_TO_SECONDS(01) - minutes_in_seconds;
        }
        
        printf("Seconds to sleep [ADVICE]: %d\r\n", seconds_to_sleep);

        sleep(seconds_to_sleep);

        /* sleep(ENERGY_ADVISOR_CHECK_INTERVAL_SECONDS); */
    }

    return 0;
}

int run_process_manager_child(ProcessManager *process_manager)
{
    setup_process_manager_signals();

    Logger process_manager_logger = {0};
    // if (Logger_Init(&process_manager_logger, "Process Manager", NULL, NULL, LOGGER_OUTPUT_TYPE_CONSOLE) != LOGGER_RESULT_OK)
    if (Logger_Init(&process_manager_logger, "Process Manager", "logfolder", "log.txt", LOGGER_OUTPUT_TYPE_FILE_TEXT) != LOGGER_RESULT_OK)
    {
        printf("Failed to initialize logger for Process Manager\n");
        return -1;
    }
    
    LOG_WRITE(&process_manager_logger, "%s", "Process Manager started");

    if (!ProcessManager_Init(process_manager, &process_manager_logger))
    {
        LOG_WRITE(&process_manager_logger, "Failed to initialize Process Manager");
        return -1;
    }

    pid_t server_pid = ProcessManager_Spawn(process_manager, "HTTP Server", http_server_process, NULL, false);

    if (server_pid < 0)
    {
        LOG_WRITE(&process_manager_logger, "Failed to spawn HTTP Server process");
        return -1;
    }

    Config_t *cfg = Config_Get_Instance("settings.json");
    if (cfg == NULL)
    {
        LOG_WRITE(&process_manager_logger, "Failed to load configuration!");
        exit(-1);
    }

    char *fetcher_exec_path = Config_Get_Field_Value_String(cfg, "fetcher_exec_path");
    size_t fetcher_command_count = Config_Get_Field_Value_Integer(cfg, "fetchers_commands_count", NULL);
    pid_t fetcher_pid[fetcher_command_count * 4];
    size_t fetcher_pid_count = 0;
    pid_t elprisetjustnu_pid = -1;
    char **args = NULL;
    char date_str[16]; /* Fetcher date for checking if date has changed */

    const char *zones[] = { "SE1", "SE2", "SE3", "SE4" };
    const size_t zone_count = 4;

    for (size_t i = 0; i < fetcher_command_count; i++)
    {
        char *cmd_args_string = Config_Get_Field_Value_From_String_Array(cfg, "fetchers_commands_args", i);

        bool is_elprisetjustnu = strstr(cmd_args_string, "elprisetjustnu") != NULL;
        if (is_elprisetjustnu)
        {
            for (size_t z = 0; z < zone_count; z++)
            {
                parse_command_args(cmd_args_string, &args);

                update_fetcher_args_date(args, date_str, zones[z]);

                pid_t price_pid = ProcessManager_SpawnByExecutable(process_manager, "Fetcher", fetcher_exec_path, args, true);
                if (price_pid < 0)
                {
                    LOG_WRITE(&process_manager_logger, "Failed to spawn fetcher process");
                    return -1;
                }

                fetcher_pid[fetcher_pid_count++] = price_pid;
        
                if (args != NULL)
                    free_args(args);
            }
        }
        else
        {
            parse_command_args(cmd_args_string, &args);

            pid_t pid = ProcessManager_SpawnByExecutable(process_manager, "Fetcher", fetcher_exec_path, args, true);

            if (pid < 0)
                return -1;
            
            fetcher_pid[fetcher_pid_count++] = pid;

            free_args(args);
        }
    }

    pid_t energy_advisor_pid = ProcessManager_Spawn(process_manager, "Energy Advisor", energy_advisor_process, NULL, false);

    if (energy_advisor_pid < 0)
    {
        LOG_WRITE(&process_manager_logger, "Failed to spawn Energy Advisor process");
        return -1;
    }

    struct timespec ts;
    ts.tv_sec = 1;
    ts.tv_nsec = 0;

    // Wait for child processes to finish or termination signal
    while (process_manager_should_quit == 0)
    {
        for (size_t fetcher_index = 0; fetcher_index < fetcher_pid_count; fetcher_index++)
        {
            pid_t pid = fetcher_pid[fetcher_index];
            char buffer[256] = {0};

            if (fetcher_pid[fetcher_index] == elprisetjustnu_pid)
            {
                if (is_date_today(date_str) == false)
                {
                    // Date has changed, update fetcher args with new date
                    for (size_t i = 0; i < fetcher_command_count; i++)
                    {
                        char *cmd_args_string = Config_Get_Field_Value_From_String_Array(cfg, "fetchers_commands_args", i);
                        parse_command_args(cmd_args_string, &args);
                        update_fetcher_args_date(args, date_str, zones[zone_count]);
                        
                        fetcher_pid[fetcher_index] = ProcessManager_ResstartProcess(process_manager, elprisetjustnu_pid, "Fetcher", fetcher_exec_path, args, true);
                        elprisetjustnu_pid = fetcher_pid[fetcher_index];

                        if (args != NULL)
                            free_args(args);
                    }
                }
            }
            
            ssize_t bytes_read = ProcessManager_ReadFromChild(process_manager, pid, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0';
                //LOG_WRITE(&process_manager_logger, "Output from fetcher process: %s", buffer);
                if (strcmp(buffer, "NEW_DATA") == 0) // Example message from fetcher indicating new data is available
                {
                    //LOG_WRITE(&process_manager_logger, "Received NEW_DATA from fetcher process");
                    ProcessManager_WriteToChild(process_manager, pid, "ACK", 4);
                }
            }
        }
        nanosleep(&ts, NULL);
    }

    LOG_WRITE(&process_manager_logger, "Process Manager starting shutdown...");
    ProcessManager_TerminateAll(process_manager);

    // Wait for all child processes to terminate
    LOG_WRITE(&process_manager_logger, "Waiting for child processes to exit...");
    int status;
    while (wait(&status) > 0)
    {
        // Reap all child processes
    }
    Config_Instance_Dispose();
    ProcessManager_Destroy(process_manager);
    LOG_WRITE(&process_manager_logger, "%s", "Process Manager shutting down.");
    Logger_Dispose(&process_manager_logger);
    return 0;
}
