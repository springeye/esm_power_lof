/**
 * @file test_hysteresis.cpp
 * RED tests for hysteresis control - T16
 * Tests hysteresis_apply() which is NOT yet implemented (RED state)
 * Hysteresis band: FAN_HYSTERESIS = 2.0°C
 */
#include <unity.h>
#include "fan/fan_curve.h"

void setUp(void) {}
void tearDown(void) {}

/**
 * Temperature rising above target+band → output follows temperature
 */
void test_rising_above_upper_band(void) {
    // target=50°C, band=2°C → upper threshold = 52°C
    // current=53°C (above upper) → output should be 53°C (follow)
    float result = hysteresis_apply(53.0f, 50.0f, 2.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 53.0f, result);
}

/**
 * Temperature falling below target-band → output follows temperature
 */
void test_falling_below_lower_band(void) {
    // target=50°C, band=2°C → lower threshold = 48°C
    // current=47°C (below lower) → output should be 47°C (follow)
    float result = hysteresis_apply(47.0f, 50.0f, 2.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 47.0f, result);
}

/**
 * Temperature within band → output stays at target (hysteresis holds)
 */
void test_within_band_stays_at_target(void) {
    // target=50°C, band=2°C → band is [48, 52]
    // current=50°C (within band) → output should be target=50°C
    float result = hysteresis_apply(50.0f, 50.0f, 2.0f);
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 50.0f, result);
}

/**
 * Exactly at boundary: target+band → should NOT trigger (boundary is exclusive or inclusive?)
 * Design: band is ±2°C, so at exactly 52°C it's at the edge
 */
void test_at_upper_boundary(void) {
    float result = hysteresis_apply(52.0f, 50.0f, 2.0f);
    // At exactly the boundary, behavior is implementation-defined
    // but result should be within [50, 52]
    TEST_ASSERT_GREATER_OR_EQUAL(50.0f, result);
    TEST_ASSERT_LESS_OR_EQUAL(52.0f, result);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_rising_above_upper_band);
    RUN_TEST(test_falling_below_lower_band);
    RUN_TEST(test_within_band_stays_at_target);
    RUN_TEST(test_at_upper_boundary);
    return UNITY_END();
}
