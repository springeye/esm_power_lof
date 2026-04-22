#include "fan/fan_curve.h"
#include "app_config.h"

uint16_t fan_temp_to_pwm(float temp_c) {
    // Force full speed at or above emergency threshold
    if (temp_c >= FAN_TEMP_FORCE) {
        return FAN_PWM_MAX;
    }
    // Full speed at high threshold
    if (temp_c >= FAN_TEMP_HIGH) {
        return FAN_PWM_MAX;
    }
    // Second segment: 50-70°C → 60%→100% (614→1023)
    if (temp_c > FAN_TEMP_MID) {
        float ratio = (temp_c - FAN_TEMP_MID) / (FAN_TEMP_HIGH - FAN_TEMP_MID);
        uint16_t mid_duty = 614u;  // 60% of 1023
        return static_cast<uint16_t>(mid_duty + ratio * (FAN_PWM_MAX - mid_duty));
    }
    // First segment: 30-50°C → 20%→60% (205→614)
    if (temp_c > FAN_TEMP_LOW) {
        float ratio = (temp_c - FAN_TEMP_LOW) / (FAN_TEMP_MID - FAN_TEMP_LOW);
        uint16_t mid_duty = 614u;
        return static_cast<uint16_t>(FAN_PWM_MIN + ratio * (mid_duty - FAN_PWM_MIN));
    }
    // Below 30°C: minimum duty
    return FAN_PWM_MIN;
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
