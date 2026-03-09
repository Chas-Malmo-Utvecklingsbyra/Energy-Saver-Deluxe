#ifndef ENERGY_HELPER_H
#define ENERGY_HELPER_H

#include <stddef.h>

#include "energy_types.h"

float clamp_value(float v);

int compare_price(const void *a, const void *b);

float normalize_price(float price, float low, float high);

float score_charge(const Quarter_Score *q);

float score_sell(const Quarter_Score *q);

float score_consume(const Quarter_Score *q);

void time_helper(const struct tm *t, char *buf, size_t size);

#endif