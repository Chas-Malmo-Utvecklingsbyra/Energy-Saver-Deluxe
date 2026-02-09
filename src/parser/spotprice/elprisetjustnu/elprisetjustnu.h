#ifndef ELPRISETJUSTNU_H
#define ELPRISETJUSTNU_H

#define _XOPEN_SOURCE
#include <time.h>

typedef struct
{
    float SEK_per_kWh;
    float EUR_per_kWh;
    float EXR;
    struct tm time_start;
    struct tm time_end;
}Elprisetjustnu_Quarter;

typedef struct
{
    Elprisetjustnu_Quarter* quarters;
    int length;
}Elprisetjustnu_Data;

Elprisetjustnu_Data Elprisetjustnu_ConvertJSONToData(const char *file_name);

void Elprisetjustnu_Destroy(Elprisetjustnu_Data *data);


#endif