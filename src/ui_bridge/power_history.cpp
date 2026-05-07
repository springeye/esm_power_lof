#include "power_history.h"
#include <mutex>
#include <algorithm>

namespace {

static constexpr uint32_t BUFFER_SIZE = 3000;
static constexpr uint8_t CHANNEL_COUNT = 3;

static PowerPoint s_buffer[CHANNEL_COUNT][BUFFER_SIZE];
static uint32_t s_head[CHANNEL_COUNT] = {0};
static uint32_t s_count[CHANNEL_COUNT] = {0};
static std::mutex s_history_mutex;

// 临时存储，用于返回逻辑上连续的数据段，避免跨越环形边界导致的绘图困难
// 3000 * sizeof(PowerPoint) = 3000 * 8 = 24000 bytes. 
// 3 channels = 72KB. ESP32-S3 stack is limited, so we use static.
static PowerPoint s_linear_temp[CHANNEL_COUNT][BUFFER_SIZE];

} // namespace

void power_history_init() {
    std::lock_guard<std::mutex> lock(s_history_mutex);
    for (int i = 0; i < CHANNEL_COUNT; ++i) {
        s_head[i] = 0;
        s_count[i] = 0;
    }
}

void power_history_push(uint8_t ch, uint32_t ts, float w) {
    if (ch >= CHANNEL_COUNT) return;

    std::lock_guard<std::mutex> lock(s_history_mutex);
    
    s_buffer[ch][s_head[ch]].timestamp_ms = ts;
    s_buffer[ch][s_head[ch]].power_w = w;

    s_head[ch] = (s_head[ch] + 1) % BUFFER_SIZE;
    if (s_count[ch] < BUFFER_SIZE) {
        s_count[ch]++;
    }
}

void power_history_get_range(uint8_t ch, uint32_t window_ms, uint32_t* out_count, const PowerPoint** out_data) {
    if (ch >= CHANNEL_COUNT || out_count == nullptr || out_data == nullptr) return;

    std::lock_guard<std::mutex> lock(s_history_mutex);

    if (s_count[ch] == 0) {
        *out_count = 0;
        *out_data = nullptr;
        return;
    }

    uint32_t now = 0;
    // 获取最新的点的时间戳作为“当前时间”
    uint32_t latest_idx = (s_head[ch] + BUFFER_SIZE - 1) % BUFFER_SIZE;
    now = s_buffer[ch][latest_idx].timestamp_ms;

    uint32_t start_time = (now > window_ms) ? (now - window_ms) : 0;

    uint32_t found_count = 0;
    
    // 将环形缓冲区的数据展开到线性临时缓冲区中，同时过滤时间范围
    // 从旧到新遍历
    for (uint32_t i = 0; i < s_count[ch]; ++i) {
        uint32_t idx = (s_head[ch] + BUFFER_SIZE - s_count[ch] + i) % BUFFER_SIZE;
        if (s_buffer[ch][idx].timestamp_ms >= start_time) {
            s_linear_temp[ch][found_count] = s_buffer[ch][idx];
            found_count++;
        }
    }

    *out_count = found_count;
    *out_data = s_linear_temp[ch];
}
