/**
 * @file i2c_bus.cpp
 * @brief I2C HAL implementation for ESP32 Fan Controller
 */

#include "hal/i2c_bus.h"
#include "pins.h"
#include "app_config.h"

static SemaphoreHandle_t s_i2c_mutex = nullptr;

void i2c_bus_init(void) {
    // Create mutex before Wire.begin to ensure thread safety from the start
    s_i2c_mutex = xSemaphoreCreateMutex();
    configASSERT(s_i2c_mutex != nullptr);

    Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ_HZ);
}

void i2c_scan(void) {
    Serial.println("[I2C] Scanning bus...");
    uint8_t count = 0;
    for (uint8_t addr = 1; addr < 127; addr++) {
        Wire.beginTransmission(addr);
        uint8_t err = Wire.endTransmission();
        if (err == 0) {
            Serial.printf("[I2C] Device found at 0x%02X\n", addr);
            count++;
        }
    }
    Serial.printf("[I2C] Scan complete: %u device(s) found\n", count);
}

bool i2c_bus_take(uint32_t timeout_ms) {
    if (s_i2c_mutex == nullptr) return false;
    return xSemaphoreTake(s_i2c_mutex, pdMS_TO_TICKS(timeout_ms)) == pdTRUE;
}

void i2c_bus_give(void) {
    if (s_i2c_mutex != nullptr) {
        xSemaphoreGive(s_i2c_mutex);
    }
}

TwoWire& i2c_get_wire(void) {
    return Wire;
}
