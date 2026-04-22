/**
 * @file watchdog.cpp
 * @brief Task WDT 封装实现
 */

#include "watchdog.h"
#include "app_config.h"
#include <esp_task_wdt.h>

namespace watchdog {

void init(uint32_t timeout_s) {
    // ESP-IDF v4 API (arduino-esp32 3.x uses IDF v5 but older boards use v4 API)
    // Use esp_task_wdt_init for compatibility
    esp_task_wdt_init(timeout_s, true);
}

void feed() {
    esp_task_wdt_reset();
}

} // namespace watchdog
