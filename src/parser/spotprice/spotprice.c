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
        strncpy(quarter.time_start, cJSON_GetStringValue(time_start), sizeof(quarter.time_start));
        strncpy(quarter.time_end, cJSON_GetStringValue(time_end), sizeof(quarter.time_end));

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

void Spotprice_Print_Item(Spotprice_Data *data, int index)
{
    if (data == NULL || data->quarters == NULL)
        return;
    
    Spotprice_Quarter *quarter = &data->quarters[index];

    printf("SEK: %f | EUR: %f | EXR: %f | START: %s | END: %s\n",
        quarter->SEK_per_kWh, quarter->EUR_per_kWh, quarter->EXR, quarter->time_start, quarter->time_end
    );
}