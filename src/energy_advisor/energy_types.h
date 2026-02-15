#ifndef ENERGY_TYPES_H
#define ENERGY_TYPES_H

#include <time.h>

#define QUARTERS        4
#define MAX_HOURS       24
#define SNAPSHOTS       (MAX_HOURS * QUARTERS)

typedef enum
{
    // Maybe we can add more later, if necessary
    ENERGY_IDLE = 0,
    ENERGY_BUY,
    ENERGY_SELL  
} Energy_Action;

typedef enum
{
    ENERGY_STATUS_OK,
    ENERGY_STATUS_DATA_MISSING
} Energy_Status;

typedef struct
{
    Energy_Action action[SNAPSHOTS];
    Energy_Status status;
    time_t timestamps[SNAPSHOTS];

    // Not yet implemented
    double simulated_storage[SNAPSHOTS];
} Energy_Plan;


#endif