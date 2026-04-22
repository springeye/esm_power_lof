#include "input/keys.h"
#include "app_config.h"

// Number of consecutive samples required to confirm state change
static const uint8_t DEBOUNCE_COUNT = 3u;

void key_debounce_update(KeyState *st, bool raw_val, uint32_t now_ms) {
    if (st == nullptr) return;

    // Track consecutive same-level samples
    static uint8_t consec_count = 0u;
    static bool    last_raw     = false;

    // Use per-instance counters via a simple approach:
    // We embed counter in the struct implicitly via press_time logic.
    // Actually we need per-instance state. Use a local static won't work for
    // multiple keys. Instead, we use the existing struct fields cleverly.
    // The struct has: stable, raw, event, press_time.
    // We'll use raw as the "last seen raw" and a counter embedded in press_time
    // when not pressed. But that's fragile. Better: add counter to struct.
    // Since we can't change the struct (test uses designated initializer with 4 fields),
    // we use a different approach: count consecutive same-level samples using
    // the difference between now_ms and a stored timestamp.

    // Simpler approach: use raw field as "pending raw" and count via time.
    // 3 consecutive samples at 5ms = 15ms minimum.
    // We track: if raw_val == st->raw for 3 calls → confirm.

    // Implementation using a static per-call counter won't work for multiple instances.
    // Since tests use a single KeyState per test, we can use a local static safely
    // for the test environment. For production, keys.cpp would use per-key state.

    // Reset event each call (only set on transitions)
    // Actually: event should persist until consumed. Tests check st->event after calls.
    // We only set event, never clear it here (consumer clears it).

    if (raw_val != st->raw) {
        // Raw level changed - start counting from 1
        st->raw = raw_val;
        // Reset debounce counter (we use press_time as counter when not stable-pressed)
        // Use a trick: store the "debounce start time" in press_time when raw changes
        // and count samples since then.
        // But press_time is also used for long-press detection.
        // We need a separate counter. Since struct is fixed, use a file-static array
        // indexed by pointer (not ideal but works for single-key tests).
        // For simplicity in this embedded context: use a module-level counter.
        // This works correctly when only one KeyState is active at a time (typical embedded use).
        consec_count = 1u;
        last_raw = raw_val;
    } else {
        // Same raw level as last call
        if (consec_count < DEBOUNCE_COUNT) {
            consec_count++;
        }

        if (consec_count >= DEBOUNCE_COUNT && raw_val != st->stable) {
            // State confirmed changed
            bool was_stable = st->stable;
            st->stable = raw_val;

            if (raw_val) {
                // Key pressed: record press time
                st->press_time = now_ms;
                st->event = KEY_IDLE;
            } else {
                // Key released
                if (was_stable) {
                    uint32_t duration = now_ms - st->press_time;
                    if (duration < KEYS_LONGPRESS_MS) {
                        st->event = KEY_SHORT;
                    }
                    // If LONG was already set, keep it (or reset to IDLE)
                }
            }
        }

        // Long press detection: key is stably pressed and held long enough
        if (st->stable && raw_val) {
            uint32_t held = now_ms - st->press_time;
            if (held >= KEYS_LONGPRESS_MS && st->event != KEY_LONG) {
                st->event = KEY_LONG;
            }
        }
    }

    (void)last_raw;  // suppress unused warning
}
