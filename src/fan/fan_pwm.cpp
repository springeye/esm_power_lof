#include "fan/fan_pwm.h"
#include "app_config.h"
#include "pins.h"
#include <Arduino.h>
#include <driver/ledc.h>

static uint16_t s_current_duty = 0u;

void fan_pwm_init(void) {
    ledc_timer_config_t timer_cfg = {
        .speed_mode      = LEDC_LOW_SPEED_MODE,
        .duty_resolution = static_cast<ledc_timer_bit_t>(FAN_PWM_RES_BITS),
        .timer_num       = LEDC_TIMER_0,
        .freq_hz         = FAN_PWM_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t ch_cfg = {
        .gpio_num   = FAN_PWM,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel    = static_cast<ledc_channel_t>(FAN_LEDC_CH),
        .intr_type  = LEDC_INTR_DISABLE,
        .timer_sel  = LEDC_TIMER_0,
        .duty       = FAN_PWM_MIN,
        .hpoint     = 0
    };
    ledc_channel_config(&ch_cfg);
    s_current_duty = FAN_PWM_MIN;
}

void fan_pwm_set_duty(uint16_t duty) {
    if (duty > FAN_PWM_MAX) duty = FAN_PWM_MAX;
    s_current_duty = duty;
    ledc_set_duty(LEDC_LOW_SPEED_MODE,
                  static_cast<ledc_channel_t>(FAN_LEDC_CH),
                  duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE,
                     static_cast<ledc_channel_t>(FAN_LEDC_CH));
}

uint16_t fan_pwm_get_duty(void) {
    return s_current_duty;
}
