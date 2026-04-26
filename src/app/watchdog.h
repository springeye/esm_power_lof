#pragma once
/**
 * @file watchdog.h
 * @brief 任务看门狗封装
 *
 * 封装 esp_task_wdt API，提供统一的初始化/喂狗接口。
 * 超时时间由 app_config.h 中的 TASK_WDT_TIMEOUT_S 定义（5s）。
 */

#include <cstdint>

namespace watchdog {

/**
 * @brief 初始化任务看门狗（在 setup() 中调用）
 * @param timeout_s 超时时间，单位秒（默认使用 TASK_WDT_TIMEOUT_S）
 */
void init(uint32_t timeout_s);

/**
 * @brief 喂看门狗（在各任务循环末尾调用）
 *
 * 封装 esp_task_wdt_reset()。
 */
void feed();

} // namespace watchdog
