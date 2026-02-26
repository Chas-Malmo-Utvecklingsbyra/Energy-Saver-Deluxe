#ifndef SPOTPRICE_H
#define SPOTPRICE_H

#define _XOPEN_SOURCE
#include <time.h>

typedef struct
{
    float SEK_per_kWh;
    float EUR_per_kWh;
    float EXR;
    struct tm time_start;
    struct tm time_end;
}Spotprice_Quarter;

typedef struct
{
    Spotprice_Quarter* quarters;
    int length;
}Spotprice_Data;

Spotprice_Data Spotprice_ConvertJSONToData(const char *file_name);

void Spotprice_Destroy(Spotprice_Data *data);

void Spotprice_Print_Item(Spotprice_Data *data, int index);


#endif