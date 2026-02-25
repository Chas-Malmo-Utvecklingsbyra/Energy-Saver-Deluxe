#ifndef ENERGY_TYPES_H
#define ENERGY_TYPES_H

#include <time.h>

#define QUARTERS        4
#define MAX_HOURS       24
#define SNAPSHOTS       (MAX_HOURS * QUARTERS)

typedef enum
{
    CHARGE_FROM_GRID = 0,
    CHARGE_FROM_SOURCE,
    CONSUME_FROM_GRID,
    CONSUME_FROM_SOURCE,
    CONSUME_FROM_BATTERY,
    SELL_FROM_BATTERY,
    SELL_FROM_SOURCE
} Energy_Action;

typedef enum
{
    ENERGY_STATUS_OK,
    ENERGY_STATUS_DATA_MISSING
} Energy_Status;

typedef enum
{
    // Not using these now, but may implement them again later
    PROD_NONE,
    PROD_LOW,
    PROD_MEDIUM,
    PROD_HIGH
} Energy_Production_Level;

typedef enum
{
    // Not using these now, but may implement them again later
    PRICE_LOW,
    PRICE_MEDIUM,
    PRICE_HIGH
} Energy_Price_Level;

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
    float soc;
} Battery_State;

typedef struct
{
    Energy_Production_Level prod_level;
    Energy_Price_Level price_level;
    Energy_Flow_Advice advice;
    Energy_Action action[SNAPSHOTS];
    Energy_Status status;
    time_t timestamps[SNAPSHOTS];
    FILE *fp;
} Energy_Plan;


#endif