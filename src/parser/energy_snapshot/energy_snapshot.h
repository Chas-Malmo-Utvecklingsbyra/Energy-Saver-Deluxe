#ifndef ENERGY_SNAPSHOT_H
#define ENERGY_SNAPSHOT_H

#include <stdint.h>
#include <time.h>
#include <stdbool.h>

#define SNAPSHOTS_PER_HOUR 4

typedef struct
{
    float sun_index;
    float temp;
    float SEK_per_kWh;
} Energy_Snapshot;

typedef struct
{
    uint32_t year;
    uint8_t month;
    uint8_t day;
} Date;

typedef struct
{
    uint8_t hour;
    uint8_t minute;
} Time_Of_Day;

typedef struct
{
    Date date;
    Energy_Snapshot data[24 * SNAPSHOTS_PER_HOUR];
} Energy_Report_Day;

bool Energy_Report_Get_From_Dates(const char* weather_file_path, const char* spotprice_file_path, Date date_start, Date date_end, Energy_Report_Day* buffer, uint32_t* out_buffer_len);

#endif