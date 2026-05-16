#include "power_history.h"
#include <mutex>
#include <algorithm>
#include <cmath>
#include <esp_heap_caps.h>

namespace {

static constexpr uint32_t BUFFER_SIZE = 3000;
static constexpr uint8_t CHANNEL_COUNT = 3;

// 使用 PSRAM 分配，减少内部 DRAM 压力 (72KB → 外部)
static PowerHistorySample* s_buffer[CHANNEL_COUNT] = {nullptr};
static uint32_t s_head[CHANNEL_COUNT] = {0};
static uint32_t s_count[CHANNEL_COUNT] = {0};
static std::mutex s_history_mutex;

} // namespace

void power_history_init() {
    std::lock_guard<std::mutex> lock(s_history_mutex);
    for (int i = 0; i < CHANNEL_COUNT; ++i) {
        if (s_buffer[i] == nullptr) {
            s_buffer[i] = static_cast<PowerHistorySample*>(
                heap_caps_malloc(
                    BUFFER_SIZE * sizeof(PowerHistorySample),
                    MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT));
            // 降级到内部 RAM
            if (s_buffer[i] == nullptr) {
                s_buffer[i] = static_cast<PowerHistorySample*>(
                    malloc(BUFFER_SIZE * sizeof(PowerHistorySample)));
            }
        }
        s_head[i] = 0;
        s_count[i] = 0;
    }
}

void power_history_push(uint8_t ch, uint32_t ts, float w) {
    if (ch >= CHANNEL_COUNT || s_buffer[ch] == nullptr) return;

    std::lock_guard<std::mutex> lock(s_history_mutex);

    s_buffer[ch][s_head[ch]].timestamp_ms = ts;
    s_buffer[ch][s_head[ch]].power_w = w;

    s_head[ch] = (s_head[ch] + 1) % BUFFER_SIZE;
    if (s_count[ch] < BUFFER_SIZE) {
        s_count[ch]++;
    }
}

uint32_t power_history_sample_window(
    uint8_t ch,
    uint32_t window_ms,
    uint16_t max_points,
    PowerHistorySample* out_points,
    uint16_t out_capacity) {

    if (ch >= CHANNEL_COUNT || max_points == 0 || out_capacity == 0
        || out_points == nullptr || s_buffer[ch] == nullptr) {
        return 0;
    }

    std::lock_guard<std::mutex> lock(s_history_mutex);

    if (s_count[ch] == 0) {
        return 0;
    }

    // 获取最新点的时间戳
    uint32_t latest_idx = (s_head[ch] + BUFFER_SIZE - 1) % BUFFER_SIZE;
    uint32_t now = s_buffer[ch][latest_idx].timestamp_ms;
    uint32_t start_time = (now > window_ms) ? (now - window_ms) : 0;

    // 第一次遍历：统计窗口内的点数
    uint32_t window_count = 0;
    for (uint32_t i = 0; i < s_count[ch]; ++i) {
        uint32_t idx = (s_head[ch] + BUFFER_SIZE - s_count[ch] + i) % BUFFER_SIZE;
        if (s_buffer[ch][idx].timestamp_ms >= start_time) {
            window_count++;
        }
    }

    if (window_count == 0) {
        return 0;
    }

    uint16_t output_limit = (max_points < out_capacity) ? max_points : out_capacity;

    // 如果窗口内点数 <= 输出上限，直接输出全部
    if (window_count <= output_limit) {
        uint16_t out_idx = 0;
        for (uint32_t i = 0; i < s_count[ch] && out_idx < output_limit; ++i) {
            uint32_t idx = (s_head[ch] + BUFFER_SIZE - s_count[ch] + i) % BUFFER_SIZE;
            if (s_buffer[ch][idx].timestamp_ms >= start_time) {
                out_points[out_idx].timestamp_ms = s_buffer[ch][idx].timestamp_ms;
                out_points[out_idx].power_w = s_buffer[ch][idx].power_w;
                out_idx++;
            }
        }
        return out_idx;
    }

    // 等间距抽样：始终保留窗口内最旧点与最新点
    float step = static_cast<float>(window_count - 1) / static_cast<float>(output_limit - 1);
    uint16_t out_idx = 0;
    uint32_t window_idx = 0;

    for (uint32_t i = 0; i < s_count[ch] && out_idx < output_limit; ++i) {
        uint32_t idx = (s_head[ch] + BUFFER_SIZE - s_count[ch] + i) % BUFFER_SIZE;
        if (s_buffer[ch][idx].timestamp_ms < start_time) {
            continue;
        }

        // 计算当前窗口内点对应的目标输出索引
        float target = step * static_cast<float>(out_idx);
        uint16_t target_idx = static_cast<uint16_t>(std::round(target));

        if (window_idx == target_idx || window_idx == window_count - 1) {
            out_points[out_idx].timestamp_ms = s_buffer[ch][idx].timestamp_ms;
            out_points[out_idx].power_w = s_buffer[ch][idx].power_w;
            out_idx++;
        }

        window_idx++;
    }

    // 确保最新点被采样
    if (out_idx > 0 && out_points[out_idx - 1].timestamp_ms != s_buffer[ch][latest_idx].timestamp_ms) {
        if (out_idx < output_limit) {
            out_points[out_idx].timestamp_ms = s_buffer[ch][latest_idx].timestamp_ms;
            out_points[out_idx].power_w = s_buffer[ch][latest_idx].power_w;
            out_idx++;
        } else {
            out_points[out_idx - 1].timestamp_ms = s_buffer[ch][latest_idx].timestamp_ms;
            out_points[out_idx - 1].power_w = s_buffer[ch][latest_idx].power_w;
        }
    }

    return out_idx;
}
