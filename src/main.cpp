#include <Arduino.h>
#include <esp_ota_ops.h>

#include "hal/i2c_bus.h"
#include "hal/spi_bus.h"
#include "display/tft_driver.h"
#include "display/tft_demo.h"
#include "display/screen_rotation.h"
#include "sensors/ina226/ina226.h"
#include "fan/fan_pwm.h"
#include "fan/fan_tach.h"
#include "power/ps_on.h"
#include "app/watchdog.h"
#include "app/tasks.h"
#include "app/config_manager.h"
#include "app_config.h"

void setup() {
    Serial.begin(115200);
    Serial.println("[FanCtrl] Booting...");

    // OTA回滚保护：标记当前固件为有效
    esp_ota_mark_app_valid_cancel_rollback();

    // ─── 显示驱动（demo / 正式 共用） ───
    spi_bus_init();
    tft_driver::init();

    // ─── DEMO 分支：仅跑屏幕测试图案，跳过其余初始化 ───
    if constexpr (USE_DISPLAY_DEMO) {
        Serial.println("[FanCtrl] >>> DISPLAY DEMO MODE <<<");
        Serial.println("[FanCtrl] Set USE_DISPLAY_DEMO=false in app_config.h to run normal UI.");
        tft_demo::run();   // 永不返回
    }

    // ─── 正式 UI 分支 ───
    i2c_bus_init();

    // Sensors & peripherals
    ina226_init_all();
    fan_pwm_init();
    fan_tach_init();
    ps_on_init();
    // keys: no global init needed; key_debounce_update() called per-key in inputTask

    // Runtime config (loads NVS defaults)
    config_manager::init();

    // 把已保存的 INA226 校准增益/偏移下发到传感器驱动
    for (uint8_t ch = 0; ch < 3u; ++ch) {
        ina226_set_gain(static_cast<Ina226Rail>(ch),
                        config_manager::get_ina_gain(ch));
        ina226_set_offset(static_cast<Ina226Rail>(ch),
                          config_manager::get_ina_offset(ch));
    }

    // Screen rotation (loads from NVS)
    screen_rotation::init();

    // Watchdog
    watchdog::init(TASK_WDT_TIMEOUT_S);

    // Start FreeRTOS tasks (non-returning)
    // LVGL init (lvgl_port::init, UI creation, screen load) happens inside lvgl_task()
    tasks::start_all();

    Serial.println("[FanCtrl] Tasks started.");
}

void loop() {
    // All work is done in FreeRTOS tasks.
    // loop() is intentionally empty.
    vTaskDelay(portMAX_DELAY);
}
