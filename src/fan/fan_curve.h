#pragma once
#include <cstdint>

/**
 * @brief 将温度转换为风扇 PWM 占空比（10 位，0-1023）。
 *
 * 分段线性曲线（参数从 config_manager 动态获取）：
 *   T < temp_low   → pwm_min_percent
 *   temp_low-mid   → 线性 pwm_min→pwm_mid
 *   temp_mid-high  → 线性 pwm_mid→100%
 *   T ≥ temp_high  → 100%
 *   T ≥ temp_force → 强制 100%（紧急状态）
 *
 * @param temp_c  温度值，单位 °C
 * @return PWM 占空比（0–1023）
 */
uint16_t fan_temp_to_pwm(float temp_c);

/**
 * @brief 对温度读数应用滞回处理。
 *
 * 如果新值 (target) 与旧有效值 (current) 的差异在 band 范围内，
 * 保持旧值（过滤温度抖动）。
 * 如果差异超出 band 范围，则跟随新值（响应真实温度变化）。
 *
 * @param current  上一次输出的有效温度（滞回参考点）
 * @param target   当前原始温度读数
 * @param band     滞回带宽（±），值在此范围内视为抖动
 * @return 滞回处理后的有效温度
 */
float hysteresis_apply(float current, float target, float band);
