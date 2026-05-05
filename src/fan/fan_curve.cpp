#include "fan_curve.h"
#include "../app/config_manager.h"

uint16_t fan_temp_to_pwm(float temp_c) {
    const float temp_low = config_manager::get_fan_temp_low();
    const float temp_mid = config_manager::get_fan_temp_mid();
    const float temp_high = config_manager::get_fan_temp_high();
    const float temp_force = config_manager::get_fan_temp_force();
    const uint16_t min_duty = static_cast<uint16_t>(config_manager::get_fan_pwm_min_percent() * 1023 / 100);
    const uint16_t mid_duty = static_cast<uint16_t>(config_manager::get_fan_pwm_mid_percent() * 1023 / 100);
    const uint16_t max_duty = 1023u;

    // Force full speed at or above emergency threshold (60°C)
    if (temp_c >= temp_force) {
        return max_duty;
    }
    // Full speed at high threshold (55°C)
    if (temp_c >= temp_high) {
        return max_duty;
    }
    // Second segment: 45-55°C → 60%→100%
    if (temp_c > temp_mid) {
        float ratio = (temp_c - temp_mid) / (temp_high - temp_mid);
        return static_cast<uint16_t>(mid_duty + ratio * (max_duty - mid_duty));
    }
    // First segment: 35-45°C → 20%→60%
    if (temp_c > temp_low) {
        float ratio = (temp_c - temp_low) / (temp_mid - temp_low);
        return static_cast<uint16_t>(min_duty + ratio * (mid_duty - min_duty));
    }
    // Below 35°C: minimum duty
    return min_duty;
}

float hysteresis_apply(float current, float target, float band) {
    if (current > target + band) {
        return current;
    }
    if (current < target - band) {
        return current;
    }
    // Within band: hold at target
    return target;
}
