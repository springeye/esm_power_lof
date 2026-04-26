#pragma once
/**
 * @file fault_guard.h
 * @brief 故障保护 — 过温/堵转/过流检测
 *
 * 职责：
 *  - 过温检测：temp > 80°C → 设置 fault_active，触发 PSU 关机
 *  - 堵转检测：duty > 30% 且 rpm < 100 持续 3s → 判定故障
 *  - 过流检测：任一路电流 > 阈值 → 判定故障
 *
 * 约束：
 *  - 不在 ISR 中调用
 *  - 故障状态写入 app_state::fault_active
 */

#include <cstdint>

namespace fault_guard {

/**
 * @brief 检测过温
 * @param temp_c 当前温度，单位 °C
 */
void check_temperature(float temp_c);

/**
 * @brief 检测风扇堵转
 * @param duty 当前 PWM 占空比（0-1023）
 * @param rpm  当前转速（RPM）
 */
void check_stall(uint16_t duty, uint32_t rpm);

/**
 * @brief 检测过流
 * @param load_ma CH1 电流，单位 mA
 * @param v12_ma  CH2 电流，单位 mA
 * @param v5_ma   CH3 电流，单位 mA
 */
void check_overcurrent(int32_t load_ma, int32_t v12_ma, int32_t v5_ma);

/**
 * @brief 清除故障状态（手动复位）
 */
void clear();

} // namespace fault_guard
