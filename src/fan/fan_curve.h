#pragma once
#include <cstdint>

/**
 * @brief 将温度转换为风扇 PWM 占空比（10 位，0-1023）。
 *
 * 分段线性曲线：
 *   T < 30°C  → 20%（205）
 *   30-50°C   → 线性 20%→60%（205→614）
 *   50-70°C   → 线性 60%→100%（614→1023）
 *   T ≥ 70°C  → 100%（1023）
 *   T ≥ 75°C  → 强制 100%（紧急状态）
 *
 * @param temp_c  温度值，单位 °C
 * @return PWM 占空比（0–1023）
 */
uint16_t fan_temp_to_pwm(float temp_c);

/**
 * @brief 对温度读数应用滞回处理。
 *
 * 如果当前值处于 [target-band, target+band] 之间，则返回 target。
 * 否则返回 current（继续跟随实际温度）。
 *
 * @param current  当前温度读数
 * @param target   目标/参考温度
 * @param band     滞回带宽（±）
 * @return 滞回处理后的有效温度
 */
float hysteresis_apply(float current, float target, float band);
