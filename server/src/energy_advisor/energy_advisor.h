#ifndef ENERGY_ADVISOR_H
#define ENERGY_ADVISOR_H

#include "energy_analysis.h"

/**
 * @brief High level orchestration for the energy advisor system.
 * 
 * This module coordinates the complete energy analysis workflow:
 * 
 * 1. Loads weather data and spot price data for each Swedish energy zone.
 * 2. Executes the energy analysis model.
 * 3. Generates human-readable reports.
 * 4. Generates machine-readable JSON reports.
 * 5. Writes diagnostic logs.
 * 
 * The module acts as the main entry point for the energy advisor subsystem.
 * 
 * External dependencies:
 * - Weather parser (OpenMeteo)
 * - Spot price parser (Elprisetjustnu)
 * - File Helper
 * - Logging subsystem
 */


/**
 * @brief Executes the complete energy advisor pipeline
 * 
 * This function performs the full analysis workflow for all configured energy zones:
 * 
 * - Loads weather forecast data
 * - Loads electricity spot prices
 * - Alings weather data with price timestamps
 * - Runs the quarter-based energy analysis
 * - Calculates daily summary windows
 * - Generates both text and JSON reports
 * 
 * The function iterates over all zones defined in the global 'zones' configuration table.
 * 
 * Generated output:
 * - Text report per zone
 * - JSON report per zone
 * - Log entries for failure
 * 
 * @return ENERGY_STATUS_OK on success.
 */
Energy_Status Energy_Advisor_Advice(void);

#endif