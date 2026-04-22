/**
 * @file test_psu_fsm.cpp
 * RED tests for PSU state machine - T14
 * Tests psu_fsm_transition() which is NOT yet implemented (RED state)
 * States: Off/Standby/Starting/On/Stopping/Fault (design.md D7)
 */
#include <unity.h>
#include "power/psu_fsm.h"

void setUp(void) {}
void tearDown(void) {}

/* --- Legal transitions --- */

void test_off_boot_to_standby(void) {
    PsuState next = psu_fsm_transition(PSU_OFF, EVT_BOOT);
    TEST_ASSERT_EQUAL_INT(PSU_STANDBY, next);
}

void test_standby_short_press_to_starting(void) {
    PsuState next = psu_fsm_transition(PSU_STANDBY, EVT_KEY_SHORT);
    TEST_ASSERT_EQUAL_INT(PSU_STARTING, next);
}

void test_starting_pwok_high_to_on(void) {
    PsuState next = psu_fsm_transition(PSU_STARTING, EVT_PWOK_HIGH);
    TEST_ASSERT_EQUAL_INT(PSU_ON, next);
}

void test_starting_timeout_to_fault(void) {
    PsuState next = psu_fsm_transition(PSU_STARTING, EVT_TIMEOUT_1S);
    TEST_ASSERT_EQUAL_INT(PSU_FAULT, next);
}

void test_on_long_press_to_stopping(void) {
    PsuState next = psu_fsm_transition(PSU_ON, EVT_KEY_LONG);
    TEST_ASSERT_EQUAL_INT(PSU_STOPPING, next);
}

void test_on_pwok_lost_to_fault(void) {
    PsuState next = psu_fsm_transition(PSU_ON, EVT_PWOK_LOST_100MS);
    TEST_ASSERT_EQUAL_INT(PSU_FAULT, next);
}

void test_stopping_pwok_low_to_off(void) {
    PsuState next = psu_fsm_transition(PSU_STOPPING, EVT_PWOK_LOW);
    TEST_ASSERT_EQUAL_INT(PSU_OFF, next);
}

void test_fault_reset_to_off(void) {
    PsuState next = psu_fsm_transition(PSU_FAULT, EVT_FAULT_RESET);
    TEST_ASSERT_EQUAL_INT(PSU_OFF, next);
}

/* --- Illegal transitions (should stay in current state) --- */

void test_standby_long_press_stays_standby(void) {
    // Long press in Standby should NOT go to Stopping
    PsuState next = psu_fsm_transition(PSU_STANDBY, EVT_KEY_LONG);
    TEST_ASSERT_NOT_EQUAL(PSU_STOPPING, next);
}

void test_on_short_press_stays_on(void) {
    // Short press in On state should NOT change state
    PsuState next = psu_fsm_transition(PSU_ON, EVT_KEY_SHORT);
    TEST_ASSERT_EQUAL_INT(PSU_ON, next);
}

void test_off_pwok_high_stays_off(void) {
    // PWOK_HIGH in Off state should not trigger anything
    PsuState next = psu_fsm_transition(PSU_OFF, EVT_PWOK_HIGH);
    TEST_ASSERT_EQUAL_INT(PSU_OFF, next);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_off_boot_to_standby);
    RUN_TEST(test_standby_short_press_to_starting);
    RUN_TEST(test_starting_pwok_high_to_on);
    RUN_TEST(test_starting_timeout_to_fault);
    RUN_TEST(test_on_long_press_to_stopping);
    RUN_TEST(test_on_pwok_lost_to_fault);
    RUN_TEST(test_stopping_pwok_low_to_off);
    RUN_TEST(test_fault_reset_to_off);
    RUN_TEST(test_standby_long_press_stays_standby);
    RUN_TEST(test_on_short_press_stays_on);
    RUN_TEST(test_off_pwok_high_stays_off);
    return UNITY_END();
}
