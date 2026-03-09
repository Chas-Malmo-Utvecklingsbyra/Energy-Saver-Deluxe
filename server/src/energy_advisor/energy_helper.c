#include <stdio.h>
#include <math.h>

#include "energy_types.h"
#include "energy_helper.h"

float clamp_value(float value)
{
    if (value <= 0.0f) return 0.0f;
    if (value >= 1.0f) return 1.0f;

    if (fabsf(value) < 1e-6f) return 0.0f;
    
    return value;
}

int compare_price(const void *a, const void *b)
{
    float pa = *(const float*)a;
    float pb = *(const float*)b;
    return (pa > pb) - (pa < pb);
}

float normalize_price(float price, float low, float high)
{
    if (high <= low)    return 0.0f;
    
    if (price <= low)   return 0.0f;
    if (price >= high)  return 1.0f;

    return (price - low) / (high - low);
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