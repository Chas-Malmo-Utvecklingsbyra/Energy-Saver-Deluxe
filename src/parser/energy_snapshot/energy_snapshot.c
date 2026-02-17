#include "energy_snapshot.h"

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>

#include "../weather_new/weather.h"
#include "../spotprice/spotprice.h"

typedef struct
{
    Weather_Quarter weather_output;
    Spotprice_Quarter spotprice_output;
} Quarter;

static bool Energy_IsSameDate(struct tm* date, Date* other_date)
{
    bool is_correct_day = date->tm_mday == other_date->day;
    bool is_correct_month = date->tm_mon + 1 == other_date->month;
    bool is_correct_year = (uint32_t)(date->tm_year + 1900) == other_date->year;

    return is_correct_day && is_correct_month && is_correct_year;
}

static bool Energy_Snapshot_Parse(const char *weather_file_path, const char *spotprice_file_path, Date *date_start, uint32_t requested_quarter_count, Quarter *out_quarter[])
{
    // TODO: Change OpenMeteo_Data to Weather_Data when it is implemented
    // TODO: Clean this up

    OpenMeteo_Data openmeteo_data = OpenMeteo_ConvertJSONToData(weather_file_path);
    Spotprice_Data spotprice_data = Spotprice_ConvertJSONToData(spotprice_file_path);

    Quarter *buffer = (Quarter*)malloc(sizeof(Quarter) * requested_quarter_count);

    size_t openmeteo_start = 0;
    
    for (size_t i = 0; i < (size_t)openmeteo_data.length; i++)
    {
        if (Energy_IsSameDate(&openmeteo_data.quarters[i].time, date_start))
        {
            openmeteo_start = i;
            break;
        }
    }

    size_t spotprice_length = 0;

    for (size_t i = 0; i < (size_t)spotprice_data.length; i++)
    {
        if (Energy_IsSameDate(&spotprice_data.quarters[i].time_start, date_start))
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

    *out_quarter = buffer;

    return true;
}

bool Energy_Report_Get_From_Date(const char *weather_file_path, const char *spotprice_file_path, Date *date_start, uint32_t quarters_to_request, Energy_Report_Day** buffer)
{
    Quarter *quarter = NULL;
    uint32_t requested_quarters = quarters_to_request;

    if (requested_quarters <= 0)
    {
        requested_quarters = 1;
    }

    bool result = Energy_Snapshot_Parse(weather_file_path, spotprice_file_path, date_start, requested_quarters, &quarter);
    if (!result) 
    {
        free(quarter);
        quarter = NULL;
        return false;
    }

    Energy_Report_Day *new_buffer = (Energy_Report_Day*)malloc((sizeof(Energy_Report_Day)));
    if (!new_buffer) 
        return false;

    memset(new_buffer, 0, sizeof(Energy_Report_Day));

    new_buffer->data = (Energy_Snapshot*)malloc(sizeof(Energy_Snapshot) * requested_quarters);
    if (!new_buffer->data)
    {
        free(new_buffer);
        new_buffer = NULL;
        return false;
    }
    
    for (uint32_t i = 0; i < requested_quarters; i++)
    {
        new_buffer->data[i].SEK_per_kWh = quarter[i].spotprice_output.SEK_per_kWh;
        new_buffer->data[i].sun_index = quarter[i].weather_output.direct_radiation;
        new_buffer->data[i].temp = quarter[i].weather_output.temperature_2m;
    }

    free(quarter);
    quarter = NULL;
    *buffer = new_buffer;

    return true;
}

