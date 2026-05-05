/**
 * @file test_keys_debounce.cpp
 * RED tests for key debounce logic - T15
 * Tests key_debounce_update() which is NOT yet implemented (RED state)
 * Debounce: 5ms sampling, 3 consecutive same-level samples to confirm
 */
#include <unity.h>
#include "input/keys.h"

void setUp(void) {}
void tearDown(void) {}

/**
 * Single bounce should NOT update stable state
 * Only 1 sample at new level → not confirmed
 */
void test_single_bounce_not_confirmed(void) {
    KeyState st = {false, false, KEY_IDLE, 0};
    // Initial: stable=false, raw=false
    key_debounce_update(&st, false, 0);
    key_debounce_update(&st, false, 5);
    key_debounce_update(&st, false, 10);
    // Now one bounce
    key_debounce_update(&st, true, 15);
    // Should NOT be confirmed yet
    TEST_ASSERT_FALSE(st.stable);
}

/**
 * 3 consecutive same-level samples (≥15ms) confirm state change
 */
void test_three_consecutive_samples_confirm(void) {
    KeyState st = {false, false, KEY_IDLE, 0};
    // Press: 3 consecutive true samples at 5ms intervals
    key_debounce_update(&st, true, 0);
    key_debounce_update(&st, true, 5);
    key_debounce_update(&st, true, 10);
    // Should be confirmed now
    TEST_ASSERT_TRUE(st.stable);
}

/**
 * Short press: press confirmed, then release confirmed
 * Duration < KEYS_LONGPRESS_MS (800ms) → SHORT event
 */
void test_short_press_event(void) {
    KeyState st = {false, false, KEY_IDLE, 0};
    // Press
    key_debounce_update(&st, true, 0);
    key_debounce_update(&st, true, 5);
    key_debounce_update(&st, true, 10);
    // Release after 100ms (short press)
    key_debounce_update(&st, false, 110);
    key_debounce_update(&st, false, 115);
    key_debounce_update(&st, false, 120);
    // Should generate SHORT event
    TEST_ASSERT_EQUAL_INT(KEY_SHORT, st.event);
}

/**
 * Long press: press held for ≥ KEYS_LONGPRESS_MS (800ms) → LONG event
 */
void test_long_press_event(void) {
    KeyState st = {false, false, KEY_IDLE, 0};
    // Press confirmed
    key_debounce_update(&st, true, 0);
    key_debounce_update(&st, true, 5);
    key_debounce_update(&st, true, 10);
    // Hold for 900ms
    key_debounce_update(&st, true, 900);
    // Should generate LONG event
    TEST_ASSERT_EQUAL_INT(KEY_LONG, st.event);
}

/**
 * Release after long press should reset event
 */
void test_release_after_long_press(void) {
    KeyState st = {false, false, KEY_IDLE, 0};
    // Press and hold
    key_debounce_update(&st, true, 0);
    key_debounce_update(&st, true, 5);
    key_debounce_update(&st, true, 10);
    key_debounce_update(&st, true, 900);  // LONG triggered
    // Release
    key_debounce_update(&st, false, 910);
    key_debounce_update(&st, false, 915);
    key_debounce_update(&st, false, 920);
    // After release, stable should be false
    TEST_ASSERT_FALSE(st.stable);
}

/**
 * Regression: polling multiple KeyState instances across cycles must not
 * corrupt debounce state for another key.
 * Matches src/app/tasks.cpp input_task pattern: 3 keys polled in a loop
 * every 5ms, each with its own KeyState.
 */
void test_multi_key_polling_no_cross_corruption(void) {
    KeyState st_k1 = {false, false, KEY_IDLE, 0};
    KeyState st_k2 = {false, false, KEY_IDLE, 0};

    // Simulate tasks.cpp input_task loop: poll K1 then K2 each cycle.
    // K1: pressed (true), K2: has a 1-sample bounce (true) then stays idle (false).
    // With shared statics, K2's bounce resets consec_count for K1.

    // Cycle 1 (t=0): K1 pressed, K2 bounces
    key_debounce_update(&st_k1, true,  0);
    key_debounce_update(&st_k2, true,  0);  // bounce

    // Cycle 2 (t=5): K1 still pressed, K2 back to idle
    key_debounce_update(&st_k1, true,  5);
    key_debounce_update(&st_k2, false, 5);

    // Cycle 3 (t=10): K1 still pressed, K2 idle
    key_debounce_update(&st_k1, true,  10);
    key_debounce_update(&st_k2, false, 10);

    // K1 should be confirmed pressed (3 consecutive true from its perspective)
    TEST_ASSERT_TRUE(st_k1.stable);
    // K2 should remain unpressed (bounce was transient)
    TEST_ASSERT_FALSE(st_k2.stable);
}

/**
 * Regression: after KEY_LONG is consumed/reset by caller, holding the same
 * key must NOT emit KEY_LONG again until key is released and pressed again.
 * Bug scenario: consumer sets event=KEY_IDLE, next poll sees event!=KEY_LONG
 * and re-emits KEY_LONG, causing repeated page-switch crashes in settings.
 */
void test_long_press_not_repeated_after_consume(void) {
    KeyState st = {false, false, KEY_IDLE, 0};

    // Press confirmed
    key_debounce_update(&st, true, 0);
    key_debounce_update(&st, true, 5);
    key_debounce_update(&st, true, 10);

    // Hold long enough → KEY_LONG
    key_debounce_update(&st, true, 900);
    TEST_ASSERT_EQUAL_INT(KEY_LONG, st.event);

    // Consumer consumes event (like tasks.cpp: st.event = KEY_IDLE)
    st.event = KEY_IDLE;

    // Next poll cycle (key still held) must NOT re-emit KEY_LONG
    key_debounce_update(&st, true, 905);
    TEST_ASSERT_EQUAL_INT(KEY_IDLE, st.event);

    // Many more cycles, still held — must stay IDLE
    key_debounce_update(&st, true, 910);
    key_debounce_update(&st, true, 915);
    key_debounce_update(&st, true, 1500);
    TEST_ASSERT_EQUAL_INT(KEY_IDLE, st.event);

    // Release
    key_debounce_update(&st, false, 1505);
    key_debounce_update(&st, false, 1510);
    key_debounce_update(&st, false, 1515);
    // After release, stable=false, no SHORT (was a long press)
    TEST_ASSERT_FALSE(st.stable);

    // Press again → KEY_LONG should be possible again
    key_debounce_update(&st, true, 2000);
    key_debounce_update(&st, true, 2005);
    key_debounce_update(&st, true, 2010);
    key_debounce_update(&st, true, 2900);
    TEST_ASSERT_EQUAL_INT(KEY_LONG, st.event);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_single_bounce_not_confirmed);
    RUN_TEST(test_three_consecutive_samples_confirm);
    RUN_TEST(test_short_press_event);
    RUN_TEST(test_long_press_event);
    RUN_TEST(test_release_after_long_press);
    RUN_TEST(test_multi_key_polling_no_cross_corruption);
    RUN_TEST(test_long_press_not_repeated_after_consume);
    return UNITY_END();
}
