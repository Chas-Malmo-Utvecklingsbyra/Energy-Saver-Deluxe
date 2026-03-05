#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>

#include "energy_types.h"
#include "energy_advisor.h"
#include "file_helper/file_helper.h"
#include "logger/logger.h"

// Using a standard amount of 4 quarters per window here to avoid getting trash-values just because one quarter has high values
// Could specify different windows for the separate actions if we want to be even more specific, but this is fine for now
#define MIN_WINDOW_QUARTERS 4


static inline float clamp_value(float v)
{
    if (v < 0.0f) return 0.0f;
    if (v > 1.0f) return 1.0f;
    if (fabsf(v) < 1e-6f) return 0.0f;
    return v;
}

int compare_price(const void *a, const void *b)
{
    float pa = *(const float*)a;
    float pb = *(const float*)b;
    return (pa > pb) - (pa < pb);
}

float normalize_price(float price, float low, float high)
{
    if (price <= low)   return 0.0f;
    if (price >= high)  return 1.0f;

    return (price - low) / (high - low);
}

Energy_Flow_Advice compute_advice(float price_norm, float production, float battery_soc)
{
    Energy_Flow_Advice advice = {0};

    advice.charge_from_grid = clamp_value((1.0f - price_norm) * (1.0f - production) * (1.0f - battery_soc));
    advice.charge_from_source = clamp_value(production * (1.0f - battery_soc));
    advice.consume_from_grid = clamp_value((1.0f - production) * (1.0f - battery_soc) * (1.0f - price_norm));
    advice.consume_from_source = clamp_value(production);
    advice.consume_from_battery = clamp_value(battery_soc * price_norm);
    advice.sell_from_battery = clamp_value(battery_soc * price_norm);    
    advice.sell_from_source = clamp_value(production * price_norm * battery_soc);

    return advice;
}

Best_Time_Window find_best_window(Quarter_Score *data, int count, float (*score_fn)(const Quarter_Score *), float threshold)
{
    Best_Time_Window best = { -1, -1, 0.0f};

    int current_start = -1;
    float sum = 0.0f;
    int len = 0;
    int i;

    for (i = 0; i < count; i++)
    {
        float score = score_fn(&data[i]);

        if (score >= threshold)
        {
            if ( current_start == -1)
            {
                current_start = i;
                sum = 0.0f;
                len = 0;
            }
            sum += score;
            len++;
        }
        else if (current_start != -1)
        {
            if (len >= MIN_WINDOW_QUARTERS)
            {
                float avg = sum / len;
                if (avg > best.average_score)
                {
                    best.start = current_start;
                    best.end = i -1;
                    best.average_score = avg;
                }
            }
            current_start = -1;
        }
    }

    if (current_start != -1 && len >= MIN_WINDOW_QUARTERS)
    {
        float avg = sum / len;
        if (avg > best.average_score)
        {
            best.start = current_start;
            best.end = count - 1;
            best.average_score = avg;
        }
    }

    return best;
}

float score_charge(const Quarter_Score *q)
{
    return q->advice.charge_from_grid;
}

float score_sell(const Quarter_Score *q)
{
    return q->advice.sell_from_battery + q->advice.sell_from_source;
}

float score_consume(const Quarter_Score *q)
{
    return q->advice.consume_from_source;
}

void time_helper(const struct tm *t, char *buf, size_t size)
{
    snprintf(buf, size, "%04d-%02d-%02dT%02d:%02d:00", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min);
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

void write_json_report(const char *path, const char *filename, const Quarter_Score *analysis, int count, const struct tm *date)
{
    char buf[1024];
    File_Helper_Write(path, filename, "{\n", 2, FILE_HELPER_MODE_WRITE, true);

    snprintf(buf, sizeof(buf), "  \"date\": \"%04d-%02d-%02d\",\n  \"quarters\": [\n", date->tm_year + 1900, date->tm_mon + 1, date->tm_mday);
    File_Helper_Write(path, filename, buf, strlen(buf), FILE_HELPER_MODE_APPEND, false);

    int i;
    for(i = 0; i < count; i++)
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
            (i < count - 1) ? "," : ""
        );
        
        File_Helper_Write(path, filename, buf, strlen(buf), FILE_HELPER_MODE_APPEND, false);
    }

    File_Helper_Write(path, filename, "  ]\n}\n", 6, FILE_HELPER_MODE_APPEND, false);
}