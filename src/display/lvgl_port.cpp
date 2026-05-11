/**
 * @file lvgl_port.cpp
 * @brief LVGL 9.x 移植 — 双缓冲 PARTIAL 模式
 *
 * 缓冲：240×28 像素 × 2 个，共 26.88KB DRAM
 * SPI：60MHz（platformio.ini 配置）
 */

#include "lvgl_port.h"
#include "tft_driver.h"
#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ── 显示分辨率 ─────────────────────────────────────────────────────────────
static constexpr int32_t DISP_W = 240;
static constexpr int32_t DISP_H = 280;
static constexpr int32_t BUF_LINES = 28;
static lv_color_t s_buf1[DISP_W * BUF_LINES];
static lv_color_t s_buf2[DISP_W * BUF_LINES];

static lv_display_t* s_disp = nullptr;

static void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    const uint16_t* data = reinterpret_cast<const uint16_t*>(px_map);
    tft_driver::push_pixels(area->x1, area->y1, area->x2, area->y2, data);
    lv_display_flush_ready(disp);
}

namespace lvgl_port {

void init() {
    lv_init();

    s_disp = lv_display_create(DISP_W, DISP_H);
    lv_display_set_buffers(s_disp,
                            s_buf1, s_buf2,
                            sizeof(s_buf1),
                            LV_DISPLAY_RENDER_MODE_PARTIAL);
    lv_display_set_flush_cb(s_disp, flush_cb);

#if LV_USE_SYSMON && LV_USE_PERF_MONITOR
    lv_sysmon_show_performance(s_disp);
#endif
}

void tick_increment() {
    lv_tick_inc(5);  // 每 5ms 调用一次
}

void task_handler() {
    lv_task_handler();
}

} // namespace lvgl_port
