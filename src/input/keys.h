#pragma once
#include <cstdint>
#include <stdbool.h>

/**
 * 按键事件类型（design.md D8）
 */
typedef enum {
    KEY_IDLE = 0,   // 空闲状态，没有按键事件
    KEY_SHORT,      // 短按事件
    KEY_LONG        // 长按事件
} KeyEvent;

/**
 * 按键去抖状态。
 * 字段：
 *   stable      - 确认后的按键状态：true=按下
 *   raw         - 最近一次原始 GPIO 读数
 *   event       - 当前按键事件（KEY_IDLE/KEY_SHORT/KEY_LONG）
 *   press_time  - 按键被确认按下时的时间戳（ms）
 */
typedef struct {
    bool     stable;
    bool     raw;
    KeyEvent event;
    uint32_t press_time;
} KeyState;

/**
 * @brief 更新按键去抖状态机。
 *
 * 按 KEYS_POLL_MS（5ms）间隔调用。
 * 需要连续 3 次同电平采样才确认状态变化。
 * 松开时若按下时长 < KEYS_LONGPRESS_MS，则产生 KEY_SHORT。
 * 按住时长 >= KEYS_LONGPRESS_MS 时产生 KEY_LONG。
 *
 * @param st       指向 KeyState 的指针（跨调用保持）
 * @param raw_val  当前原始 GPIO 读数（true=按下）
 * @param now_ms   当前时间戳，单位毫秒
 */
void key_debounce_update(KeyState *st, bool raw_val, uint32_t now_ms);
