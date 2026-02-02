#ifndef SPOTPRICE_H
#define SPOTPRICE_H

typedef struct
{
    float SEK_per_kWh;
    float EUR_per_kWh;
    float EXR;
    char time_start[26];
    char time_end[26];
}Spotprice_Quarter;

typedef struct
{
    Spotprice_Quarter* quarters;
}Spotprice_Data;

Spotprice_Data Spotprice_ConvertJSONToData(const char *file_name);

void Spotprice_Destroy(Spotprice_Data *data);

void Spotprice_Print_Item(Spotprice_Data *data, int index);


#endif