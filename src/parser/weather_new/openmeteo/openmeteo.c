#include "openmeteo.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "json/fileHelper/fileHelper.h"


OpenMeteo_Data OpenMeteo_ConvertJSONToData(const char *file_name)
{
    cJSON *root = json_read_from_file(file_name);

    if (root == NULL)
    {
        printf("[OPENMETEO_CONVERTJSONTODATA] JSON failed to Parse\n");
    }

    cJSON *minutely_15 = cJSON_GetObjectItem(root, "minutely_15");
    cJSON *time_array = cJSON_GetObjectItem(minutely_15, "time");
    cJSON *direct_radiation_array = cJSON_GetObjectItem(minutely_15, "direct_radiation");
    cJSON *diffuse_radiation_array = cJSON_GetObjectItem(minutely_15, "diffuse_radiation");
    cJSON *direct_normal_irradiance_array = cJSON_GetObjectItem(minutely_15, "direct_normal_irradiance");
    cJSON *temperature_2m_array = cJSON_GetObjectItem(minutely_15, "temperature_2m");
    cJSON *weather_code_array = cJSON_GetObjectItem(minutely_15, "weather_code");

    int array_size = cJSON_GetArraySize(time_array);

    OpenMeteo_Data data = {0};
    data.length = array_size;

    data.quarters = (OpenMeteo_Quarter*)malloc(sizeof(OpenMeteo_Quarter) * array_size);

    for (int i = 0; i < array_size; i++)
    {
        cJSON *time_item = cJSON_GetArrayItem(time_array, i);
        if (time_item == NULL)
            break;

        cJSON *direct_radiation_item = cJSON_GetArrayItem(direct_radiation_array, i);
        if (direct_radiation_item == NULL)
            break;

        cJSON *diffuse_radiation_item = cJSON_GetArrayItem(diffuse_radiation_array, i);
        if (diffuse_radiation_item == NULL)
            break;
    
        cJSON *direct_normal_irradiance_item = cJSON_GetArrayItem(direct_normal_irradiance_array, i);
        if (direct_normal_irradiance_item == NULL)
            break;

        cJSON *temperature_2m_item = cJSON_GetArrayItem(temperature_2m_array, i);
        if (temperature_2m_item == NULL)
            break;

        cJSON *weather_code_item = cJSON_GetArrayItem(weather_code_array, i);
        if (weather_code_item == NULL)
            break;

        OpenMeteo_Quarter quarter = {0};

        memset(&quarter.time, 0, sizeof(struct tm));

        // 2026-02-06T00:00
        if (strptime(cJSON_GetStringValue(time_item), "%Y-%m-%dT%H:%M", &quarter.time) == NULL)
        {
            printf("Failed to parse Time in OpenMeteo\n");
        }
        //strncpy(quarter.time., cJSON_GetStringValue(time_item), sizeof(quarter.time));

        quarter.direct_radiation = (float)cJSON_GetNumberValue(direct_radiation_item);
        quarter.diffuse_radiation = (float)cJSON_GetNumberValue(diffuse_radiation_item);
        quarter.direct_normal_irradiance = (float)cJSON_GetNumberValue(direct_normal_irradiance_item);
        quarter.temperature_2m = (float)cJSON_GetNumberValue(temperature_2m_item);
        quarter.weather_code = (unsigned char)cJSON_GetNumberValue(weather_code_item);

        data.quarters[i] = quarter;
    }

    cJSON_Delete(root);

    return data;
}

void OpenMeteo_Destroy(OpenMeteo_Data *data)
{
    if (data == NULL || data->quarters == NULL)
        return;
    
    free(data->quarters);
    data->quarters = NULL;
}

void OpenMeteo_Print_Item(OpenMeteo_Data *data, int index)
{
    if (data == NULL || data->quarters == NULL)
        return;

    OpenMeteo_Quarter* quarter = &data->quarters[index];

    OpenMeteo_Print_Quarter(quarter);
}

void OpenMeteo_Get_Quarter_Time_String(OpenMeteo_Quarter* quarter, char* buffer, size_t length)
{
    // "2026-02-06T00:00"
    snprintf(buffer, length, "%d-%02d-%02d %02d:%02d", quarter->time.tm_year + 1900, quarter->time.tm_mon, quarter->time.tm_mday, quarter->time.tm_hour, quarter->time.tm_min);
}   

void OpenMeteo_Print_Quarter(OpenMeteo_Quarter* quarter)
{
    size_t length = 17;
    char time_buffer[length];
    memset(time_buffer, 0, length);

    OpenMeteo_Get_Quarter_Time_String(quarter, time_buffer, length);

    printf("Time: %s | Direct_Rad: %f | Diffuse: %f | Direct_Norm: %f | Temp_2m: %f | Weather_Code: %d\n", 
        time_buffer, quarter->direct_radiation, quarter->diffuse_radiation, quarter->direct_normal_irradiance, quarter->temperature_2m, quarter->weather_code
    );
}