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
    //Energy_Snapshot data[24 * SNAPSHOTS_PER_HOUR];
    Energy_Snapshot* data;
} Energy_Report_Day;


// weather_file_path: Path to the weather file
// spotprice_file_path: Path to spotprice file
// Energy_Report_Bufer** buffer: NEEDS TGO BE FREED(Destroy via Destroy function)
// uint32_t quarters_to_request: AMOUNT OF QUARTERS TO GET (24 * 4 quarters is a day)
bool Energy_Report_Get_From_Date(const char *weather_file_path, const char *spotprice_file_path, Date *date_start, uint32_t quarters_to_request, Energy_Report_Day **buffer);


void Energy_Report_Destroy(Energy_Report_Day **report);

#endif