#pragma once
/**
 * @file i2c_bus.h
 * @brief I2C HAL abstraction for ESP32 Fan Controller
 *
 * Wraps Wire library with mutex protection for multi-task access.
 * Provides init, scan, and mutex lock/unlock API.
 */

#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize I2C bus with pins and frequency from app_config.h
 *
 * Calls Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ_HZ).
 * Creates FreeRTOS mutex for thread-safe access.
 * Must be called before any I2C device init.
 */
void i2c_bus_init(void);

/**
 * @brief Scan I2C bus and print found device addresses to Serial
 *
 * Debug utility. Prints addresses of all responding devices.
 * Not thread-safe by itself; call only during init phase.
 */
void i2c_scan(void);

/**
 * @brief Take I2C bus mutex (blocking)
 *
 * Must be called before any direct Wire transaction.
 * Always pair with i2c_bus_give().
 *
 * @param timeout_ms Maximum wait time in milliseconds
 * @return true if mutex acquired, false on timeout
 */
bool i2c_bus_take(uint32_t timeout_ms);

/**
 * @brief Release I2C bus mutex
 *
 * Must be called after i2c_bus_take() when transaction is complete.
 */
void i2c_bus_give(void);

/**
 * @brief Get the Wire instance (use only while holding mutex)
 *
 * @return Reference to the Wire TwoWire instance
 */
TwoWire& i2c_get_wire(void);

#ifdef __cplusplus
}
#endif
