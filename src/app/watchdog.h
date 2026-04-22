#pragma once
/**
 * @file watchdog.h
 * @brief Task WDT 封装
 *
 * 封装 esp_task_wdt API，提供统一的 init/feed 接口。
 * 超时时间由 app_config.h TASK_WDT_TIMEOUT_S 定义（5s）。
 */

#include <cstdint>

namespace watchdog {

/**
 * @brief 初始化 Task WDT（在 setup() 中调用）
 * @param timeout_s 超时秒数（默认使用 TASK_WDT_TIMEOUT_S）
 */
void init(uint32_t timeout_s);

/**
 * @brief 喂狗（在各任务循环末尾调用）
 *
 * 封装 esp_task_wdt_reset()。
 */
void feed();

} // namespace watchdog
