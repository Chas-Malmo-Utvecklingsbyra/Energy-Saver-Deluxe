#ifndef ENERGY_TYPES_H
#define ENERGY_TYPES_H

#include <time.h>

#define ZONE_COUNT 4
#define QUARTERS_PER_HOUR   4
#define MAX_HOURS           24
#define SNAPSHOTS           (MAX_HOURS * QUARTERS_PER_HOUR)

typedef enum
{
    ENERGY_STATUS_OK,
    ENERGY_STATUS_ERROR,
    ENERGY_STATUS_DATA_MISSING,
} Energy_Status;

typedef struct
{
    const char *zone;
    const char *weather_file;
    const char *price_file;
} Energy_Zone;

typedef struct
{
    int start;
    int end;
    float average_score;
} Best_Time_Window;

typedef struct
{
    float charge_from_grid;
    float charge_from_source;

    float consume_from_grid;
    float consume_from_source;
    float consume_from_battery;

    float sell_from_battery;
    float sell_from_source;
} Energy_Flow_Advice;

typedef struct
{
    struct tm time;
    float price;
    float price_norm;
    float sun;
    Energy_Flow_Advice advice;
} Quarter_Score;

typedef struct
{
    float soc;
} Battery_State;

static const Energy_Zone zones[ZONE_COUNT] = 
{
    {"SE1", "data/weather/weather_SE1.json", "data/price/price_SE1.json"},
    {"SE2", "data/weather/weather_SE2.json", "data/price/price_SE2.json"},
    {"SE3", "data/weather/weather_SE3.json", "data/price/price_SE3.json"},
    {"SE4", "data/weather/weather_SE4.json", "data/price/price_SE4.json"}
};

#endif