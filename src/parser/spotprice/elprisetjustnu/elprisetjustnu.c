#include "elprisetjustnu.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "json/fileHelper/fileHelper.h"

Elprisetjustnu_Data Elprisetjustnu_ConvertJSONToData(const char *file_name)
{
    cJSON *root = json_read_from_file(file_name);

    if (root == NULL)
    {
        printf("[ELPRISETJUSTNU_COVNERTJSONTODATA] JSON failed to Parse\n");
    }

    int array_size = cJSON_GetArraySize(root);
    Elprisetjustnu_Data data = {0};
    data.length = array_size;

    data.quarters = (Elprisetjustnu_Quarter*)malloc(sizeof(Elprisetjustnu_Quarter) * array_size);

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


        Elprisetjustnu_Quarter quarter = {0};
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

void Elprisetjustnu_Destroy(Elprisetjustnu_Data *data)
{
    if (data == NULL || data->quarters == NULL)
        return;
    
    free(data->quarters);
    data->quarters = NULL;
}