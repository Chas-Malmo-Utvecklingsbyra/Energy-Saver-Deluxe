#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "energy_advisor.h"
#include "logger/logger.h"

#define ENERGY_ADVISOR_WEATHER_FILE     "weather.json"
#define ENERGY_ADVISOR_SPOTPRICE_FILE   "spotprice.json"

static int compare_price(const void *a, const void *b)
{
    float pa = *(const float*)a;
    float pb = *(const float*)b;
    return (pa > pb) - (pa < pb);
}

static Energy_Price_Level price_grading(float price, float low, float high)
{
    if (price <= low)   return PRICE_LOW;
    if (price >= high)  return PRICE_HIGH;

    return PRICE_MEDIUM;
}

static Energy_Production_Level production_grading(double sun_index)
{
    if (sun_index < 0.05)   return PROD_NONE;
    if (sun_index < 0.20)   return PROD_LOW;
    if (sun_index < 0.60)   return PROD_MEDIUM;

    return PROD_HIGH;
}

static Energy_Action decide_action(Energy_Price_Level price_level, Energy_Production_Level prod_level)
{
    if (price_level == PRICE_LOW && prod_level <= PROD_LOW)
        return ENERGY_CHARGE;

    if (price_level == PRICE_HIGH && prod_level >= PROD_MEDIUM)
        return ENERGY_SELL;

    if (prod_level >= PROD_MEDIUM)
        return ENERGY_USE;

    return ENERGY_IDLE;
}

static const char *action_to_string(Energy_Action action)
{
    switch(action)
    {
        case ENERGY_CHARGE:    
            return "CHARGE";

        case ENERGY_SELL:   
            return "SELL";

        case ENERGY_USE:
            return "USE";

        default:            
            return "IDLE";
    }
}

Energy_Status Energy_Advisor_Advice()
{
    OpenMeteo_Data weather = OpenMeteo_ConvertJSONToData(ENERGY_ADVISOR_WEATHER_FILE);
    Spotprice_Data prices = Spotprice_ConvertJSONToData(ENERGY_ADVISOR_SPOTPRICE_FILE);

    time_t current_time = time(NULL);
    struct tm *tm_info = localtime(&current_time);
    char log_filename[64];
    snprintf(log_filename, sizeof(log_filename), "Energy_Advice_%04d-%02d-%02d.txt", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday + 1);

    Logger energy_advisor_log = {0};
    Logger_Init(&energy_advisor_log, "ENERGY ADVISOR", "logfolder", log_filename, LOGGER_OUTPUT_TYPE_FILE_TEXT);

    if(weather.length == 0 || prices.length == 0)
    {
        Logger_Write(&energy_advisor_log, "%s" ,"Failed to load input data");
        return ENERGY_STATUS_DATA_MISSING;
    }

    int count = weather.length < prices.length ? weather.length : prices.length;

    float *price_buffer = malloc(sizeof(float) * count);
    int i;
    for(i = 0; i < count; i++)
    {
        price_buffer[i] = prices.quarters[i].SEK_per_kWh;
    }

    qsort(price_buffer, count, sizeof(float), compare_price);

    float low_price = price_buffer[(int)(count * 0.25f)];
    float high_price = price_buffer[(int)(count * 0.75f)];

    free(price_buffer);

    Logger_Write(&energy_advisor_log, "=========== ENERGY ADVICE FOR %04d-%02d-%02d ===========", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday + 1);
    Logger_Write(&energy_advisor_log, "----------------------------------------------------\n");

    Logger_Write(&energy_advisor_log, "Lowest price: %.3f SEK/kWh\n", low_price);
    Logger_Write(&energy_advisor_log, "Highest price: %.3f SEK/kWh\n\n", high_price);    

    Logger_Write(&energy_advisor_log, "Time             | Sun  | Price |  Temp(C) | Action\n");
    Logger_Write(&energy_advisor_log, "-----------------+------+-------+----------+-------\n");

    for(i = 0; i < count; i++)
    {
        OpenMeteo_Quarter *weather_quarter = &weather.quarters[i];
        Spotprice_Quarter *price_quarter = &prices.quarters[i];

        char timebuf[256];
        snprintf(
            timebuf, sizeof(timebuf), 
            "%04d-%02d-%02d %02d:%02d", 
            weather_quarter->time.tm_year + 1900,
            weather_quarter->time.tm_mon + 1,
            weather_quarter->time.tm_mday,
            weather_quarter->time.tm_hour,
            weather_quarter->time.tm_min
        );

        double sun_index = (weather_quarter->direct_radiation + weather_quarter->diffuse_radiation) / 300.0;
        float temp = weather_quarter->temperature_2m;

        if(sun_index > 1.0)
        {
            sun_index = 1.0;
        }

        Energy_Price_Level price_level = price_grading(price_quarter->SEK_per_kWh, low_price, high_price);
        Energy_Production_Level prod_level = production_grading(sun_index);
        Energy_Action action = decide_action(price_level, prod_level);
        
        
        Logger_Write(&energy_advisor_log, "%s | %.2f | %.3f |   %.1f   | %s\n", timebuf, sun_index, price_quarter->SEK_per_kWh, temp, action_to_string(action));
    }

    Logger_Dispose(&energy_advisor_log);
    OpenMeteo_Destroy(&weather);
    Spotprice_Destroy(&prices);

    return ENERGY_STATUS_OK;
}