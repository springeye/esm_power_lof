/**
 * @file tft_driver.cpp
 * @brief TFT_eSPI 封装实现 — ST7789 240×280
 *
 * 背光使用 LEDC channel 1（channel 0 已被 fan_pwm 占用），
 * 10-bit duty，频率 5kHz，GPIO16（TFT_BL）。
 */

#include "tft_driver.h"
#include "pins.h"
#include "app_config.h"
#include <driver/ledc.h>

// ── 背光 LEDC 配置 ──────────────────────────────────────────────────────────
static constexpr ledc_channel_t BL_LEDC_CH    = LEDC_CHANNEL_1;
static constexpr ledc_timer_t   BL_LEDC_TIMER = LEDC_TIMER_1;
static constexpr uint32_t       BL_FREQ_HZ    = 5000;
static constexpr ledc_timer_bit_t BL_RES      = LEDC_TIMER_10_BIT;

// ── TFT 单例 ────────────────────────────────────────────────────────────────
static TFT_eSPI s_tft;

namespace tft_driver {

void init() {
    // 1. 初始化 TFT
    s_tft.init();
    s_tft.setRotation(0);          // 竖屏 240×280
    s_tft.fillScreen(TFT_BLACK);

    // 2. 背光 LEDC 初始化
    ledc_timer_config_t timer_cfg = {};
    timer_cfg.speed_mode      = LEDC_LOW_SPEED_MODE;
    timer_cfg.duty_resolution = BL_RES;
    timer_cfg.timer_num       = BL_LEDC_TIMER;
    timer_cfg.freq_hz         = BL_FREQ_HZ;
    timer_cfg.clk_cfg         = LEDC_AUTO_CLK;
    ledc_timer_config(&timer_cfg);

    ledc_channel_config_t ch_cfg = {};
    ch_cfg.gpio_num   = TFT_BL;
    ch_cfg.speed_mode = LEDC_LOW_SPEED_MODE;
    ch_cfg.channel    = BL_LEDC_CH;
    ch_cfg.timer_sel  = BL_LEDC_TIMER;
    ch_cfg.duty       = 0;
    ch_cfg.hpoint     = 0;
    ledc_channel_config(&ch_cfg);

    // 3. 默认亮度 200/255
    set_backlight(200);
}

void set_backlight(uint8_t brightness) {
    // brightness 0-255 → duty 0-1023 (10-bit)
    uint32_t duty = (static_cast<uint32_t>(brightness) * 1023U) / 255U;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, BL_LEDC_CH, duty);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, BL_LEDC_CH);
}

TFT_eSPI& get_tft() {
    return s_tft;
}

void push_pixels(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                 const uint16_t* data) {
    s_tft.startWrite();
    s_tft.setAddrWindow(x1, y1, x2 - x1 + 1, y2 - y1 + 1);
    // pushPixels 接受 uint16_t*，内部 DMA 传输，不分配堆内存
    s_tft.pushPixels(const_cast<uint16_t*>(data),
                     static_cast<uint32_t>((x2 - x1 + 1) * (y2 - y1 + 1)));
    s_tft.endWrite();
}

} // namespace tft_driver
