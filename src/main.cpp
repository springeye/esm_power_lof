#include <Arduino.h>

#include "hal/i2c_bus.h"
#include "hal/spi_bus.h"
#include "display/tft_driver.h"
#include "display/tft_demo.h"
#include "display/lvgl_port.h"
#include "ui_bridge/screen_manager.h"
#include "ui_bridge/data_bridge.h"
#include "ui_bridge/input_bridge.h"
extern "C" {
#include "../ui/lof_power_system.h"
}
#include "sensors/ina226/ina226.h"
#include "fan/fan_pwm.h"
#include "fan/fan_tach.h"
#include "power/ps_on.h"
#include "app/watchdog.h"
#include "app/tasks.h"
#include "app_config.h"

void setup() {
    Serial.begin(115200);
    Serial.println("[FanCtrl] Booting...");

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

    lvgl_port::init();

    lof_power_system_init(NULL);
    ui_bridge::screen_manager_init(2000);
    ui_bridge::data_bridge_attach(ui_bridge::screen_manager_get_home());
    ui_bridge::data_bridge_init();
    ui_bridge::input_bridge_attach_home(ui_bridge::screen_manager_get_home());

    // Sensors & peripherals
    ina226_init_all();
    fan_pwm_init();
    fan_tach_init();
    ps_on_init();
    // keys: no global init needed; key_debounce_update() called per-key in inputTask

    // Watchdog
    watchdog::init(TASK_WDT_TIMEOUT_S);

    // Start FreeRTOS tasks (non-returning)
    tasks::start_all();

    Serial.println("[FanCtrl] Tasks started.");
}

void loop() {
    // All work is done in FreeRTOS tasks.
    // loop() is intentionally empty.
    vTaskDelay(portMAX_DELAY);
}
