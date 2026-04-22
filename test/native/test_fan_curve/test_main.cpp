/**
 * @file test_fan_curve.cpp
 * RED tests for fan temperature-to-PWM curve - T13
 * Tests fan_temp_to_pwm() which is NOT yet implemented (RED state)
 * PWM range: 0-1023 (10-bit), design.md D4-D5
 */
#include <unity.h>
#include "fan/fan_curve.h"

void setUp(void) {}
void tearDown(void) {}

/* Below 30°C: constant 20% = 205/1023 */
void test_below_30c_returns_min_duty(void) {
    uint16_t duty = fan_temp_to_pwm(25.0f);
    TEST_ASSERT_EQUAL_UINT16(205, duty);
}

void test_at_30c_returns_min_duty(void) {
    uint16_t duty = fan_temp_to_pwm(30.0f);
    TEST_ASSERT_EQUAL_UINT16(205, duty);
}

/* Linear segment 30-50°C: 20%→60% (205→614) */
void test_at_40c_midpoint_first_segment(void) {
    uint16_t duty = fan_temp_to_pwm(40.0f);
    // 40°C is midpoint of 30-50°C segment: 20% + (40-30)/(50-30) * (60%-20%) = 40%
    // 40% of 1023 = 409, allow ±5
    TEST_ASSERT_UINT16_WITHIN(5, 409, duty);
}

void test_at_50c_returns_60_percent(void) {
    uint16_t duty = fan_temp_to_pwm(50.0f);
    // 60% of 1023 = 614, allow ±2
    TEST_ASSERT_UINT16_WITHIN(2, 614, duty);
}

/* Linear segment 50-70°C: 60%→100% (614→1023) */
void test_at_60c_midpoint_second_segment(void) {
    uint16_t duty = fan_temp_to_pwm(60.0f);
    // 60°C is midpoint of 50-70°C: 60% + (60-50)/(70-50) * (100%-60%) = 80%
    // 80% of 1023 = 818, allow ±5
    TEST_ASSERT_UINT16_WITHIN(5, 818, duty);
}

void test_at_70c_returns_full_duty(void) {
    uint16_t duty = fan_temp_to_pwm(70.0f);
    TEST_ASSERT_EQUAL_UINT16(1023, duty);
}

/* At or above 75°C: emergency full speed */
void test_at_75c_force_full_duty(void) {
    uint16_t duty = fan_temp_to_pwm(75.0f);
    TEST_ASSERT_EQUAL_UINT16(1023, duty);
}

void test_above_75c_force_full_duty(void) {
    uint16_t duty = fan_temp_to_pwm(80.0f);
    TEST_ASSERT_EQUAL_UINT16(1023, duty);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_below_30c_returns_min_duty);
    RUN_TEST(test_at_30c_returns_min_duty);
    RUN_TEST(test_at_40c_midpoint_first_segment);
    RUN_TEST(test_at_50c_returns_60_percent);
    RUN_TEST(test_at_60c_midpoint_second_segment);
    RUN_TEST(test_at_70c_returns_full_duty);
    RUN_TEST(test_at_75c_force_full_duty);
    RUN_TEST(test_above_75c_force_full_duty);
    return UNITY_END();
}
