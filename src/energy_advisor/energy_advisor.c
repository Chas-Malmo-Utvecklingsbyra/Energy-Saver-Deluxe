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

static Energy_Action decide_action(float price, float low_price, float high_price, double sun_index)
{
    (void)sun_index;

    if(price <= low_price)
    {
        return ENERGY_BUY;
    }

    // Don't know if this is too strict during the winter, because it never comes true
    /* if(price >= high_price && sun_index > 0.2)
    {
        return ENERGY_SELL;
    } */

    // Here we trigger to sell when the price is high, but don't care about sun_index
    if(price >= high_price)
    {
        return ENERGY_SELL;
    }

    return ENERGY_IDLE;
}

static const char *action_to_string(Energy_Action action)
{
    switch(action)
    {
        case ENERGY_BUY:    return "BUY";
        case ENERGY_SELL:    return "SELL";
        default:    return "IDLE";
    }
}

Energy_Status Energy_Advisor_Advice()
{
    OpenMeteo_Data weather = OpenMeteo_ConvertJSONToData(ENERGY_ADVISOR_WEATHER_FILE);
    Spotprice_Data prices = Spotprice_ConvertJSONToData(ENERGY_ADVISOR_SPOTPRICE_FILE);
    Logger energy_advisor_log = {0};
    Logger_Init(&energy_advisor_log, "ENERGY ADVISOR", "logfolder", "Energy_Advice.txt", LOGGER_OUTPUT_TYPE_FILE_TEXT);

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

    Logger_Write(&energy_advisor_log, "Low price threshold : %.3f SEK/kWh\n", low_price);
    Logger_Write(&energy_advisor_log, "High price threshold : %.3f SEK/kWh\n\n", high_price);    

    Logger_Write(&energy_advisor_log, "Time             | Sun  | Price | Action\n");
    Logger_Write(&energy_advisor_log, "-----------------+------+-------+-------\n");

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

        if(sun_index > 1.0)
        {
            sun_index = 1.0;
        }

        Energy_Action action = decide_action(price_quarter->SEK_per_kWh, low_price, high_price, sun_index);
        
        
        Logger_Write(&energy_advisor_log, "%s | %.2f | %.3f | %s\n", timebuf, sun_index, price_quarter->SEK_per_kWh, action_to_string(action));
    }

    Logger_Dispose(&energy_advisor_log);
    OpenMeteo_Destroy(&weather);
    Spotprice_Destroy(&prices);

    return ENERGY_STATUS_OK;
}