#pragma once
/**
 * @file tft_driver.h
 * @brief TFT_eSPI 封装层 — ST7789 240×280 SPI 显示驱动
 *
 * 职责：
 *  - 初始化 TFT_eSPI 实例（SPI 引脚由 platformio.ini build_flags 注入）
 *  - 背光 PWM 控制（LEDC 通道 1，GPIO16）
 *  - 提供 flush_cb 所需的原始像素写入接口
 *
 * 约束：
 *  - 仅在 esp32dev env 编译（不参与 native 测试）
 *  - 不在此层分配堆内存
 *  - 背光亮度 0-255，映射到 LEDC 10-bit duty
 */

#include <TFT_eSPI.h>
#include <cstdint>

namespace tft_driver {

/**
 * @brief 初始化 TFT 显示屏和背光 PWM
 *
 * 调用顺序：在 setup() 或 lvgl_port::init() 之前调用。
 * 内部执行：
 *  1. tft.init()
 *  2. tft.setRotation(0)  — 竖屏 240×280
 *  3. tft.fillScreen(TFT_BLACK)
 *  4. 背光 LEDC 初始化，默认亮度 200
 */
void init();

/**
 * @brief 设置背光亮度
 * @param brightness 0（关）~ 255（最亮）
 */
void set_backlight(uint8_t brightness);

/**
 * @brief 获取 TFT_eSPI 实例引用（供 lvgl_port 的 flush_cb 使用）
 * @return TFT_eSPI& 全局单例引用
 */
TFT_eSPI& get_tft();

/**
 * @brief 将像素数据写入显示屏指定区域
 *
 * 供 LVGL flush_cb 调用，不分配堆内存。
 * @param x1  起始列
 * @param y1  起始行
 * @param x2  结束列（含）
 * @param y2  结束行（含）
 * @param data 16 位 RGB565 像素数据指针
 */
void push_pixels(int32_t x1, int32_t y1, int32_t x2, int32_t y2,
                 const uint16_t* data);

} // namespace tft_driver
