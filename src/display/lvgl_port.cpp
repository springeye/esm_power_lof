/**
 * @file lvgl_port.cpp
 * @brief LVGL 9.x 移植实现 — 双缓冲 flush_cb
 *
 * 双缓冲尺寸：240×28 像素 × 2B = 13,440 字节/缓冲，共 26.88KB
 * 静态分配在 DRAM，不使用堆，不使用 PSRAM。
 *
 * LVGL 9.x API 变更说明：
 *  - lv_display_create() 替代 lv_disp_drv_init()
 *  - lv_display_set_buffers() 替代 lv_disp_draw_buf_init()
 *  - flush_cb 签名：void(lv_display_t*, const lv_area_t*, uint8_t*)
 */

#include "lvgl_port.h"
#include "tft_driver.h"
#include <lvgl.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

// ── 显示分辨率 ───────────────────────────────────────────────────────────────
static constexpr int32_t DISP_W = 240;
static constexpr int32_t DISP_H = 280;

// ── 双缓冲（静态 DRAM，各 240×28 像素）────────────────────────────────────
static constexpr int32_t BUF_LINES = 28;
static lv_color_t s_buf1[DISP_W * BUF_LINES];
static lv_color_t s_buf2[DISP_W * BUF_LINES];

// ── LVGL display 句柄 ────────────────────────────────────────────────────────
static lv_display_t* s_disp = nullptr;

// ── flush_cb ─────────────────────────────────────────────────────────────────
static void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
    // px_map 指向 RGB565 像素数据（LVGL 9.x 传 uint8_t*，实际内容是 uint16_t）
    const uint16_t* data = reinterpret_cast<const uint16_t*>(px_map);
    tft_driver::push_pixels(area->x1, area->y1, area->x2, area->y2, data);
    lv_display_flush_ready(disp);
}

namespace lvgl_port {

void init() {
    lv_init();

    // 创建 display 驱动
    s_disp = lv_display_create(DISP_W, DISP_H);

    // 注册双缓冲（LV_DISPLAY_RENDER_MODE_PARTIAL）
    lv_display_set_buffers(s_disp,
                           s_buf1, s_buf2,
                           sizeof(s_buf1),
                           LV_DISPLAY_RENDER_MODE_PARTIAL);

    // 注册 flush_cb
    lv_display_set_flush_cb(s_disp, flush_cb);
}

void tick_increment() {
    lv_tick_inc(5);  // 每 5ms 调用一次
}

void task_handler() {
    lv_task_handler();
}

} // namespace lvgl_port
