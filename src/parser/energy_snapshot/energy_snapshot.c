#include "energy_snapshot.h"

#include <stdio.h>


#include "../weather_new/weather.h"
#include "../spotprice/spotprice.h"

typedef struct
{
    Weather_Quarter weather_output;
    Spotprice_Quarter spotprice_output;
} Quarter;

static bool Energy_Snapshot_Parse(const char* weather_file_path, const char* spotprice_file_path, Date date_start, uint32_t requested_quarter_count, Quarter* quarter, uint32_t* out_count)
{
    (void)weather_file_path;
    (void)spotprice_file_path;
    (void)date_start;
    (void)requested_quarter_count;
    (void)quarter;
    (void)out_count;
    /* Weather */
    
    /* Spotprice */
    
    /* Build Quarter array */

    return false;
}

bool Energy_Report_Get_From_Dates(const char* weather_file_path, const char* spotprice_file_path, Date date_start, Date date_end, Energy_Report_Day* buffer, uint32_t* out_buffer_len)
{
    /* Call parse using the supplied filepaths and return snapshots from between start and end dates as a list of Energy_Report_Day */
    (void)date_end;
    (void)buffer;
    (void)out_buffer_len;

    OpenMeteo_Quarter *quarter = NULL;
    Quarter temp;
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

