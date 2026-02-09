#include "energy_snapshot.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "../weather_new/weather.h"
#include "../spotprice/spotprice.h"

typedef struct
{
    Weather_Quarter weather_output;
    Spotprice_Quarter spotprice_output;
} Quarter;

static bool Energy_Snapshot_Parse(const char* weather_file_path, const char* spotprice_file_path, Date date_start, uint32_t requested_quarter_count, Quarter* quarter[], uint32_t* out_count)
{
    // TODO: Change OpenMeteo_Data to Weather_Data when it is implemented
    // TODO: Clean this up

    OpenMeteo_Data openmeteo_data = OpenMeteo_ConvertJSONToData(weather_file_path);
    Spotprice_Data spotprice_data = Spotprice_ConvertJSONToData(spotprice_file_path);

    Quarter* buffer = (Quarter*)malloc(sizeof(Quarter) * requested_quarter_count);

    size_t openmeteo_start = 0;
    
    for (size_t i = 0; i < (size_t)openmeteo_data.length; i++)
    {
        bool is_correct_day = openmeteo_data.quarters[i].time.tm_mday == date_start.day;
        bool is_correct_month = openmeteo_data.quarters[i].time.tm_mon + 1 == date_start.month;
        bool is_correct_year = (uint32_t)(openmeteo_data.quarters[i].time.tm_year + 1900) == date_start.year;

        if (is_correct_day && is_correct_month && is_correct_year)
        {
            openmeteo_start = i;
            break;
        }
    }

    size_t spotprice_length = 0;

    for (size_t i = 0; i < (size_t)spotprice_data.length; i++)
    {
        bool is_correct_day = spotprice_data.quarters[i].time_start.tm_mday == date_start.day;
        bool is_correct_month = spotprice_data.quarters[i].time_start.tm_mon + 1 == date_start.month;
        bool is_correct_year = (uint32_t)(spotprice_data.quarters[i].time_start.tm_year + 1900) == date_start.year;

        if (is_correct_day && is_correct_month && is_correct_year)
        {
            spotprice_length = i;
            break;
        }
    }


    for (size_t i = 0; i < requested_quarter_count; i++)
    {
        size_t offset = i + openmeteo_start;

        buffer[offset].weather_output = openmeteo_data.quarters[i];
    }

    for (size_t i = 0; i < requested_quarter_count; i++)
    {
        size_t offset = i + spotprice_length;

        buffer[offset].spotprice_output = spotprice_data.quarters[i];
    }

    *quarter = buffer;
    *out_count = requested_quarter_count;

    return false;
}

bool Energy_Report_Get_From_Dates(const char* weather_file_path, const char* spotprice_file_path, Date date_start, Date date_end, Energy_Report_Day* buffer, uint32_t* out_buffer_len)
{
    /* Call parse using the supplied filepaths and return snapshots from between start and end dates as a list of Energy_Report_Day */
    (void)date_end;
    (void)buffer;
    (void)out_buffer_len;

    OpenMeteo_Quarter* quarter = NULL;
    Quarter* temp;
    uint32_t out_count = 0;
    uint32_t quarters_to_request = 24 * 4;
    bool result = Energy_Snapshot_Parse(weather_file_path, spotprice_file_path, date_start, quarters_to_request, &temp, &out_count);
    
    if (result == true)
    {
        for (uint32_t i = 0; i < out_count; i++)
        {
            OpenMeteo_Quarter* q = &quarter[i];
            OpenMeteo_Print_Quarter(q);
        }

        return true;
    }

    return false;
}

