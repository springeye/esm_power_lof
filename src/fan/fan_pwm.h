#pragma once
#include <cstdint>

/**
 * @brief Initialize fan PWM using ESP32 LEDC.
 *
 * Configures LEDC channel FAN_LEDC_CH at FAN_PWM_FREQ_HZ (25kHz),
 * FAN_PWM_RES_BITS (10-bit), on FAN_PWM pin.
 */
void fan_pwm_init(void);

/**
 * @brief Set fan PWM duty cycle.
 *
 * @param duty  10-bit duty (0–1023). Values outside range are clamped.
 */
void fan_pwm_set_duty(uint16_t duty);

/**
 * @brief Get current fan PWM duty cycle.
 * @return Current duty (0–1023)
 */
uint16_t fan_pwm_get_duty(void);
