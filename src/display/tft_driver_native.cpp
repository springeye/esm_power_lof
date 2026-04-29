/**
 * @file tft_driver_native.cpp
 * @brief SDL2 显示后端 — 用于 PC 原生 LVGL 模拟器
 *
 * 仅在 BUILD_NATIVE 定义时编译。
 * 使用 SDL2 窗口模拟 ST7789 240×280 显示屏。
 */

#ifdef BUILD_NATIVE

#include "tft_driver.h"
#include <SDL2/SDL.h>
#include <cstdio>
#include <cstdint>

// ── 显示分辨率 ───────────────────────────────────────────────────────────────
static constexpr int32_t DISP_W = 240;
static constexpr int32_t DISP_H = 280;
static constexpr int32_t SCALE  = 2;  // 窗口缩放倍数

// ── SDL2 全局对象 ────────────────────────────────────────────────────────────
static SDL_Window*   s_window   = nullptr;
static SDL_Renderer* s_renderer = nullptr;
static SDL_Texture*  s_texture  = nullptr;

// ── RGB565 → RGB888 转换 ─────────────────────────────────────────────────────
static inline void rgb565_to_rgb888(uint16_t pixel, uint8_t& r, uint8_t& g, uint8_t& b) {
    r = (pixel >> 11) & 0x1F;
    g = (pixel >> 5)  & 0x3F;
    b = pixel & 0x1F;
    r = (r << 3) | (r >> 2);
    g = (g << 2) | (g >> 4);
    b = (b << 3) | (b >> 2);
}

namespace tft_driver {

void init() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
        return;
    }

    s_window = SDL_CreateWindow(
        "LVGL Simulator — 240×280",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        DISP_W * SCALE, DISP_H * SCALE,
        SDL_WINDOW_SHOWN
    );
    if (!s_window) {
        fprintf(stderr, "SDL_CreateWindow failed: %s\n", SDL_GetError());
        return;
    }

    s_renderer = SDL_CreateRenderer(s_window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!s_renderer) {
        fprintf(stderr, "SDL_CreateRenderer failed: %s\n", SDL_GetError());
        return;
    }

    s_texture = SDL_CreateTexture(s_renderer,
        SDL_PIXELFORMAT_RGB888,
        SDL_TEXTUREACCESS_STREAMING,
        DISP_W, DISP_H);
    if (!s_texture) {
        fprintf(stderr, "SDL_CreateTexture failed: %s\n", SDL_GetError());
        return;
    }

    // 清屏为黑色
    SDL_SetRenderDrawColor(s_renderer, 0, 0, 0, 255);
    SDL_RenderClear(s_renderer);
    SDL_RenderPresent(s_renderer);

    printf("[Native] SDL2 display initialized: %dx%d (scale %dx)\n", DISP_W, DISP_H, SCALE);
}

void set_backlight(uint8_t brightness) {
    // PC 无背光控制，忽略
    (void)brightness;
}

// get_tft() 不需要实现 — native 构建不使用 TFT_eSPI
// 提供一个 dummy 实现避免链接错误
// 注意：TFT_eSPI 在 native 构建中不可用，此函数不应被调用
// 如果链接器报错，说明有代码路径错误地调用了 get_tft()

void push_pixels(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                 const uint16_t* data) {
    if (!s_texture || !s_renderer) return;

    // 锁定纹理，写入像素数据
    void* pixels;
    int pitch;
    if (SDL_LockTexture(s_texture, nullptr, &pixels, &pitch) < 0) {
        fprintf(stderr, "SDL_LockTexture failed: %s\n", SDL_GetError());
        return;
    }

    // 将 RGB565 转换为 RGB888 并写入纹理
    uint32_t* dst = static_cast<uint32_t*>(pixels);
    int32_t w = x2 - x1 + 1;
    int32_t h = y2 - y1 + 1;

    for (int32_t y = 0; y < h; y++) {
        for (int32_t x = 0; x < w; x++) {
            uint16_t pixel = data[y * w + x];
            uint8_t r, g, b;
            rgb565_to_rgb888(pixel, r, g, b);

            int32_t dst_x = x1 + x;
            int32_t dst_y = y1 + y;
            dst[dst_y * (pitch / 4) + dst_x] = (r << 16) | (g << 8) | b;
        }
    }

    SDL_UnlockTexture(s_texture);

    // 渲染到窗口
    SDL_RenderClear(s_renderer);
    SDL_RenderCopy(s_renderer, s_texture, nullptr, nullptr);
    SDL_RenderPresent(s_renderer);
}

} // namespace tft_driver

// ── SDL 事件处理（供 native_main_sim.cpp 调用）───────────────────────────────
extern "C" bool native_sdl_poll_quit() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        if (event.type == SDL_QUIT) return true;
        if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE) return true;
    }
    return false;
}

extern "C" void native_sdl_cleanup() {
    if (s_texture)  { SDL_DestroyTexture(s_texture);  s_texture  = nullptr; }
    if (s_renderer) { SDL_DestroyRenderer(s_renderer); s_renderer = nullptr; }
    if (s_window)   { SDL_DestroyWindow(s_window);     s_window   = nullptr; }
    SDL_Quit();
}

#endif // BUILD_NATIVE
