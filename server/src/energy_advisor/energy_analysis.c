#define _POSIX_C_SOURCE 199309L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>

#include "energy_analysis.h"
#include "file_helper/file_helper.h"
#include "logger/logger.h"
#include "utils/radix_sort.h"
//#include "benchmark/benchmark.h"

#define MIN_WINDOW_QUARTERS 4

/**
 * ENERGY MODEL OVERVIEW
 * 
 * Each day is divided into 96 quarters (15-minute intervals).
 * 
 * For each quarter the system evaluates:
 * 
 * - Electricity prices
 * - Solar production potential
 * - Battery state of charge
 * 
 * These inputs produce normalized recommendation scores for:
 * 
 * - Charging the battery
 * - Consuming energy
 * - Selling energy
 * 
 * Scores range from 0 to 1 where:
 * 
 * 0 = strongly discouraged
 * 1 = strongly recommended
 * 
 * The system then scans the results to identify optimal time windoes
 * for each energy action.
 */

Energy_Flow_Advice grading_actions(float price_norm, float production, float battery_soc)
{
    Energy_Flow_Advice advice = {0};

    float inv_price = (1.0f - price_norm);
    float inv_prod = (1.0f - production);
    float inv_soc = (1.0f - battery_soc);

    // Makes the decision promote selling energy over consuming
    float sell_bias = 1.2f;
    
    float grid_charge = inv_price * inv_soc;
    float grid_consume = inv_price * inv_prod;
    float grid_total = grid_charge + grid_consume + 1e-6f;

    float bat_consume = battery_soc * price_norm;
    float bat_sell = battery_soc * price_norm * sell_bias;
    float bat_total = bat_consume + bat_sell + 1e-6f;
    
    advice.charge_from_grid = clamp_value(grid_charge / grid_total);
    advice.charge_from_source = clamp_value(production * inv_soc);
    advice.consume_from_grid = clamp_value(grid_consume / grid_total);
    advice.consume_from_source = clamp_value(production);
    advice.consume_from_battery = clamp_value(bat_consume / bat_total);
    advice.sell_from_battery = clamp_value(bat_sell / bat_total);    
    advice.sell_from_source = clamp_value(production * price_norm * battery_soc);

    return advice;
}

Best_Time_Window find_best_window(Quarter_Score *data, int count, float (*score_fn)(const Quarter_Score *), float threshold)
{
    Best_Time_Window best = { -1, -1, 0.0f, false };

    int current_start = -1;
    float sum = 0.0f;
    int len = 0;
    int i;

    for (i = 0; i < count; i++)
    {
        float score = score_fn(&data[i]);

        if (score >= threshold)
        {
            if (current_start < 0)
                current_start = i;

            sum += score;
            len++;
        }
        else if (current_start >= 0)
        {
            if (len >= MIN_WINDOW_QUARTERS)
            {
                float avg = sum / len;
                if (!best.found || avg > best.average_score)
                {
                    best.start = current_start;
                    best.end = i - 1;
                    best.average_score = avg;
                    best.found = true;
                }
            }
            current_start = -1;
            sum = 0.0f;
            len = 0;
        }
    }

    if (current_start >= 0 && len >= MIN_WINDOW_QUARTERS)
    {
        float avg = sum / len;
        if (!best.found || avg > best.average_score)
        {
            best.start = current_start;
            best.end = count - 1;
            best.average_score = avg;
            best.found = true;
        }
    }

    return best;
}

int Energy_Find_Weather_Start(OpenMeteo_Data *weather, const struct tm *date, int *out_count)
{
    int weather_start = -1;
    int weather_count = 0;

    int i;
    for (i = 0; i < weather->length; i++)
    {
        struct tm *t = &weather->quarters[i].time;

        if (t->tm_year == date->tm_year &&
            t->tm_mon == date->tm_mon &&
            t->tm_mday == date->tm_mday)
        {
            if (weather_start == -1)
                weather_start = i;

            weather_count++;
        }
    }

    *out_count = weather_count;
    return weather_start;
}

Quarter_Score *Energy_Run_Analysis(OpenMeteo_Data *weather, Spotprice_Data *prices, int *out_count, float *out_low, float *out_high, int weather_offset)
{
    int i;
    int count = prices->length;
    
    Logger energy_analysis_log = {0};
    const char log_filename[] = "Energy_Analysis_Log.txt";
    Logger_Init(&energy_analysis_log, "ENERGY ANALYSIS", "logfolder", log_filename, LOGGER_OUTPUT_TYPE_FILE_TEXT);

    Quarter_Score *analysis = calloc(count, sizeof(Quarter_Score));
    if (!analysis)
    {
        LOG_WRITE(&energy_analysis_log, LOGGER_LEVEL_ERROR, "Analysis data is missing");
        return NULL;
    }

    float *price_buffer = malloc(sizeof(float) *count);
    if (!price_buffer)
    {
        free(analysis);
        LOG_WRITE(&energy_analysis_log, LOGGER_LEVEL_ERROR, "Price buffer is empty");
        return NULL;
    }

    for (i = 0; i < count; i++)
    {
        price_buffer[i] = prices->quarters[i].SEK_per_kWh;
    }

    //Benchmark benchmark = {0};

    //printf("Benchmarking radix sort...\r\n");

    //Benchmark_Start(&benchmark);
    radix_sort_float(price_buffer, count);
    //Benchmark_Stop(&benchmark);

    //Benchmark_Print(&benchmark);

    int low_index = ((count - 1) * 0.25f);
    int high_index = ((count - 1) * 0.75f);

    float low_price = price_buffer[low_index];
    float high_price = price_buffer[high_index];

    free(price_buffer);

    for (i = 0; i < count; i++)
    {
        OpenMeteo_Quarter *weather_quarter = &weather->quarters[weather_offset + i];
        Spotprice_Quarter *price_quarter = &prices->quarters[i];

        double sun_index = (weather_quarter->direct_radiation + weather_quarter->diffuse_radiation) / 300.0;
        if (sun_index > 1.0)
            sun_index = 1.0;

        float price_norm = normalize_price(price_quarter->SEK_per_kWh, low_price, high_price);

        float battery_soc = 0.5f;

        Energy_Flow_Advice advice = grading_actions(price_norm, sun_index, battery_soc);
        analysis[i].time = price_quarter->time_start;
        analysis[i].price = price_quarter->SEK_per_kWh;
        analysis[i].price_norm = price_norm;
        analysis[i].sun = sun_index;
        analysis[i].advice = advice;
    }

    *out_count = count;
    *out_low = low_price;
    *out_high = high_price;

    Logger_Dispose(&energy_analysis_log);

    return analysis;
}

void write_advice_report(const char *path, const char *filename, const char *fmt, ...)
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

void write_advice_report_header(const char *path, const char *filename, const struct tm *date, float low_price, float high_price)
{
    write_advice_report(path, filename, "\n============================================= ENERGY ADVICE FOR %04d-%02d-%02d ============================================\n\n"
                                            "How to handle this information:\n"
                                            "The numbers in the table below are graded between 0 and 1, and will help you to evaluate your choices with more care.\n"
                                            "If the number is 1 or close to = A strong recommendation as this field is optimal for this quarter.\n"
                                            "If the number is 0 or close to = A recommendation to AVOID these actions during this time as they are in the low range.\n"
                                            "Norm is the normalized price info - where 1 indicates that this is a high price compared to the rest of the day.\n\n"
                                            "A gentle reminder that all of these values are only a recommendation based on the information gathered, not a definitive result.\n\n"
                                            "========================================== Low price threshold: %.3f SEK/kWh =========================================\n"
                                            "========================================== High price threshold: %.3f SEK/kWh ========================================\n\n",
                                            date->tm_year + 1900, date->tm_mon + 1, date->tm_mday + 1, low_price , high_price);
}

Energy_Summary calculate_summary(Quarter_Score *analysis, int count)
{
    float *scores = malloc(sizeof(float) * count);

    for (int i = 0; i < count; i++)
    {
        scores[i] = score_charge(&analysis[i]);
    }
    radix_sort_float(scores, count);
    float charge_threshold = scores[(int)((count - 1) * 0.75f)];
    
    for (int i = 0; i < count; i++)
    {
        scores[i] = score_consume(&analysis[i]);
    }
    radix_sort_float(scores, count);
    float consume_threshold = scores[(int)((count - 1) * 0.75f)];
    
    for (int i = 0; i < count; i++)
    {
        scores[i] = score_sell(&analysis[i]);
    }
    radix_sort_float(scores, count);
    float sell_threshold = scores[(int)((count - 1) * 0.75f)];
    
    Energy_Summary summary;
    
    summary.charge = find_best_window(analysis, count, score_charge, charge_threshold);
    summary.consume = find_best_window(analysis, count, score_consume, consume_threshold);
    summary.sell = find_best_window(analysis, count, score_sell, sell_threshold);

    free(scores);

    return summary;
}

void write_advice_report_summary(const char *path, const char *filename, Quarter_Score *analysis, Energy_Summary *summary)
{
    Best_Time_Window best_charge = summary->charge;       
    Best_Time_Window best_consume = summary->consume;
    Best_Time_Window best_sell = summary->sell;

    write_advice_report(path, filename, "\n====================== SUMMARY FOR THE DAY ======================\n\n");

    if (best_charge.start != -1)
    {
        write_advice_report(path, filename, "The best time to CHARGE from grid: %02d:%02d - %02d:%02d (avg %.2f)\n",
        analysis[best_charge.start].time.tm_hour,
        analysis[best_charge.start].time.tm_min,
        analysis[best_charge.end].time.tm_hour,
        analysis[best_charge.end].time.tm_min,
        best_charge.average_score);
    }
    else
    {
        write_advice_report(path, filename, "The best time to CHARGE from grid: There is no window that fulfills the requirements today\n");
    }

    if (best_consume.start != -1)
    {
        write_advice_report(path, filename, "The best time to CONSUME solar: %02d:%02d - %02d:%02d (avg %.2f)\n",
        analysis[best_consume.start].time.tm_hour,
        analysis[best_consume.start].time.tm_min,
        analysis[best_consume.end].time.tm_hour,
        analysis[best_consume.end].time.tm_min,
        best_consume.average_score);
    }
    else
    {
        write_advice_report(path, filename, "The best time to CONSUME solar: There is no window that fulfills the requirements today\n");
    }

    if (best_sell.start != -1)
    {
        write_advice_report(path, filename, "The best time to SELL energy: %02d:%02d - %02d:%02d (avg %.2f)\n",
        analysis[best_sell.start].time.tm_hour,
        analysis[best_sell.start].time.tm_min,
        analysis[best_sell.end].time.tm_hour,
        analysis[best_sell.end].time.tm_min,
        best_sell.average_score);
    }
    else
    {
        write_advice_report(path, filename, "The best time to SELL energy: There is no window that fulfills the requirements today\n");
    }

    write_advice_report(path, filename, "\n=================================================================\n");
}

void write_advice_report_remaining(const char *path, const char *filename, Quarter_Score *analysis, int count)
{
     write_advice_report(path, filename, "\n=================================================================\n\n"
                                            "Time             | Sun  | Price | Norm | Charge (Grid/Source) | Consume (Grid/Source/Battery) | Sell (Battery/Source) |\n"
                                            "-----------------+------+-------+------+----------------------+-------------------------------+-----------------------+\n");

    int i;
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

        write_advice_report(path, filename,
            "%s | %.2f | %.3f | %.2f | "
            " G: %.2f  | S: %.2f  | "
            "G: %.2f  | S: %.2f | B: %.2f  | "
            "  B: %.2f  |  S: %.2f |\n", 
            timebuf, analysis[i].sun, analysis[i].price, analysis[i].price_norm, 
            analysis[i].advice.charge_from_grid, analysis[i].advice.charge_from_source, 
            analysis[i].advice.consume_from_grid, analysis[i].advice.consume_from_source, analysis[i].advice.consume_from_battery, 
            analysis[i].advice.sell_from_battery, analysis[i].advice.sell_from_source);
    }
}

void Energy_Write_JSON_Report(const char *path, const char *filename, Quarter_Score *analysis, const struct tm *date, Energy_Summary *summary, int count)
{
    char buf[1024];
    File_Helper_Write(path, filename, "{\n", 2, FILE_HELPER_MODE_WRITE, true);

    char charge_buf[256];
    char consume_buf[256];
    char sell_buf[256];

    if (!summary->charge.found || summary->charge.start >= count || summary->charge.end >= count)
    {
        snprintf(charge_buf, sizeof(charge_buf), "\"charge\": { \"found\": false }");
    }
    else
    {
        snprintf(charge_buf, sizeof(charge_buf), 
            "\"charge\": { \"found\": true, \"start\": \"%02d:%02d\", \"end\": \"%02d:%02d\", \"avg\": %.2f }", 
            analysis[summary->charge.start].time.tm_hour,
            analysis[summary->charge.start].time.tm_min,
            analysis[summary->charge.end].time.tm_hour,
            analysis[summary->charge.end].time.tm_min,
            summary->charge.average_score);
    }

    if (!summary->consume.found || summary->consume.start >= count || summary->consume.end >= count)
    {
        snprintf(consume_buf, sizeof(consume_buf), "\"consume\": { \"found\": false }");
    }
    else
    {
        snprintf(consume_buf, sizeof(consume_buf), 
            "\"consume\": { \"found\": true, \"start\": \"%02d:%02d\", \"end\": \"%02d:%02d\", \"avg\": %.2f }",
            analysis[summary->consume.start].time.tm_hour,
            analysis[summary->consume.start].time.tm_min,
            analysis[summary->consume.end].time.tm_hour,
            analysis[summary->consume.end].time.tm_min,
            summary->consume.average_score);        
    }
    
    if (!summary->sell.found || summary->sell.start >= count || summary->sell.end >= count)
    {
        snprintf(sell_buf, sizeof(sell_buf), "\"sell\": { \"found\": false }");
    }
    else
    {
        snprintf(sell_buf, sizeof(sell_buf), 
            "\"sell\": { \"found\": true, \"start\": \"%02d:%02d\", \"end\": \"%02d:%02d\", \"avg\": %.2f }",
            analysis[summary->sell.start].time.tm_hour,
            analysis[summary->sell.start].time.tm_min,
            analysis[summary->sell.end].time.tm_hour,
            analysis[summary->sell.end].time.tm_min,
            summary->sell.average_score);
    }

    
    snprintf(buf, sizeof(buf), 
        "  \"date\": \"%04d-%02d-%02d\",\n"
        "  \"summary\": {\n"
        "    %s,\n"
        "    %s,\n"
        "    %s\n"
        "  },\n"
        "  \"quarters\": [\n", 
        date->tm_year + 1900, date->tm_mon + 1, date->tm_mday, charge_buf, consume_buf, sell_buf
    );

    File_Helper_Write(path, filename, buf, strlen(buf), FILE_HELPER_MODE_APPEND, false);

    size_t i;
    for(i = 0; i < (size_t)count; i++)
    {
        char timebuf[256];
        time_helper(&analysis[i].time, timebuf, sizeof(timebuf));

        snprintf(buf, sizeof(buf),
            "   {\n"
            "     \"time\": \"%s\",\n"
            "     \"sun\": %.3f,\n"
            "     \"price_sek_kwh\": %.3f,\n"
            "     \"price_norm\": %.3f,\n"
            "     \"advice\": {\n"
            "       \"charge\": { \"grid\": %.3f, \"source\": %.3f },\n"
            "       \"consume\": { \"grid\": %.3f, \"source\": %.3f, \"battery\": %.3f },\n"
            "       \"sell\": { \"battery\": %.3f, \"source\": %.3f }\n"
            "     }\n"
            "   }%s\n",
            timebuf,
            analysis[i].sun,
            analysis[i].price,
            analysis[i].price_norm,
            analysis[i].advice.charge_from_grid,
            analysis[i].advice.charge_from_source,
            analysis[i].advice.consume_from_grid,
            analysis[i].advice.consume_from_source,
            analysis[i].advice.consume_from_battery,
            analysis[i].advice.sell_from_battery,
            analysis[i].advice.sell_from_source,
            (i < (size_t)count - 1) ? "," : ""
        );
        
        File_Helper_Write(path, filename, buf, strlen(buf), FILE_HELPER_MODE_APPEND, false);
    }

    File_Helper_Write(path, filename, "  ]\n}\n", 6, FILE_HELPER_MODE_APPEND, false);
}
