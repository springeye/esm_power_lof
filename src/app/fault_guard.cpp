/**
 * @file fault_guard.cpp
 * @brief 故障保护实现
 *
 * 过温阈值：80°C（硬编码，超出 app_config.h 温控范围的安全边界）
 * 堵转阈值：duty > FAN_STALL_DUTY_THRESH(307) 且 rpm < 100，持续 FAN_STALL_TIMEOUT_MS(3000ms)
 * 过流阈值：负载 > 40A（INA226_MAX_CURRENT_A）
 */

#include "fault_guard.h"
#include "app_state.h"
#include "app_config.h"
#include <Arduino.h>

static constexpr float    OVER_TEMP_C      = 80.0f;
static constexpr uint32_t STALL_RPM_THRESH = 100;

// 堵转计时
static uint32_t s_stall_start_ms = 0;
static bool     s_stall_timing   = false;

namespace fault_guard {

void check_temperature(float temp_c) {
    if (temp_c >= OVER_TEMP_C) {
        app_state::set_fault(true);
    }
}

void check_stall(uint16_t duty, uint32_t rpm) {
    if (duty > FAN_STALL_DUTY_THRESH && rpm < STALL_RPM_THRESH) {
        if (!s_stall_timing) {
            s_stall_timing   = true;
            s_stall_start_ms = millis();
        } else if (millis() - s_stall_start_ms >= FAN_STALL_TIMEOUT_MS) {
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
