/**
 * @file tasks.cpp
 * @brief FreeRTOS 任务实现 — 5个应用任务
 *
 * 任务调度：
 *  - lvglTask   : Core 1, 优先级 5, 5ms 周期
 *  - sensorTask : Core 0, 优先级 3, 250ms 周期（INA226 × 3 轮询）
 *  - ctrlTask   : Core 0, 优先级 3, 500ms 周期
 *  - inputTask  : Core 0, 优先级 4, 5ms 周期（按键去抖）
 *  - powerTask  : Core 0, 优先级 6, 10ms 周期
 *
 * WDT：每个任务在循环末尾调用 esp_task_wdt_reset()
 */

#include "tasks.h"
#include "watchdog.h"
#include "fault_guard.h"
#include "app_state.h"
#include "../sensors/ntc/ntc.h"
#include "../sensors/ina226/ina226.h"
#include "../fan/fan_curve.h"
#include "../fan/fan_pwm.h"
#include "../fan/fan_tach.h"
#include "../power/psu_fsm.h"
#include "../power/ps_on.h"
#include "../input/keys.h"
#include "../display/lvgl_port.h"
#include "../ui/ui_main.h"
#include "../ui/ui_events.h"
#include "app_config.h"
#include "pins.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>

namespace tasks {

// ── lvglTask ─────────────────────────────────────────────────────────────────
void lvgl_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    for (;;) {
        lvgl_port::tick_increment();
        lvgl_port::task_handler();

        // 刷新 UI 数据
        ui_main::update_temperature(app_state::get_temp_c());
        ui_main::update_fan_rpm(app_state::get_rpm());
        ui_main::update_current(app_state::get_load_a(),
                                app_state::get_v12_a(),
                                app_state::get_v5_a());

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// ── sensorTask ───────────────────────────────────────────────────────────────
void sensor_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    for (;;) {
        // NTC 温度采样（直接读 ADC）
        uint16_t adc_raw = static_cast<uint16_t>(analogRead(NTC_ADC_CH));
        float temp = ntc_adc_to_temp(adc_raw);
        app_state::set_temp_c(temp);

        // INA226 三路电流采样（轮询，每路 250ms）
        Ina226Data d;
        if (ina226_read(INA_RAIL_LOAD, &d)) {
            app_state::set_load_ma(static_cast<int32_t>(d.current_a * 1000.0f));
        }
        vTaskDelay(pdMS_TO_TICKS(250));
        if (ina226_read(INA_RAIL_12V, &d)) {
            app_state::set_v12_ma(static_cast<int32_t>(d.current_a * 1000.0f));
        }
        vTaskDelay(pdMS_TO_TICKS(250));
        if (ina226_read(INA_RAIL_5V, &d)) {
            app_state::set_v5_ma(static_cast<int32_t>(d.current_a * 1000.0f));
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

// ── ctrlTask ─────────────────────────────────────────────────────────────────
void ctrl_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    static float prev_temp = 25.0f;
    for (;;) {
        float temp = app_state::get_temp_c();
        // 对温度应用滞回，再转 PWM
        float eff_temp = hysteresis_apply(prev_temp, temp, FAN_HYSTERESIS);
        prev_temp = eff_temp;
        uint16_t duty = fan_temp_to_pwm(eff_temp);

        fan_pwm_set_duty(duty);
        app_state::set_duty(duty);

        // 转速读取
        uint32_t rpm = fan_tach_get_rpm();
        app_state::set_rpm(rpm);

        // 过温检测
        fault_guard::check_temperature(temp);
        // 堵转检测
        fault_guard::check_stall(duty, rpm);
        // 过流检测
        fault_guard::check_overcurrent(app_state::get_load_a() * 1000.0f,
                                       app_state::get_v12_a() * 1000.0f,
                                       app_state::get_v5_a() * 1000.0f);

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ── inputTask ────────────────────────────────────────────────────────────────
void input_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    static KeyState s_keys[3] = {};
    static const uint8_t key_pins[3] = {KEY_UP, KEY_ENTER, KEY_DOWN};
    for (;;) {
        uint32_t now = millis();
        for (uint8_t k = 0; k < 3; ++k) {
            bool raw = (digitalRead(key_pins[k]) == LOW);
            key_debounce_update(&s_keys[k], raw, now);
            if (s_keys[k].event != KEY_IDLE) {
                ui_events::handle_key(k, s_keys[k]);
                s_keys[k].event = KEY_IDLE;  // 消费事件
            }
        }
        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// ── powerTask ────────────────────────────────────────────────────────────────
void power_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    // 系统启动事件
    PsuState psu_st = psu_fsm_transition(PSU_OFF, EVT_BOOT);
    app_state::set_psu_state(static_cast<uint8_t>(psu_st));

    for (;;) {
        // PWOK 信号检测
        bool pwok = ps_on_pwok_read();
        if (pwok && psu_st == PSU_STARTING) {
            psu_st = psu_fsm_transition(psu_st, EVT_PWOK_HIGH);
        } else if (!pwok && psu_st == PSU_ON) {
            psu_st = psu_fsm_transition(psu_st, EVT_PWOK_LOW);
        }

        // 故障检测 → 触发 FAULT_RESET 并关闭电源
        if (app_state::is_fault()) {
            psu_st = psu_fsm_transition(psu_st, EVT_FAULT_RESET);
            ps_on_deassert();
        }

        // 更新 PSU 状态到 app_state
        app_state::set_psu_state(static_cast<uint8_t>(psu_st));

        // 更新 UI 状态字符串
        const char* state_str = "---";
        switch (psu_st) {
            case PSU_OFF:      state_str = "OFF";      break;
            case PSU_STANDBY:  state_str = "STANDBY";  break;
            case PSU_STARTING: state_str = "STARTING"; break;
            case PSU_ON:       state_str = "ON";       break;
            case PSU_STOPPING: state_str = "STOPPING"; break;
            case PSU_FAULT:    state_str = "FAULT";    break;
        }
        ui_main::update_psu_state(state_str);

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ── start_all ────────────────────────────────────────────────────────────────
void start_all() {
    xTaskCreatePinnedToCore(lvgl_task,   "lvgl",   TASK_STACK_LVGL,   nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(sensor_task, "sensor", TASK_STACK_SENSOR, nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(ctrl_task,   "ctrl",   TASK_STACK_CTRL,   nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(input_task,  "input",  TASK_STACK_INPUT,  nullptr, 4, nullptr, 0);
    xTaskCreatePinnedToCore(power_task,  "power",  TASK_STACK_POWER,  nullptr, 6, nullptr, 0);
}

} // namespace tasks
