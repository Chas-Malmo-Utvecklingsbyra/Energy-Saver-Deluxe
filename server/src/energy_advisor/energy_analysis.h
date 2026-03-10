#ifndef ENERGY_ANALYSIS_H
#define ENERGY_ANALYSIS_H

#include <time.h>

#include "energy_helper.h"
#include "../parser/weather_new/weather.h"
#include "../parser/spotprice/spotprice.h"

Energy_Flow_Advice compute_advice(float price_norm, float production, float battery_soc);

int Energy_Find_Weather_Start(OpenMeteo_Data *weather, const struct tm *date, int *out_count);

Quarter_Score *Energy_Run_Analysis(OpenMeteo_Data *weather, Spotprice_Data *prices, int *out_count, float *out_low, float *out_high, int weather_offset);

Best_Time_Window find_best_window(Quarter_Score *data, int count, float (*score_fn)(const Quarter_Score *), float threshold);

void write_advice_report_header(const char *path, const char *filename, const struct tm *date, float low_price, float high_price);

Energy_Summary calculate_summary(Quarter_Score *analysis, int count);

void write_advice_report_summary(const char *path, const char *filename, Quarter_Score *analysis, Energy_Summary *summary, int count);

void write_advice_report(const char *path, const char *filename, const char *fmt, ...);

void Energy_Write_JSON_Report(const char *path, const char *filename, Quarter_Score *analysis, const struct tm *date, Energy_Summary *summary, int count);

#endif