#ifndef ENERGY_TYPES_H
#define ENERGY_TYPES_H

#include <time.h>
#include <stdbool.h>

#define ZONE_COUNT 4
#define QUARTERS_PER_HOUR   4
#define MAX_HOURS           24
#define SNAPSHOTS           (MAX_HOURS * QUARTERS_PER_HOUR)

/**
 * @brief Core data structures used by the energy advisor.
 * 
 * This header defines the primary data structures used throughout
 * the energy analysis system.
 * 
 * Key structures include:
 * 
 * - Quarter_Score      : Analysis results for a single 15-minute period
 * - Energy_Flow_Advice : Recommendation scores for energy flows
 * - Best_Time_Window   : Optimal time window for a specific action
 * - Energy_Summary     : Daily summary of optimal actions
 * 
 * These structures form the data model used across the entire system.
 */

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
    bool found;
} Best_Time_Window;

typedef struct
{
    Best_Time_Window charge_from_grid;
    Best_Time_Window charge_from_source;
    Best_Time_Window consume;
    Best_Time_Window sell;
} Energy_Summary;

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
