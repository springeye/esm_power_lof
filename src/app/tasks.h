#pragma once
/**
 * @file tasks.h
 * @brief FreeRTOS 任务声明 — 5 个应用任务
 *
 * 任务列表：
 *  - lvglTask   : LVGL 渲染 + tick（Core 1，优先级 5，栈 8192）
 *  - sensorTask : NTC + INA226 采样（Core 0，优先级 3，栈 4096）
 *  - ctrlTask   : 温控曲线 + 风扇 PWM（Core 0，优先级 3，栈 4096）
 *  - inputTask  : 按键去抖 + 事件路由（Core 0，优先级 4，栈 3072）
 *  - powerTask  : PSU 状态机 + 故障保护（Core 0，优先级 6，栈 3072）
 *
 * 约束：
 *  - 仅在 esp32dev env 编译
 *  - 所有任务函数签名符合 FreeRTOS pvTaskCode 规范
 */

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

namespace tasks {

/**
 * @brief 创建并启动全部 FreeRTOS 任务
 *
 * 在 main.cpp 的 setup() 末尾调用，且需确保所有模块初始化完成。
 */
void start_all();

// ── 任务函数（供内部使用，不对外暴露实现细节）──────────────────────────────
void lvgl_task(void* param);
void sensor_task(void* param);
void ctrl_task(void* param);
void input_task(void* param);
void power_task(void* param);

} // namespace tasks
