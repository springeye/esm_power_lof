/**
 * @file spi_bus.cpp
 * @brief SPI HAL implementation for ESP32 Fan Controller
 *
 * TFT_eSPI manages the actual SPI peripheral.
 * This module only provides mutex infrastructure.
 */

#include "hal/spi_bus.h"

static SemaphoreHandle_t s_spi_mutex = nullptr;

void spi_bus_init(void) {
    s_spi_mutex = xSemaphoreCreateMutex();
    configASSERT(s_spi_mutex != nullptr);
    // TFT_eSPI will initialize the actual SPI peripheral in tft_driver_init()
}

bool spi_bus_take(uint32_t timeout_ms) {
    if (s_spi_mutex == nullptr) return false;
    return xSemaphoreTake(s_spi_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void spi_bus_give(void) {
    if (s_spi_mutex != nullptr) {
        xSemaphoreGive(s_spi_mutex);
    }
}
