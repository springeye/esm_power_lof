#pragma once
#include <cstdint>

/**
 * @brief Convert ADC reading to temperature using NTC beta formula.
 *
 * Circuit: Vref -- R_pullup -- NTC -- GND
 * V_ntc = Vref * R_ntc / (R_pullup + R_ntc)
 * ADC = V_ntc / Vref * ADC_MAX
 *
 * Beta formula: 1/T = 1/T0 + (1/B) * ln(R/R25)
 *
 * Boundary conditions (from app_config.h):
 *   ADC=0   → short circuit → returns NTC_TEMP_SHORT_C (150.0f)
 *   ADC=MAX → open circuit  → returns NTC_TEMP_OPEN_C  (-40.0f)
 *
 * @param adc_raw  12-bit ADC reading (0–4095)
 * @return Temperature in °C
 */
float ntc_adc_to_temp(uint16_t adc_raw);
