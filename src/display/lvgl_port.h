#pragma once
/**
 * @file lvgl_port.h
 * @brief LVGL 9.x 移植层 — 双缓冲 flush_cb + tick 驱动
 *
 * 职责：
 *  - 初始化 LVGL，注册 display driver 和 flush_cb
 *  - 双缓冲（各 240×28 像素，静态分配，不使用堆）
 *  - 提供 tick_increment() 供 FreeRTOS timer 每 1ms 调用
 *  - 提供 task_handler() 供 lvglTask 调用（每 5ms）
 *
 * 约束：
 *  - 仅在 esp32dev env 编译
 *  - flush_cb 内不分配堆内存
 *  - 双缓冲静态分配在 DRAM（非 PSRAM）
 */

#include <lvgl.h>

namespace lvgl_port {

/**
 * @brief 初始化 LVGL 并注册显示驱动
 *
 * 必须在 tft_driver::init() 之后调用。
 * 内部执行：
 *  1. lv_init()
 *  2. 静态分配双缓冲（各 240×28 像素）
 *  3. lv_display_create() + lv_display_set_buffers()
 *  4. lv_display_set_flush_cb() 注册 flush_cb
 */
void init();

/**
 * @brief LVGL tick 递增（每 1ms 调用一次）
 *
 * 在 FreeRTOS 软件定时器或 esp_timer 回调中调用。
 */
void tick_increment();

/**
 * @brief LVGL 任务处理（每 5ms 调用一次）
 *
 * 在 lvglTask 主循环中调用，驱动 LVGL 渲染引擎。
 */
void task_handler();

} // namespace lvgl_port
