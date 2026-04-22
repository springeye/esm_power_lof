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

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_single_bounce_not_confirmed);
    RUN_TEST(test_three_consecutive_samples_confirm);
    RUN_TEST(test_short_press_event);
    RUN_TEST(test_long_press_event);
    RUN_TEST(test_release_after_long_press);
    return UNITY_END();
}
