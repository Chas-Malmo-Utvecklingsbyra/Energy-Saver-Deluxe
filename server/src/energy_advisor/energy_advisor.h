#ifndef ENERGY_ADVISOR_H
#define ENERGY_ADVISOR_H

#include "energy_types.h"
#include "../parser/weather_new/weather.h"
#include "../parser/energy_snapshot/energy_snapshot.h"
#include "../parser/spotprice/spotprice.h"


int compare_price(const void *a, const void *b);

float normalize_price(float price, float low, float high);

float score_charge(const Quarter_Score *q);

float score_sell(const Quarter_Score *q);

float score_consume(const Quarter_Score *q);

void tm_to_iso8601(const struct tm *t, char *buf, size_t size);

void write_advice_report(const char *path, const char *filename, const char *fmt, ...);

void write_json_report(const char *path, const char *filename, const Quarter_Score *analysis, int count, const struct tm *date);

Energy_Flow_Advice compute_advice(float price_norm, float production, float battery_soc);

Best_Time_Window find_best_window(Quarter_Score *data, int count, float (*score_fn)(const Quarter_Score *), float threshold);

Energy_Status Energy_Advisor_Advice();


#endif