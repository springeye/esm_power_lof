#pragma once
/**
 * @file tft_demo.h
 * @brief ST7789 屏幕底层测试图案（不依赖 LVGL）
 *
 * 用途：
 *  - 验证 TFT 接线（VCC/GND/CS/DC/RST/MOSI/SCLK/BL）
 *  - 验证 SPI 引脚映射（platformio.ini 中 -DTFT_xxx 宏）
 *  - 验证背光 LEDC 配置
 *  - 排除 LVGL/UI 层问题，缩小故障范围
 *
 * 显示内容（顺序循环）：
 *  1. 全红 → 全绿 → 全蓝 → 全白（验证显存写入和 RGB 通道）
 *  2. 三色横向彩条（验证地址窗口）
 *  3. 文字 "ST7789 OK" + 计数器（验证字体渲染）
 *  4. 中心闪烁方块（验证刷新节奏）
 *
 * 调用约束：
 *  - 调用前必须先执行 tft_driver::init()
 *  - run() 为阻塞函数，内部 vTaskDelay，永不返回
 */

namespace tft_demo {

/**
 * @brief 启动测试图案循环（永不返回）
 *
 * 在屏幕上循环显示测试图案，每帧间隔约 1 秒。
 * 串口同步打印当前帧号，便于排查屏幕黑屏 vs 程序卡死。
 */
[[noreturn]] void run();

} // namespace tft_demo
