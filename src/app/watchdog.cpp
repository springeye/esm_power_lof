/**
 * @file watchdog.cpp
 * @brief Task WDT 封装实现
 */

#include "watchdog.h"
#include "app_config.h"
#include <esp_task_wdt.h>
#include <esp_idf_version.h>

namespace watchdog {

void init(uint32_t timeout_s) {
#if ESP_IDF_VERSION >= ESP_IDF_VERSION_VAL(5, 0, 0)
    esp_task_wdt_config_t cfg = {};
    cfg.timeout_ms = timeout_s * 1000;
    cfg.idle_core_mask = 0;
    cfg.trigger_panic = true;
    esp_task_wdt_init(&cfg);
#else
    esp_task_wdt_init(timeout_s, true);
#endif
}

void feed() {
    esp_task_wdt_reset();
}

} // namespace watchdog
