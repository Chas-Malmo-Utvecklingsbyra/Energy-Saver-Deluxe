#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "energy_advisor.h"
#include "logger/logger.h"
#include "file_helper/file_helper.h"

#define ENERGY_ADVISOR_WEATHER_FILE     "data/weather/weather.json"
#define ENERGY_ADVISOR_SPOTPRICE_FILE   "data/price/price.json"


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

    Quarter_Score *analysis = calloc(count, sizeof(Quarter_Score));

    for (i = 0; i < count; i++)
    {
        OpenMeteo_Quarter *weather_quarter = &weather.quarters[weather_start + i];
        Spotprice_Quarter *price_quarter = &prices.quarters[i];        

        double sun_index = (weather_quarter->direct_radiation + weather_quarter->diffuse_radiation) / 300.0;
        if (sun_index > 1.0)
            sun_index = 1.0;

        float price_norm = normalize_price(price_quarter->SEK_per_kWh, low_price, high_price);

        Battery_State battery = { .soc = 0.5f };

        Energy_Flow_Advice advice = compute_advice(price_norm, sun_index, battery.soc);
        analysis[i].time = price_quarter->time_start;
        analysis[i].price = price_quarter->SEK_per_kWh;
        analysis[i].sun = sun_index;
        analysis[i].advice = advice;
    }

    Best_Time_Window best_charge = find_best_window(analysis, count, score_charge, 0.3f);       // Low threshold just to prove that it works
    Best_Time_Window best_consume = find_best_window(analysis, count, score_consume, 0.6f);     // Average threshold
    Best_Time_Window best_sell = find_best_window(analysis, count, score_sell, 0.6f);

    write_advice_report(advice_dir, filename, "\n====================== SUMMARY FOR THE DAY ======================\n\n");

    if (best_charge.start != -1)
    {
        write_advice_report(advice_dir, filename, "The best time to CHARGE from grid: %02d:%02d - %02d:%02d (avg %.2f)\n",
        analysis[best_charge.start].time.tm_hour,
        analysis[best_charge.start].time.tm_min,
        analysis[best_charge.end].time.tm_hour,
        analysis[best_charge.end].time.tm_min,
        best_charge.average_score);
    }
    else if (best_charge.start == -1)
    {
        write_advice_report(advice_dir, filename, "The best time to CHARGE from grid: There is no window that fulfills the requirements today\n");
    }

    if (best_consume.start != -1)
    {
        write_advice_report(advice_dir, filename, "The best time to CONSUME solar: %02d:%02d - %02d:%02d (avg %.2f)\n",
        analysis[best_consume.start].time.tm_hour,
        analysis[best_consume.start].time.tm_min,
        analysis[best_consume.end].time.tm_hour,
        analysis[best_consume.end].time.tm_min,
        best_consume.average_score);
    }
    else if (best_consume.start == -1)
    {
        write_advice_report(advice_dir, filename, "The best time to CONSUME solar: There is no window that fulfills the requirements today\n");
    }

    if (best_sell.start != -1)
    {
        write_advice_report(advice_dir, filename, "The best time to SELL energy: %02d:%02d - %02d:%02d (avg %.2f)\n",
        analysis[best_sell.start].time.tm_hour,
        analysis[best_sell.start].time.tm_min,
        analysis[best_sell.end].time.tm_hour,
        analysis[best_sell.end].time.tm_min,
        best_sell.average_score);
    }
    else if (best_sell.start == -1)
    {
        write_advice_report(advice_dir, filename, "The best time to SELL energy: There is no window that fulfills the requirements today\n");
    }

    write_advice_report(advice_dir, filename, "\n=================================================================\n\n");
    
    write_advice_report(advice_dir, filename, "Time             | Sun  | Price | Norm | Charge (Grid/Source) | Consume (Grid/Source/Battery) | Sell (Battery/Source) |\n");
    write_advice_report(advice_dir, filename, "-----------------+------+-------+------+----------------------+-------------------------------+-----------------------+\n");


    for (i = 0; i < count; i++)
    {
        char timebuf[256];
        snprintf(
            timebuf, sizeof(timebuf), 
            "%04d-%02d-%02d %02d:%02d", 
            analysis[i].time.tm_year + 1900,
            analysis[i].time.tm_mon + 1,
            analysis[i].time.tm_mday,
            analysis[i].time.tm_hour,
            analysis[i].time.tm_min
        );

        float price_norm = normalize_price(analysis[i].price, low_price, high_price);

        Energy_Flow_Advice *a = &analysis[i].advice;

        write_advice_report(advice_dir, filename,  
            "%s | %.2f | %.3f | %.2f | "
            " G: %.2f  | S: %.2f  | "
            "G: %.2f  | S: %.2f | B: %.2f  | "
            "  B: %.2f  |  S: %.2f |\n", timebuf, analysis[i].sun, analysis[i].price, price_norm, a->charge_from_grid, a->charge_from_source, a->consume_from_grid, a->consume_from_source, a->consume_from_battery, a->sell_from_battery, a->sell_from_source);

    }

    char json_filename[64];
    snprintf(json_filename, sizeof(json_filename), "Energy_Advice_%04d-%02d-%02d.json", tm_info->tm_year + 1900, tm_info->tm_mon + 1, tm_info->tm_mday);
    write_json_report(advice_dir, json_filename, analysis, count, &report_date);

    free(analysis);

    Logger_Dispose(&energy_advisor_log);
    OpenMeteo_Destroy(&weather);
    Spotprice_Destroy(&prices);

    return ENERGY_STATUS_OK;
}