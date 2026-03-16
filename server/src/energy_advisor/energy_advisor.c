#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "energy_advisor.h"
#include "logger/logger.h"
#include "file_helper/file_helper.h"
#include "benchmark/benchmark.h"


Energy_Status Energy_Advisor_Advice()
{
    Benchmark benchmark = {0};
    Benchmark_Start(&benchmark);

    OpenMeteo_Data weather[ZONE_COUNT];
    Spotprice_Data prices[ZONE_COUNT];

    int z;
    for (z = 0; z < ZONE_COUNT; z++)
    {
        weather[z] = OpenMeteo_ConvertJSONToData(zones[z].weather_file);
        prices[z] = Spotprice_ConvertJSONToData(zones[z].price_file);
    }
    
    Logger energy_advisor_log = {0};
    const char log_filename[] = "Energy_Advice_Log.txt";
    Logger_Init(&energy_advisor_log, "ENERGY ADVISOR", "logfolder", log_filename, LOGGER_OUTPUT_TYPE_FILE_TEXT);

    const char *advice_dir = "Energy_Advice_Reports";
    File_Helper_Create_Dir(advice_dir);

    for (z = 0; z < ZONE_COUNT; z++)
    {
        if (weather[z].length == 0 || prices[z].length == 0)
        {
            Logger_Write(&energy_advisor_log, "Failed to load input data (zone: %s)", zones[z].zone);
            continue;
        }

        struct tm report_date = prices[z].quarters[0].time_start;

        time_t current_time = time(NULL);
        struct tm tomorrow = *localtime(&current_time);
        tomorrow.tm_mday += 1;
        mktime(&tomorrow);

        int weather_count;
        int weather_start = Energy_Find_Weather_Start(&weather[z], &report_date, &weather_count);    
        if (weather_start == -1)
        {
            Logger_Write(&energy_advisor_log, "Failed to get weather data for requested date (zone %s)", zones[z].zone);
            continue;
        }

        int count;
        float low_price, high_price;

        Quarter_Score *analysis = Energy_Run_Analysis(&weather[z], &prices[z], &count, &low_price, &high_price, weather_start);
        if (!analysis)
        {
            Logger_Write(&energy_advisor_log, "Out of memory for analysis (zone: %s)", zones[z].zone);
            continue;
        }

        Energy_Summary summary = calculate_summary(analysis, count);
        
        char filename[64];
        snprintf(filename, sizeof(filename), "Energy_Advice_%s_%04d-%02d-%02d.txt", zones[z].zone, tomorrow.tm_year + 1900, tomorrow.tm_mon + 1, tomorrow.tm_mday);
        File_Helper_Create(advice_dir, filename);  
        
        write_advice_report(advice_dir, filename, "\n============================================= DATA IS VALID FOR ENERGY ZONE %s ============================================\n", zones[z].zone);
        write_advice_report_header(advice_dir, filename, &tomorrow, low_price, high_price);
        write_advice_report_summary(advice_dir, filename, analysis, &summary, count);

        char json_filename[64];
        snprintf(json_filename, sizeof(json_filename), "Energy_Advice_%s_%04d-%02d-%02d.json", zones[z].zone, tomorrow.tm_year + 1900, tomorrow.tm_mon + 1, tomorrow.tm_mday);
        Energy_Write_JSON_Report(advice_dir, json_filename, analysis, &tomorrow, &summary, count);

        free(analysis);
    }   

    for (z = 0; z < ZONE_COUNT; z++)
    {
        OpenMeteo_Destroy(&weather[z]);
        Spotprice_Destroy(&prices[z]);
    }

    Logger_Dispose(&energy_advisor_log);

    Benchmark_Stop(&benchmark);
    Benchmark_Print(&benchmark);

    return ENERGY_STATUS_OK;
}