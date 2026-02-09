#include "spotprice.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json/fileHelper/fileHelper.h"



Spotprice_Data Spotprice_ConvertJSONToData(const char *file_name)
{
    cJSON *root = json_read_from_file(file_name);

    if (root == NULL)
    {
        printf("[SPOTPRICE_COVNERTJSONTODATA] JSON failed to Parse\n");
    }

    int array_size = cJSON_GetArraySize(root);
    Spotprice_Data data = {0};
    data.length = array_size;

    data.quarters = (Spotprice_Quarter*)malloc(sizeof(Spotprice_Quarter) * array_size);

    for (int i = 0; i < array_size; i++)
    {
        cJSON *object = cJSON_GetArrayItem(root, i);
        if (object == NULL)
            break;

        cJSON *SEK_per_kWh = cJSON_GetObjectItem(object, "SEK_per_kWh");
        if (SEK_per_kWh == NULL)
            break;
        
        cJSON *EUR_per_kWh = cJSON_GetObjectItem(object, "EUR_per_kWh");
        if (EUR_per_kWh == NULL)
            break;

        cJSON *EXR = cJSON_GetObjectItem(object, "EXR");
        if (EXR == NULL)
            break;

        cJSON *time_start = cJSON_GetObjectItem(object, "time_start");
        if (time_start == NULL)
            break;

        cJSON *time_end = cJSON_GetObjectItem(object, "time_end");
        if (time_end == NULL)
            break;


        Spotprice_Quarter quarter = {0};
        quarter.SEK_per_kWh = (float)cJSON_GetNumberValue(SEK_per_kWh);
        quarter.EUR_per_kWh = (float)cJSON_GetNumberValue(EUR_per_kWh);
        quarter.EXR = (float)cJSON_GetNumberValue(EXR);


        memset(&quarter.time_start, 0, sizeof(struct tm));
        memset(&quarter.time_end, 0, sizeof(struct tm));

        //"time_start": "2026-02-01T00:30:00+01:00",
        // 2026-02-06T00:00

        // Ignoring Time Zones!
        if (strptime(cJSON_GetStringValue(time_start), "%Y-%m-%dT%H:%M", &quarter.time_start) == NULL)
        {
            printf("Failed to parse time_start in SpotPrice\n");
        }

        if (strptime(cJSON_GetStringValue(time_end), "%Y-%m-%dT%H:%M", &quarter.time_end) == NULL)
        {
            printf("Failed to parse time_end in SpotPrice\n");
        }

        data.quarters[i] = quarter;
    }

    cJSON_Delete(root);

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