#ifndef ENERGY_ANALYSIS_H
#define ENERGY_ANALYSIS_H

/**
 * @brief Core analysis engine for the energy advisor.
 * 
 * This module performs the core calculations used by the advisor system.
 * 
 * Responsibilities:
 * - Compute energy flow recommendations for each 15-minute quarter.
 * - Normalize electricity prices.
 * - Estimate solar production potential.
 * - Identify optimal time windows for charging, consuming, or selling energy.
 * - Generate report output.
 * 
 * The analysis combines:
 * - electricity price signals
 * - solar radiation estimates
 * - battery state of charge
 * 
 * Output is produced as a sequence of 'Quarter_Score' entries which contain
 * calculated recommendations for each quarter of the day.
 */


#include <time.h>

#include "energy_helper.h"
#include "../parser/weather_new/weather.h"
#include "../parser/spotprice/spotprice.h"


/**
 * @brief Computes energy flow recommendations for a single time slot.
 * 
 * This function produces normalized recommendation scores (0..1)
 * for how energy should flow within the system during a specific quarter of an hour.
 * 
 * The recommendations are based on three normalized inputs:
 * 
 * - electricity price (price_norm)
 * - solar production potential (production)
 * - battery state of charge (battery_soc)
 * 
 * The scoring model combines these factors multiplicateively to express
 * implicit "AND" conditions (for example: cheap price AND low production AND empty battery)
 * 
 * Certain action groups are internally normalized to introduce competition:
 * * Grid actions:
 * charge_from_grid vs consume_from_grid
 * 
 * * Battery actions:
 * consume_from_battery vs sell_from_battery
 * 
 * Within each group, scores are normalized so that they form a proportional distribution
 * rather than independent signals. This avoids conflicting recommandations and forces relative
 * prioritization.
 * 
 * A bias factor (sell_bias) is applied to battery selling to favor exporting energy over internal
 * consumption when conditions are otherwise equal.
 * 
 * Source-based actions (solar) are evaluated independently and are not part of a normalized competition group.
 * 
 * All outputs are clamped to the range [0, 1]
 * 
 * @param price_norm Normalized electricity price (0 = cheap, 1 = expensive)
 * @param production Normalized solar production potential (0..1)
 * @param battery_soc Battery state-of-charge (0..1)
 * 
 * @return Energy_Flow_Advice containing recommendation scores.
 * 
 * @note The returned values are heuristic scores, not absolute decisions.
 * 
 * A higher score indicates a stronger recommendation relative to other actions
 * in the same category.
 */
Energy_Flow_Advice grading_actions(float price_norm, float production, float battery_soc);

int Energy_Find_Weather_Start(OpenMeteo_Data *weather, const struct tm *date, int *out_count);

/**
 * @brief Finds the best continuous time window for a given action.
 * 
 * The function scans the entire day of quarter-based scores and identifies
 * the best contiguous window where the score exceeds a given threshold.
 * 
 * A valid window must satisfy:
 * - minimum length requirement
 * - score >= threshold
 * 
 * The window with the highest average score is selected.
 * 
 * @param data Array of quarter analysis results.
 * @param count Number of entries in the array.
 * @param score_fn Function used to extract a score from each entry.
 * @param threshold Minimum score required to include a quarter in the window.
 * 
 * @return Best_Time_Window describing the optimal interval.
 */
Best_Time_Window find_best_window(Quarter_Score *data, int count, float (*score_fn)(const Quarter_Score *), float threshold);

/**
 * @brief Performs the full quarter-level energy analysis.
 * 
 * This function combines weather data and electricity prices to produce
 * recommendations for each 15-minute quarter of the day.
 * 
 * Steps performed:
 * 
 * 1. Extract price distribution
 * 2. Compute low/high price thresholds (quartiles)
 * 3. Normalize electricity prices
 * 4. Estimate solar production index
 * 5. Compute energy flow recommendations
 * 
 * The result is an array of 'Quarter_Score' structures.
 * 
 * Memory for the returned array is allocated dynamically and must be released
 * by the caller.
 * 
 * @param weather Weather dataset.
 * @param prices Spot Price dataset.
 * @param out_count Returns a number of quarters analyzed.
 * @param out_low Returns the calculated low price threshold.
 * @param out_high Returns the calculated high price threshold.
 * @param weather_offset Offset used to align weather data with prices.
 * 
 * @return Dynamically allocated Quarter_Score array or NULL on failure.
 */
Quarter_Score *Energy_Run_Analysis(OpenMeteo_Data *weather, Spotprice_Data *prices, int *out_count, float *out_low, float *out_high, int weather_offset);

void write_advice_report(const char *path, const char *filename, const char *fmt, ...);

void write_advice_report_header(const char *path, const char *filename, const struct tm *date, float low_price, float high_price);

/**
 * @brief Calculate daily summary recommendations.
 * 
 * The summary identifies the optimal time windows for:
 * 
 * - Charging the battery
 * - Consuming solar production
 * - Selling electricity
 * 
 * Window detection uses scoring functions and configurable thresholds.
 * 
 * @param analysis Quarter-level analysis results.
 * @param count Number of quarters.
 * 
 * @return Energy_Summary containing the optimal windows.
 */
Energy_Summary calculate_summary(Quarter_Score *analysis, int count);

void write_advice_report_summary(const char *path, const char *filename, Quarter_Score *analysis, Energy_Summary *summary);

void write_advice_report_remaining(const char *path, const char *filename, Quarter_Score *analysis, int count);

void Energy_Write_JSON_Report(const char *path, const char *filename, Quarter_Score *analysis, const struct tm *date, Energy_Summary *summary, int count);

#endif