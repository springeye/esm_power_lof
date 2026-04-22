#pragma once
#include <cstdint>
#include <stdbool.h>

/**
 * Key event types (design.md D8)
 */
typedef enum {
    KEY_IDLE = 0,
    KEY_SHORT,
    KEY_LONG
} KeyEvent;

/**
 * Key debounce state.
 * Fields:
 *   stable      - confirmed (debounced) key state: true=pressed
 *   raw         - last raw GPIO reading
 *   event       - current key event (KEY_IDLE/KEY_SHORT/KEY_LONG)
 *   press_time  - timestamp (ms) when key was confirmed pressed
 */
typedef struct {
    bool     stable;
    bool     raw;
    KeyEvent event;
    uint32_t press_time;
} KeyState;

/**
 * @brief Update key debounce state machine.
 *
 * Call at KEYS_POLL_MS (5ms) intervals.
 * Requires 3 consecutive same-level samples to confirm state change.
 * Generates KEY_SHORT on release after < KEYS_LONGPRESS_MS.
 * Generates KEY_LONG when held >= KEYS_LONGPRESS_MS.
 *
 * @param st       Pointer to KeyState (persistent across calls)
 * @param raw_val  Current raw GPIO reading (true=pressed)
 * @param now_ms   Current timestamp in milliseconds
 */
void key_debounce_update(KeyState *st, bool raw_val, uint32_t now_ms);
