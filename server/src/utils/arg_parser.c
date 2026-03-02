#define _POSIX_C_SOURCE 200809L

#include "arg_parser.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

int parse_command_args(const char *cmd_string, char ***args_out)
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

void free_args(char **args)
{
    if (!args)
        return;
    for (int i = 0; args[i]; i++)
    {
        free(args[i]);
    }
    free(args);
}

int update_fetcher_args_date(char **args)
{
    for (int j = 0; args != NULL && args[j] != NULL; j++)
    {
        if (strcmp(args[j], "-u") == 0 && args[j + 1])
        {
            if (strcmp(args[j + 1], "https://www.elprisetjustnu.se") != 0)
            {
                return 0;
            }
        }
        if (strcmp(args[j], "-r") == 0 && args[j + 1])
        {
            time_t t = time(NULL);
            struct tm tm = *localtime(&t);
            char date_buffer[64];
            snprintf(date_buffer, sizeof(date_buffer), "/api/v1/prices/%04d/%02d-%02d_SE4.json", 
                     tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
            free(args[j + 1]);
            args[j + 1] = strdup(date_buffer);
        }
    }

    return 0;
}
