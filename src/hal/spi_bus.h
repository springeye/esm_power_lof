#pragma once
/**
 * @file spi_bus.h
 * @brief SPI HAL abstraction for ESP32 Fan Controller
 *
 * SPI bus is managed internally by TFT_eSPI library.
 * This module provides documentation, mutex reservation,
 * and a unified init point for future SPI device additions.
 *
 * NOTE: Do NOT re-initialize SPI here; TFT_eSPI manages VSPI/HSPI.
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize SPI HAL (mutex only; TFT_eSPI manages the bus)
 *
 * Creates a FreeRTOS mutex for future SPI device arbitration.
 * Must be called before tft_driver_init().
 */
void spi_bus_init(void);

/**
 * @brief Take SPI bus mutex (blocking)
 *
 * Reserved for future multi-device SPI arbitration.
 * Currently only TFT_eSPI uses the SPI bus.
 *
 * @param timeout_ms Maximum wait time in milliseconds
 * @return true if mutex acquired, false on timeout
 */
bool spi_bus_take(uint32_t timeout_ms);

/**
 * @brief Release SPI bus mutex
 */
void spi_bus_give(void);

#ifdef __cplusplus
}
#endif
