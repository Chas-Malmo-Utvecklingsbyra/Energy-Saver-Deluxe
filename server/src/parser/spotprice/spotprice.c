#include "spotprice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json/fileHelper/fileHelper.h"
#include "elprisetjustnu/elprisetjustnu.h"


Spotprice_Data Spotprice_ConvertJSONToData(const char *file_name)
{
    Elprisetjustnu_Data elprisetjustnu_data = Elprisetjustnu_ConvertJSONToData(file_name);
    
    Spotprice_Data data = {0};

    data.quarters = (Spotprice_Quarter*)malloc(sizeof(Spotprice_Quarter) * elprisetjustnu_data.length);

    data.length = elprisetjustnu_data.length;

    for (int i = 0; i < data.length; i++)
    {
        data.quarters[i].EUR_per_kWh = elprisetjustnu_data.quarters[i].EUR_per_kWh;
        data.quarters[i].EXR = elprisetjustnu_data.quarters[i].EXR;
        data.quarters[i].SEK_per_kWh = elprisetjustnu_data.quarters[i].SEK_per_kWh;
        data.quarters[i].time_end = elprisetjustnu_data.quarters[i].time_end;
        data.quarters[i].time_start = elprisetjustnu_data.quarters[i].time_start;
    }
    
    Elprisetjustnu_Destroy(&elprisetjustnu_data);

    return data;
}

void Spotprice_Destroy(Spotprice_Data *data)
{
    if (data == NULL || data->quarters == NULL)
        return;
    
    free(data->quarters);
    data->quarters = NULL;
}

void Spotprice_Get_Quarter_Time_String(Spotprice_Quarter* quarter, char* buffer, size_t length)
{
    //"time_start": "2026-02-01T00:30:00+01:00",
    snprintf(buffer, length, "%d-%02d-%02d %02d:%02d", quarter->time_start.tm_year + 1900, quarter->time_start.tm_mon, quarter->time_start.tm_mday, quarter->time_start.tm_hour, quarter->time_start.tm_min);
}   


void Spotprice_Print_Item(Spotprice_Data *data, int index)
{
    if (data == NULL || data->quarters == NULL)
        return;
    
    Spotprice_Quarter *quarter = &data->quarters[index];

    size_t length = 17;
    char time_buffer[length];
    memset(time_buffer, 0, length);

    Spotprice_Get_Quarter_Time_String(quarter, time_buffer, length);

    printf("SEK: %f | EUR: %f | EXR: %f | TIME_START: %s \n",
        quarter->SEK_per_kWh, quarter->EUR_per_kWh, quarter->EXR, time_buffer
    );
}