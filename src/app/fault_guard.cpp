/**
 * @file fault_guard.cpp
 * @brief 故障保护实现
 *
 * 过温阈值：由 config_manager 提供关机阈值
 * 堵转阈值：由 config_manager 提供 duty/rpm/timeout 参数
 * 过流阈值：负载 > 40A（INA226_MAX_CURRENT_A）
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

void check_overcurrent(int32_t load_ma, int32_t /*v12_ma*/, int32_t /*v5_ma*/) {
    // 负载电流超过 INA226_MAX_CURRENT_A（40A = 40000mA）
    if (load_ma > static_cast<int32_t>(INA226_MAX_CURRENT_A * 1000.0f)) {
        app_state::set_fault(true);
    }
}

void clear() {
    app_state::set_fault(false);
    s_stall_timing = false;
}

} // namespace fault_guard
