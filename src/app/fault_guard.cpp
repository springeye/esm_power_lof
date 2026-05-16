/**
 * @file fault_guard.cpp
 * @brief 故障保护实现
 *
 * 过温阈值：由 config_manager 提供关机阈值
 * 堵转阈值：由 config_manager 提供 duty/rpm/timeout 参数
 * 过流阈值：三路独立检测，任一路 > INA226_MAX_CURRENT_A 触发故障
 */

#include "fault_guard.h"
#include "config_manager.h"
#include "app_state.h"
#include "app_config.h"
#include <Arduino.h>

// 堵转计时
static uint32_t s_stall_start_ms = 0;
static bool     s_stall_timing   = false;

namespace fault_guard {

void check_temperature(float temp_c) {
    if (temp_c >= config_manager::get_temp_shutdown_threshold()) {
        app_state::set_fault(true);
    }
}

void check_stall(uint16_t duty, uint32_t rpm) {
    if (duty > config_manager::get_fan_stall_duty_thresh()
        && rpm < config_manager::get_fan_stall_rpm_thresh()) {
        if (!s_stall_timing) {
            s_stall_timing   = true;
            s_stall_start_ms = millis();
        } else if (millis() - s_stall_start_ms >= config_manager::get_fan_stall_timeout_ms()) {
            app_state::set_fault(true);
        }
    } else {
        s_stall_timing = false;
    }
}

void check_overcurrent(int32_t ch1_ma, int32_t ch2_ma, int32_t ch3_ma) {
    const int32_t threshold = static_cast<int32_t>(INA226_MAX_CURRENT_A * 1000.0f);
    if (ch1_ma > threshold || ch2_ma > threshold || ch3_ma > threshold) {
        app_state::set_fault(true);
    }
}

void clear() {
    app_state::set_fault(false);
    s_stall_timing = false;
}

} // namespace fault_guard
