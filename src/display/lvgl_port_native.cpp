/**
 * @file lvgl_port_native.cpp
 * @brief LVGL 9.x 移植层 — PC 原生版本
 *
 * 仅在 BUILD_NATIVE 定义时编译。
 * 使用 SDL2 tick 和 tft_driver_native 的 flush_cb。
 */

#ifdef BUILD_NATIVE

#include "lvgl_port.h"
#include "tft_driver.h"
#include <lvgl.h>
#include <SDL2/SDL.h>

// ── 显示分辨率 ───────────────────────────────────────────────────────────────
static constexpr int32_t DISP_W = 240;
static constexpr int32_t DISP_H = 280;

// ── 双缓冲（各 240×28 像素）────────────────────────────────────────────────
static constexpr int32_t BUF_LINES = 28;
static lv_color_t s_buf1[DISP_W * BUF_LINES];
static lv_color_t s_buf2[DISP_W * BUF_LINES];

// ── LVGL display 句柄 ────────────────────────────────────────────────────────
static lv_display_t* s_disp = nullptr;

// ── flush_cb ─────────────────────────────────────────────────────────────────
static void flush_cb(lv_display_t* disp, const lv_area_t* area, uint8_t* px_map) {
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
    // 使用 SDL_GetTicks() 推进 LVGL tick
    static uint32_t last_tick = 0;
    uint32_t now = SDL_GetTicks();
    if (now != last_tick) {
        lv_tick_inc(now - last_tick);
        last_tick = now;
    }
}

void task_handler() {
    lv_task_handler();
}

} // namespace lvgl_port

#endif // BUILD_NATIVE
