#pragma once
/**
 * @file ui_events.h
 * @brief 按键事件路由 — 将 keys 模块事件分发到 UI 层
 *
 * 职责：
 *  - 接收 KeyState 事件，根据当前活跃屏幕路由到对应 UI 模块
 *  - 管理主仪表盘 ↔ 设置菜单的屏幕切换
 *
 * 约束：
 *  - 仅在 esp32dev env 编译
 *  - 不在 ISR 中调用
 */

#include "input/keys.h"

namespace ui_events {

/**
 * @brief 初始化事件路由（设置初始活跃屏幕为主仪表盘）
 */
void init();

/**
 * @brief 处理按键事件
 *
 * 由 inputTask 在检测到按键状态变化时调用。
 * @param key   按键 ID（KEY_K1 / KEY_K2 / KEY_K3）
 * @param state 按键状态（KEY_SHORT / KEY_LONG）
 */
void handle_key(uint8_t key, KeyState state);

} // namespace ui_events
