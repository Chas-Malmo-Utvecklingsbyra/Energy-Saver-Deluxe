#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "energy_advisor.h"
#include "logger/logger.h"
#include "file_helper/file_helper.h"

#define ENERGY_ADVISOR_WEATHER_FILE     "data/weather/weather.json"
#define ENERGY_ADVISOR_SPOTPRICE_FILE   "data/price/price.json"

// Testing an implementation of colors to the textfile for easier readability for the user.
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define RED     "\033[31m"
#define RESET   "\033[0m"

static int compare_price(const void *a, const void *b)
{
    float pa = *(const float*)a;
    float pb = *(const float*)b;
    return (pa > pb) - (pa < pb);
}

static float normalize_price(float price, float low, float high)
{
    if (price <= low)   return 0.0f;
    if (price >= high)  return 1.0f;

    return (price - low) / (high - low);
}

static Energy_Flow_Advice compute_advice(float price_norm, float production, float battery_soc)
{
    Energy_Flow_Advice advice = {0};

    advice.charge_from_grid = (1.0f - price_norm) * (1.0f - production) * (1.0f - battery_soc);
    if (advice.charge_from_grid < 0.0)
        advice.charge_from_grid = 0.0;
    else if (advice.charge_from_grid > 1.0)
        advice.charge_from_grid = 1.0;

    advice.charge_from_source = production * (1.0f - battery_soc);
    if (advice.charge_from_source < 0.0)
        advice.charge_from_source = 0.0;
    else if (advice.charge_from_source > 1.0)
        advice.charge_from_source = 1.0;

    advice.consume_from_grid = (1.0f - production) * (1.0f - battery_soc) * (1.0f - price_norm);
    if (advice.consume_from_grid < 0.0)
        advice.consume_from_grid = 0.0;
    else if (advice.consume_from_grid > 1.0)
        advice.consume_from_grid = 1.0;    

    advice.consume_from_source = production;
    if (advice.consume_from_source < 0.0)
        advice.consume_from_source = 0.0;
    else if (advice.consume_from_source > 1.0)
        advice.consume_from_source = 1.0;

    advice.consume_from_battery = battery_soc * price_norm;
    if (advice.consume_from_battery < 0.0)
        advice.consume_from_battery = 0.0;
    else if (advice.consume_from_battery > 1.0)
        advice.consume_from_battery = 1.0;
    
    advice.sell_from_battery = battery_soc * price_norm;
    if (advice.sell_from_battery < 0.0)
        advice.sell_from_battery = 0.0;
    else if (advice.sell_from_battery > 1.0)
        advice.sell_from_battery = 1.0;
    
    advice.sell_from_source = production * price_norm * battery_soc;
    if (advice.sell_from_source < 0.0)
        advice.sell_from_source = 0.0;
    else if (advice.sell_from_source > 1.0)
        advice.sell_from_source = 1.0;

    return advice;
}

static void write_advice_report(const char *path, const char *filename, const char *fmt, ...)
{
    char buffer[4096];

    va_list args;
    va_start(args, fmt);
    int len = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (len <= 0)
        return;

    File_Helper_Write(path, filename, buffer, (size_t)len, FILE_HELPER_MODE_APPEND, false);
}

Energy_Status Energy_Advisor_Advice()
{
    OpenMeteo_Data weather = OpenMeteo_ConvertJSONToData(ENERGY_ADVISOR_WEATHER_FILE);
    Spotprice_Data prices = Spotprice_ConvertJSONToData(ENERGY_ADVISOR_SPOTPRICE_FILE);
    
    time_t current_time = time(NULL);
    struct tm *tm_info = localtime(&current_time);

    char filename[64];
    snprintf(filename, sizeof(filename), "Energy_Advice_%04d-%02d-%02d.txt", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday + 1);
    
    const char *advice_dir = "Energy_Advice_Reports";
    File_Helper_Create_Dir(advice_dir);
    File_Helper_Create(advice_dir, filename);
    
    Logger energy_advisor_log = {0};
    char log_filename[64];
    snprintf(log_filename, sizeof(log_filename), "Energy_Advice_Log.txt");
    
    Logger_Init(&energy_advisor_log, "ENERGY ADVISOR", "logfolder", log_filename, LOGGER_OUTPUT_TYPE_FILE_TEXT);
    if (weather.length == 0 || prices.length == 0)
    {
        Logger_Write(&energy_advisor_log, "%s" ,"Failed to load input data");
        return ENERGY_STATUS_DATA_MISSING;
    }

    int count = weather.length < prices.length ? weather.length : prices.length;

    float *price_buffer = malloc(sizeof(float) * count);
    int i;
    for (i = 0; i < count; i++)
    {
        price_buffer[i] = prices.quarters[i].SEK_per_kWh;
    }

    qsort(price_buffer, count, sizeof(float), compare_price);

    float low_price = price_buffer[(int)(count * 0.25f)];
    float high_price = price_buffer[(int)(count * 0.75f)];

    free(price_buffer);

    struct tm report_date = prices.quarters[0].time_start;

    int weather_start = -1;
    int weather_count = 0;

    for (i = 0; i < weather.length; i++)
    {
        struct tm *weather_time = &weather.quarters[i].time;

        if (weather_time->tm_year == report_date.tm_year && weather_time->tm_mon == report_date.tm_mon && weather_time->tm_mday == report_date.tm_mday)
        {
            if (weather_start == -1)
            {
                weather_start = i;
            }
            weather_count++;
        }
    }

    if (weather_count < 0)
    {
        Logger_Write(&energy_advisor_log, "%s", "Failed to get weather data for requested date");
        return ENERGY_STATUS_DATA_MISSING;
    }
    if (weather_count < prices.length)
    {
        Logger_Write(&energy_advisor_log, "%s", "Insufficient weather data (%d quarters) for %d price quarters", weather_count, prices.length);
        return ENERGY_STATUS_DATA_MISSING;
    }

    write_advice_report(advice_dir, filename, "\n============================================= ENERGY ADVICE FOR %04d-%02d-%02d ============================================\n\n", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday + 1);
    write_advice_report(advice_dir, filename, "How to handle this information:\n");
    write_advice_report(advice_dir, filename, "The numbers in the table below are graded between 0 and 1, and will help you to evaluate your choices with more care.\n");
    write_advice_report(advice_dir, filename, "If the number is 1 or close to = A strong recommendation as this field is optimal for this quarter.\n");
    write_advice_report(advice_dir, filename, "If the number is 0 or close to = A recommendation to AVOID these actions during this time as they are in the low range.\n");
    write_advice_report(advice_dir, filename, "\nA gentle reminder that all of these values are only a recommendation based on the information gathered, not a definitive result.\n\n");
    

    write_advice_report(advice_dir, filename, "========================================== Low price threshold: %.3f SEK/kWh =========================================\n", low_price);
    write_advice_report(advice_dir, filename, "========================================== High price threshold: %.3f SEK/kWh ========================================\n\n", high_price);    

    write_advice_report(advice_dir, filename, "Time             | Sun  | Price | Norm | Charge (Grid/Source) | Consume (Grid/Source/Battery) | Sell (Battery/Source) |\n");
    write_advice_report(advice_dir, filename, "-----------------+------+-------+------+----------------------+-------------------------------+-----------------------+\n");

    for (i = 0; i < count; i++)
    {
        OpenMeteo_Quarter *weather_quarter = &weather.quarters[weather_start + i];
        Spotprice_Quarter *price_quarter = &prices.quarters[i];
        

        char timebuf[256];
        snprintf(
            timebuf, sizeof(timebuf), 
            "%04d-%02d-%02d %02d:%02d", 
            price_quarter->time_start.tm_year + 1900,
            price_quarter->time_start.tm_mon + 1,
            price_quarter->time_start.tm_mday,
            price_quarter->time_start.tm_hour,
            price_quarter->time_start.tm_min
        );

        double sun_index = (weather_quarter->direct_radiation + weather_quarter->diffuse_radiation) / 300.0;
        float price_norm = normalize_price(price_quarter->SEK_per_kWh, low_price, high_price);

        Battery_State battery = { .soc = 0.5f };

        Energy_Flow_Advice advice = compute_advice(price_norm, sun_index, battery.soc);

        if (sun_index > 1.0)
        {
            sun_index = 1.0;
        }
        
        write_advice_report(advice_dir, filename,  
            "%s | %.2f | %.3f | %.2f | "
            " G: %.2f  | S: %.2f  | "
            "G: %.2f  | S: %.2f | B: %.2f  | "
            "  B: %.2f  |  S: %.2f |\n", timebuf, sun_index, price_quarter->SEK_per_kWh, price_norm, advice.charge_from_grid, advice.charge_from_source, advice.consume_from_grid, advice.consume_from_source, advice.consume_from_battery, advice.sell_from_battery, advice.sell_from_source);
    }

    Logger_Dispose(&energy_advisor_log);
    OpenMeteo_Destroy(&weather);
    Spotprice_Destroy(&prices);

    return ENERGY_STATUS_OK;
}