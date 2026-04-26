/**
 * @file tft_demo.cpp
 * @brief ST7789 屏幕底层测试图案实现
 *
 * 仅依赖 TFT_eSPI 与 Arduino，不接入 LVGL/UI/任务系统。
 */

#include "tft_demo.h"
#include "tft_driver.h"
#include "pins.h"

#include <Arduino.h>
#include <TFT_eSPI.h>

namespace tft_demo {

namespace {

// 屏幕逻辑尺寸（与 platformio.ini -DTFT_WIDTH/-DTFT_HEIGHT 一致）
constexpr int16_t W = 240;
constexpr int16_t H = 280;

// 单帧间隔
constexpr uint32_t FRAME_DELAY_MS = 1000;

void fill_solid(TFT_eSPI& tft, uint16_t color, const char* tag) {
    tft.fillScreen(color);
    tft.setTextColor(TFT_WHITE, color);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(3);
    tft.drawString(tag, W / 2, H / 2);
    Serial.printf("[tft_demo] solid %s\n", tag);
}

void color_bars(TFT_eSPI& tft) {
    // 6 段彩条：红 / 绿 / 蓝 / 黄 / 青 / 品红
    static const uint16_t bars[6] = {
        TFT_RED, TFT_GREEN, TFT_BLUE,
        TFT_YELLOW, TFT_CYAN, TFT_MAGENTA
    };
    const int16_t band_h = H / 6;
    for (int i = 0; i < 6; ++i) {
        tft.fillRect(0, i * band_h, W, band_h, bars[i]);
    }
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString("COLOR BARS", W / 2, H - 16);
    Serial.println("[tft_demo] color bars");
}

void text_screen(TFT_eSPI& tft, uint32_t counter) {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_GREEN, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);

    tft.setTextSize(3);
    tft.drawString("ST7789 OK", W / 2, H / 2 - 30);

    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.drawString("240 x 280", W / 2, H / 2 + 10);

    char buf[24];
    snprintf(buf, sizeof(buf), "frame %lu", static_cast<unsigned long>(counter));
    tft.drawString(buf, W / 2, H / 2 + 40);

    Serial.printf("[tft_demo] text frame=%lu\n", static_cast<unsigned long>(counter));
}

void blink_box(TFT_eSPI& tft, bool on) {
    tft.fillScreen(TFT_BLACK);
    const int16_t box = 80;
    const uint16_t color = on ? TFT_WHITE : TFT_DARKGREY;
    tft.fillRect((W - box) / 2, (H - box) / 2, box, box, color);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString(on ? "ON" : "OFF", W / 2, H - 24);
    Serial.printf("[tft_demo] blink %s\n", on ? "ON" : "OFF");
}

} // namespace

[[noreturn]] void run() {
    TFT_eSPI& tft = tft_driver::get_tft();
    tft.setRotation(0);
    tft.setTextDatum(MC_DATUM);

    Serial.println("[tft_demo] start (USE_DISPLAY_DEMO=true)");
    Serial.printf("[tft_demo] pins: MOSI=%d SCLK=%d CS=%d DC=%d RST=%d BL=%d\n",
                  TFT_MOSI, TFT_SCLK, TFT_CS, TFT_DC, TFT_RST, TFT_BL);

    uint32_t frame = 0;
    bool blink = false;

    while (true) {
        switch (frame % 8) {
            case 0: fill_solid(tft, TFT_RED,   "RED");   break;
            case 1: fill_solid(tft, TFT_GREEN, "GREEN"); break;
            case 2: fill_solid(tft, TFT_BLUE,  "BLUE");  break;
            case 3: fill_solid(tft, TFT_WHITE, "WHITE"); break;
            case 4: color_bars(tft);                     break;
            case 5: text_screen(tft, frame);             break;
            case 6: blink_box(tft, blink); blink = !blink; break;
            case 7: blink_box(tft, blink); blink = !blink; break;
        }
        ++frame;
        vTaskDelay(pdMS_TO_TICKS(FRAME_DELAY_MS));
    }
}

} // namespace tft_demo
