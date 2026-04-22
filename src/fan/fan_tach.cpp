#include "fan/fan_tach.h"
#include "app_config.h"
#include "pins.h"
#include <Arduino.h>
#include <driver/pcnt.h>

// PCNT unit for fan tachometer
static const pcnt_unit_t TACH_UNIT = PCNT_UNIT_0;
// Measurement window in ms
static const uint32_t TACH_WINDOW_MS = 500u;
// Pulses per revolution (4-wire fan: 2 pulses/rev)
static const uint8_t PULSES_PER_REV = 2u;

static uint32_t s_last_measure_ms = 0u;
static uint32_t s_last_rpm        = 0u;

void fan_tach_init(void) {
    pcnt_config_t cfg = {
        .pulse_gpio_num = FAN_TACH,
        .ctrl_gpio_num  = PCNT_PIN_NOT_USED,
        .lctrl_mode     = PCNT_MODE_KEEP,
        .hctrl_mode     = PCNT_MODE_KEEP,
        .pos_mode       = PCNT_COUNT_INC,   // count rising edges
        .neg_mode       = PCNT_COUNT_DIS,
        .counter_h_lim  = 32767,
        .counter_l_lim  = 0,
        .unit           = TACH_UNIT,
        .channel        = PCNT_CHANNEL_0
    };
    pcnt_unit_config(&cfg);
    pcnt_filter_disable(TACH_UNIT);
    pcnt_counter_pause(TACH_UNIT);
    pcnt_counter_clear(TACH_UNIT);
    pcnt_counter_resume(TACH_UNIT);
    s_last_measure_ms = millis();
}

uint32_t fan_tach_get_rpm(void) {
    uint32_t now = millis();
    uint32_t elapsed = now - s_last_measure_ms;

    if (elapsed < TACH_WINDOW_MS) {
        return s_last_rpm;
    }

    int16_t count = 0;
    pcnt_get_counter_value(TACH_UNIT, &count);
    pcnt_counter_clear(TACH_UNIT);
    s_last_measure_ms = now;

    if (count < 0) count = 0;

    // RPM = (pulses / pulses_per_rev) / (elapsed_ms / 60000)
    //     = pulses * 60000 / (pulses_per_rev * elapsed_ms)
    s_last_rpm = (static_cast<uint32_t>(count) * 60000u)
                 / (PULSES_PER_REV * elapsed);

    return s_last_rpm;
}
