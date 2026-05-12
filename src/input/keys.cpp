#include "input/keys.h"
#include "app_config.h"

static const uint8_t DEBOUNCE_COUNT = 3u;

void key_debounce_update(KeyState *st, bool raw_val, uint32_t now_ms) {
    if (st == nullptr) return;

    if (raw_val != st->raw) {
        st->raw = raw_val;
        st->consec_count = 1u;
        st->last_raw = raw_val;
    } else {
        if (st->consec_count < DEBOUNCE_COUNT) {
            st->consec_count++;
        }

        if (st->consec_count >= DEBOUNCE_COUNT && raw_val != st->stable) {
            bool was_stable = st->stable;
            st->stable = raw_val;

            if (raw_val) {
                st->press_time = now_ms;
                st->event = KEY_IDLE;
                st->long_fired = false;
            } else {
                if (was_stable) {
                    uint32_t duration = now_ms - st->press_time;
                    if (duration < KEYS_LONGPRESS_MS) {
                        // Check for double-click: two short presses within KEYS_DOUBLECLICK_MS
                        if (st->click_count > 0 && (now_ms - st->last_release_time) <= KEYS_DOUBLECLICK_MS) {
                            st->event = KEY_DOUBLE_CLICK;
                            st->click_count = 0;
                        } else {
                            st->event = KEY_SHORT;
                            st->click_count = 1;
                            st->last_release_time = now_ms;
                        }
                    }
                }
                st->long_fired = false;
            }
        }

        if (st->stable && raw_val) {
            uint32_t held = now_ms - st->press_time;
            if (held >= KEYS_LONGPRESS_MS && !st->long_fired) {
                st->event = KEY_LONG;
                st->long_fired = true;
                st->click_count = 0; // Reset click count on long press
            }
        }
    }
}
