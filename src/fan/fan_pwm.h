#pragma once
#include <cstdint>

/**
 * @brief 使用 ESP32 LEDC 初始化风扇 PWM。
 *
 * 将 FAN_LEDC_CH 通道配置为 FAN_PWM_FREQ_HZ（25kHz），
 * FAN_PWM_RES_BITS（10 位）分辨率，并输出到 FAN_PWM 引脚。
 */
void fan_pwm_init(void);

/**
 * @brief 设置风扇 PWM 占空比。
 *
 * @param duty  10 位占空比（0–1023），超出范围的值会被自动夹紧。
 */
void fan_pwm_set_duty(uint16_t duty);

/**
 * @brief 获取当前风扇 PWM 占空比。
 * @return 当前占空比（0–1023）
 */
uint16_t fan_pwm_get_duty(void);
