#ifndef ENERGY_HELPER_H
#define ENERGY_HELPER_H

/**
 * @brief Utility functions used by the energy analysis engine.
 * 
 * This module provides helper functions for:
 * 
 * - value normalization
 * - price comparison
 * - score extraction
 * - time formatting
 * 
 * These helpers are intentionally lightweight and stateless.
 */

#include <stddef.h>

#include "energy_types.h"

/**
 * @brief Clamps a floating point value to the range [0,1].
 * 
 * Extremely small floating point noise values are treated as zero.
 * 
 * @param value Input value.
 * 
 * @return Clamped value within [0,1].
 */
float clamp_value(float value);

int compare_price(const void *a, const void *b);

/**
 * @brief Noramlizes an electricity price into the range [0,1].
 * 
 * Values below the low threshold map to 0,
 * values above the high threshold map to 1.
 * 
 * This normalization is used to make price comparisons independent
 * of absolute currency values.
 * 
 * @param price Raw electricity price.
 * @param low Lower quartile threshold.
 * @param high Higher quartile threshold.
 * 
 * @return Nomarlized price value.
 */
float normalize_price(float price, float low, float high);

float score_charge_from_grid(const Quarter_Score *q);

float score_charge_from_source(const Quarter_Score *q);

float score_sell(const Quarter_Score *q);

float score_consume(const Quarter_Score *q);

/**
 * @brief Formats a timestamp as ISO-8601.
 * 
 * Output format:
 * 
 * YYYY-MM-DDTHH:MM:SS
 * 
 * @param t Input time structure.
 * @param buf Destination buffer.
 * @param size Size of buffer.
 */
void time_helper(const struct tm *t, char *buf, size_t size);

#endif