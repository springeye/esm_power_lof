#pragma once
#include <cstdint>

/**
 * @brief 使用 ESP32 PCNT（旧版驱动）初始化风扇测速计。
 *
 * 在 FAN_TACH 引脚上使用 driver/pcnt.h（旧版 API）。
 * 按测量窗口统计上升沿脉冲数。
 */
void fan_tach_init(void);

/**
 * @brief 获取风扇转速（RPM）。
 *
 * 读取 PCNT 计数器，并根据脉冲数与经过时间计算转速。
 * 4 线风扇：每转 2 个脉冲。
 *
 * @return 风扇转速（RPM），堵转时返回 0
 */
uint32_t fan_tach_get_rpm(void);
