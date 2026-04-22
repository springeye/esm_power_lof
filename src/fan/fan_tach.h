#pragma once
#include <cstdint>

/**
 * @brief Initialize fan tachometer using ESP32 PCNT (legacy driver).
 *
 * Uses driver/pcnt.h (legacy API) on FAN_TACH pin.
 * Counts rising edges per measurement window.
 */
void fan_tach_init(void);

/**
 * @brief Get fan speed in RPM.
 *
 * Reads PCNT counter, calculates RPM based on pulse count and elapsed time.
 * 4-wire fans: 2 pulses per revolution.
 *
 * @return Fan speed in RPM (0 if stalled)
 */
uint32_t fan_tach_get_rpm(void);
