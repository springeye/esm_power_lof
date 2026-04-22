#pragma once
#include <cstdint>

/**
 * @brief Convert temperature to fan PWM duty cycle (10-bit, 0-1023).
 *
 * Piecewise linear curve:
 *   T < 30°C  → 20% (205)
 *   30-50°C   → linear 20%→60% (205→614)
 *   50-70°C   → linear 60%→100% (614→1023)
 *   T ≥ 70°C  → 100% (1023)
 *   T ≥ 75°C  → force 100% (emergency)
 *
 * @param temp_c  Temperature in °C
 * @return PWM duty cycle (0–1023)
 */
uint16_t fan_temp_to_pwm(float temp_c);

/**
 * @brief Apply hysteresis to a temperature reading.
 *
 * If current is within [target-band, target+band], return target.
 * Otherwise return current (follow the temperature).
 *
 * @param current  Current temperature reading
 * @param target   Target/reference temperature
 * @param band     Hysteresis band (±)
 * @return Effective temperature after hysteresis
 */
float hysteresis_apply(float current, float target, float band);
