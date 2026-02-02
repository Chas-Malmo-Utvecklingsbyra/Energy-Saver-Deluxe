#ifndef OPENMETEO_H
#define OPENMETEO_H

typedef struct 
{
    char time[17];
    float direct_radiation;
    float diffuse_radiation;
    float direct_normal_irradiance;
    float temperature_2m;
    unsigned char weather_code;
} OpenMeteo_Quarter;

typedef struct
{
    OpenMeteo_Quarter *quarters;
} OpenMeteo_Data;

// OpenMeteo_Data needs to be destroyed with OpenMeteo_Destroy
OpenMeteo_Data OpenMeteo_ConvertJSONToData(const char *file_name);

void OpenMeteo_Destroy(OpenMeteo_Data *data);

void OpenMeteo_Print_Item(OpenMeteo_Data *data, int index);

void OpenMeteo_Print_Quarter(OpenMeteo_Quarter* quarter);

#endif