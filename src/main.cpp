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
    Serial.flush();

    // OTA回滚保护：标记当前固件为有效
    esp_ota_mark_app_valid_cancel_rollback();
    Serial.println("[boot] 01 ota-valid"); Serial.flush();

    // ─── 显示驱动（demo / 正式 共用） ───
    spi_bus_init();
    Serial.println("[boot] 02 spi-bus"); Serial.flush();
    tft_driver::init();
    Serial.println("[boot] 03 tft-init"); Serial.flush();

    // ─── DEMO 分支：仅跑屏幕测试图案，跳过其余初始化 ───
    if constexpr (USE_DISPLAY_DEMO) {
        Serial.println("[FanCtrl] >>> DISPLAY DEMO MODE <<<");
        Serial.println("[FanCtrl] Set USE_DISPLAY_DEMO=false in app_config.h to run normal UI.");
        tft_demo::run();   // 永不返回
    }

    // ─── 正式 UI 分支 ───
    i2c_bus_init();
    Serial.println("[boot] 04 i2c-bus"); Serial.flush();

    // Sensors & peripherals
    ina226_init_all();
    Serial.println("[boot] 05 ina226"); Serial.flush();
    fan_pwm_init();
    Serial.println("[boot] 06 fan-pwm"); Serial.flush();
    fan_tach_init();
    Serial.println("[boot] 07 fan-tach"); Serial.flush();
    ps_on_init();
    Serial.println("[boot] 08 ps-on"); Serial.flush();
    // keys: no global init needed; key_debounce_update() called per-key in inputTask

    // Runtime config (loads NVS defaults)
    config_manager::init();
    Serial.println("[boot] 09 config-mgr"); Serial.flush();

    // 把已保存的 INA226 校准增益/偏移下发到传感器驱动
    for (uint8_t ch = 0; ch < 3u; ++ch) {
        ina226_set_gain(static_cast<Ina226Rail>(ch),
                        config_manager::get_ina_gain(ch));
        ina226_set_offset(static_cast<Ina226Rail>(ch),
                          config_manager::get_ina_offset(ch));
    }
    Serial.println("[boot] 10 ina-cal-push"); Serial.flush();

    // Screen rotation (loads from NVS)
    screen_rotation::init();
    Serial.println("[boot] 11 screen-rot"); Serial.flush();

    // Watchdog
    watchdog::init(TASK_WDT_TIMEOUT_S);
    Serial.println("[boot] 12 watchdog"); Serial.flush();

    // Start FreeRTOS tasks (non-returning)
    // LVGL init (lvgl_port::init, UI creation, screen load) happens inside lvgl_task()
    Serial.println("[boot] 13 pre-start-all"); Serial.flush();
    tasks::start_all();

    Serial.println("[FanCtrl] Tasks started.");
    Serial.flush();
}

void loop() {
    // All work is done in FreeRTOS tasks.
    // loop() is intentionally empty.
    vTaskDelay(portMAX_DELAY);
}
