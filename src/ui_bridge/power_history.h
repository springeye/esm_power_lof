#pragma once

#include <cstdint>

/**
 * @brief 功率数据点
 */
struct PowerPoint {
    uint32_t timestamp_ms;
    float power_w;
};

/**
 * @brief 初始化功率历史缓冲
 */
void power_history_init();

/**
 * @brief 存入一个新的采样点
 * @param ch 通道索引 (0-2)
 * @param ts 时间戳 (ms)
 * @param w 功率 (W)
 */
void power_history_push(uint8_t ch, uint32_t ts, float w);

/**
 * @brief 获取指定时间范围内的历史数据
 * @param ch 通道索引 (0-2)
 * @param window_ms 时间窗口长度 (ms)，例如 600000 表示最近 10 分钟
 * @param out_count 输出点数
 * @param out_data 输出指针，指向缓冲中的连续数据段或起始点
 * @note 由于是环形缓冲，如果请求的数据跨越了边界，可能需要特殊处理或者返回最近的连续段。
 *       在本实现中，为了简化 UI 绘图，我们将确保返回的数据在逻辑上是连续的。
 */
void power_history_get_range(uint8_t ch, uint32_t window_ms, uint32_t* out_count, const PowerPoint** out_data);
