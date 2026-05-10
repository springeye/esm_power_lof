#pragma once

#include <cstdint>

struct PowerPoint {
    uint32_t timestamp_ms;
    float power_w;
};

struct PowerHistorySample {
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
 * @brief 采样式窗口查询
 * @param ch 通道索引 (0-2)
 * @param window_ms 时间窗口长度 (ms)
 * @param max_points 输出点数上限
 * @param out_points 输出缓冲区
 * @param out_capacity 输出缓冲区容量
 * @return 实际输出点数（不超过 min(max_points, out_capacity)）
 */
uint32_t power_history_sample_window(
    uint8_t ch,
    uint32_t window_ms,
    uint16_t max_points,
    PowerHistorySample* out_points,
    uint16_t out_capacity);

void power_history_get_range(uint8_t ch, uint32_t window_ms, uint32_t* out_count, const PowerPoint** out_data);
