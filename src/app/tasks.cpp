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
#include "../ui_bridge/screen_manager.h"
#include "../ui_bridge/data_bridge.h"
#include "../ui_bridge/input_bridge.h"
extern "C" {
#include "../../ui/lof_power_system.h"
}
#include "../wifi/wifi_manager.h"
#include "../web/web_server.h"
#include "config_manager.h"
#include "app_config.h"
#include "pins.h"
#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_task_wdt.h>
#include <algorithm>

namespace tasks {

// ── lvglTask ─────────────────────────────────────────────────────────────────
void lvgl_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    static bool initialized = false;
    for (;;) {
        if (!initialized) {
            lvgl_port::init();
            lof_power_system_init(NULL);
            ui_bridge::screen_manager_init(1500);
            ui_bridge::data_bridge_attach(ui_bridge::screen_manager_get_home());
            ui_bridge::data_bridge_init();
            ui_bridge::input_bridge_attach_home(ui_bridge::screen_manager_get_home());
            initialized = true;
        }

        lvgl_port::tick_increment();
        lvgl_port::task_handler();

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

// ── sensorTask ───────────────────────────────────────────────────────────────
void sensor_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    for (;;) {
        // NTC 温度采样 — 中值滤波
        {
            uint16_t samples[NTC_SAMPLES];
            for (uint8_t i = 0; i < NTC_SAMPLES; ++i) {
                samples[i] = static_cast<uint16_t>(analogRead(NTC_ADC_CH));
                vTaskDelay(pdMS_TO_TICKS(1));
            }
            std::sort(samples, samples + NTC_SAMPLES);
            uint16_t adc_median = samples[NTC_SAMPLES / 2];
            float temp = ntc_adc_to_temp(adc_median);
            app_state::set_temp_c(temp);
        }

        // INA226 三路电流/电压采样（轮询，每路 250ms）
        Ina226Data d;
        if (ina226_read(INA_CH1, &d)) {
            app_state::set_ch1_ma(static_cast<int32_t>(d.current_a * 1000.0f));
            app_state::set_ch1_mv(static_cast<uint16_t>(d.voltage_v * 1000.0f));
        }
        vTaskDelay(pdMS_TO_TICKS(250));
        if (ina226_read(INA_CH2, &d)) {
            app_state::set_ch2_ma(static_cast<int32_t>(d.current_a * 1000.0f));
            app_state::set_ch2_mv(static_cast<uint16_t>(d.voltage_v * 1000.0f));
        }
        vTaskDelay(pdMS_TO_TICKS(250));
        if (ina226_read(INA_CH3, &d)) {
            app_state::set_ch3_ma(static_cast<int32_t>(d.current_a * 1000.0f));
            app_state::set_ch3_mv(static_cast<uint16_t>(d.voltage_v * 1000.0f));
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(250));
    }
}

// ── ctrlTask ─────────────────────────────────────────────────────────────────
void ctrl_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    static float prev_temp = 0.0f;
    static bool  prev_valid = false;
    static uint32_t last_log_ms = 0;
    for (;;) {
        float temp = app_state::get_temp_c();
        // 首次迭代直接初始化为实际温度，避免热启动时 25°C 默认值偏差
        if (!prev_valid) {
            prev_temp = temp;
            prev_valid = true;
        }
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
        // 过流检测（三路独立）
        fault_guard::check_overcurrent(app_state::get_ch1_a() * 1000.0f,
                                       app_state::get_ch2_a() * 1000.0f,
                                       app_state::get_ch3_a() * 1000.0f);

        // ─ 1s 心跳日志（节流，便于串口监控运行状态）─
        uint32_t now_ms = millis();
        if (now_ms - last_log_ms >= 1000u) {
            last_log_ms = now_ms;
            float v1 = app_state::get_ch1_mv() / 1000.0f;
            float v2 = app_state::get_ch2_mv() / 1000.0f;
            float v3 = app_state::get_ch3_mv() / 1000.0f;
            float a1 = app_state::get_ch1_a();
            float a2 = app_state::get_ch2_a();
            float a3 = app_state::get_ch3_a();
            Serial.printf(
                "[HB] up=%lus T=%.1fC fan=%lurpm duty=%u(%u%%) "
                "CH1=%.2fV/%.2fA/%.1fW CH2=%.2fV/%.2fA/%.1fW "
                "CH3=%.2fV/%.2fA/%.1fW psu=%u fault=%u\n",
                static_cast<unsigned long>(now_ms / 1000u),
                temp,
                static_cast<unsigned long>(rpm),
                static_cast<unsigned>(duty),
                static_cast<unsigned>(static_cast<uint32_t>(duty) * 100u / FAN_PWM_MAX),
                v1, a1, v1 * a1,
                v2, a2, v2 * a2,
                v3, a3, v3 * a3,
                static_cast<unsigned>(app_state::psu_state_id.load()),
                static_cast<unsigned>(app_state::is_fault() ? 1u : 0u));
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}

// ── inputTask ────────────────────────────────────────────────────────────────
void input_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    static KeyState s_keys[3] = {};
    static const uint8_t key_pins[3] = {KEY_K1, KEY_K2, KEY_K3};
    for (;;) {
        uint32_t now = millis();
        for (uint8_t k = 0; k < 3; ++k) {
            bool raw = (digitalRead(key_pins[k]) == LOW);
            key_debounce_update(&s_keys[k], raw, now);
            if (s_keys[k].event != KEY_IDLE) {
                ui_bridge::input_handle_key(k, s_keys[k]);
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

    // 启动超时计时器
    uint32_t starting_enter_ms = 0;
    bool starting_armed = false;

    for (;;) {
        uint32_t now = millis();

        // ─ 跨任务 PSU 事件处理 ─
        uint8_t req = app_state::psu_event_request.exchange(0);
        if (req != 0) {
            PsuFsmEvent evt = static_cast<PsuFsmEvent>(req);
            psu_st = psu_fsm_transition(psu_st, evt);

            // 执行状态进入/退出操作
            if (evt == EVT_KEY_SHORT && psu_st == PSU_STARTING) {
                ps_on_assert();
                starting_enter_ms = now;
                starting_armed = true;
            } else if (evt == EVT_KEY_LONG) {
                ps_on_deassert();
            }
        }

        // ─ 启动超时：PSU_STARTING 超过 1s 触发 EVT_TIMEOUT_1S ─
        if (starting_armed && psu_st == PSU_STARTING) {
            if (now - starting_enter_ms >= 1000) {
                psu_st = psu_fsm_transition(psu_st, EVT_TIMEOUT_1S);
                starting_armed = false;
            }
        } else if (psu_st != PSU_STARTING) {
            starting_armed = false;
        }

        // ─ PWOK 信号检测 ─
        bool pwok = ps_on_pwok_read();
        if (pwok && psu_st == PSU_STARTING) {
            psu_st = psu_fsm_transition(psu_st, EVT_PWOK_HIGH);
            starting_armed = false;
        } else if (!pwok && psu_st == PSU_ON) {
            psu_st = psu_fsm_transition(psu_st, EVT_PWOK_LOW);
        }

        // ─ 故障检测 → 仅在状态首次转移时关闭电源，避免每 10ms 重复写 GPIO ─
        if (app_state::is_fault()) {
            PsuState new_st = psu_fsm_transition(psu_st, EVT_FAULT_RESET);
            if (new_st != psu_st) {
                psu_st = new_st;
                ps_on_deassert();
            }
        }

        // 更新 PSU 状态到 app_state
        app_state::set_psu_state(static_cast<uint8_t>(psu_st));

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// ── webTask ──────────────────────────────────────────────────────────────────
void web_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);

    // 启动时恢复上次 Web 管理状态
    if (config_manager::get_web_mgmt_enabled()) {
        wifi_mgr::start_ap();
        web_server::start();
    }

    for (;;) {
        bool web_enabled = config_manager::get_web_mgmt_enabled();
        wifi_mgr::WifiState wifi_st = wifi_mgr::get_state();

        // 同步开关状态与 WiFi/Web 服务器状态
        if (web_enabled && wifi_st == wifi_mgr::WifiState::OFF) {
            wifi_mgr::start_ap();
            web_server::start();
        } else if (!web_enabled && wifi_st != wifi_mgr::WifiState::OFF) {
            web_server::stop();
            wifi_mgr::stop();
        }

        // OTA 完成后延时 3 秒重启
        int8_t ota_prog = app_state::get_ota_progress();
        if (ota_prog == 100) {
            vTaskDelay(pdMS_TO_TICKS(3000));
            ESP.restart();
        }

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(WEB_TASK_PERIOD_MS));
    }
}

// ── start_all ────────────────────────────────────────────────────────────────
void start_all() {
    wifi_mgr::init();

    xTaskCreatePinnedToCore(lvgl_task,   "lvgl",   TASK_STACK_LVGL,   nullptr, 5, nullptr, 1);
    xTaskCreatePinnedToCore(sensor_task, "sensor", TASK_STACK_SENSOR, nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(ctrl_task,   "ctrl",   TASK_STACK_CTRL,   nullptr, 3, nullptr, 0);
    xTaskCreatePinnedToCore(input_task,  "input",  TASK_STACK_INPUT,  nullptr, 4, nullptr, 0);
    xTaskCreatePinnedToCore(power_task,  "power",  TASK_STACK_POWER,  nullptr, 6, nullptr, 0);
    xTaskCreatePinnedToCore(web_task,    "web",    WEB_TASK_STACK,    nullptr, 2, nullptr, 0);
}

} // namespace tasks
