/**
 * @file test_ntc_beta.cpp
 * RED tests for NTC beta formula - T12
 * Tests ntc_adc_to_temp() which is NOT yet implemented (RED state)
 */
#include <unity.h>
#include "sensors/ntc/ntc.h"

void setUp(void) {}
void tearDown(void) {}

/**
 * At 25°C (298.15K), NTC resistance = R25 = 10kΩ
 * With 10kΩ pullup and 3.3V ref:
 * V_ntc = 3.3 * 10000 / (10000 + 10000) = 1.65V
 * ADC = 1.65 / 3.3 * 4095 = 2047 (approx)
 */
void test_beta_25c_returns_25_degrees(void) {
    // ADC ~2047 corresponds to ~25°C with R25=10k, B=3950, pullup=10k
    float temp = ntc_adc_to_temp(2047);
    TEST_ASSERT_FLOAT_WITHIN(0.5f, 25.0f, temp);
}

/**
 * At 55°C, resistance decreases (NTC is negative coefficient)
 * R_55 = R25 * exp(B * (1/T55 - 1/T25))
 * T55 = 328.15K, T25 = 298.15K
 * R_55 = 10000 * exp(3950 * (1/328.15 - 1/298.15)) ≈ 2972Ω
 * V_ntc = 3.3 * 2972 / (10000 + 2972) ≈ 0.756V
 * ADC = 0.756 / 3.3 * 4095 ≈ 938
 */
void test_beta_55c_monotonic_decrease(void) {
    float temp_25 = ntc_adc_to_temp(2047);
    float temp_55 = ntc_adc_to_temp(938);
    // 55°C reading should be higher temperature than 25°C reading
    TEST_ASSERT_GREATER_THAN(temp_25, temp_55);
    // And should be approximately 55°C
    TEST_ASSERT_FLOAT_WITHIN(2.0f, 55.0f, temp_55);
}

/**
 * Boundary test: ADC=0 (short circuit, very high temp) → NTC_SHORT fault
 * Should return NTC_TEMP_SHORT_C (150.0f) or higher
 */
void test_adc_to_temp_boundary_adc_0(void) {
    float temp = ntc_adc_to_temp(0);
    // ADC=0 means V_ntc=0, R_ntc=0 → short circuit → very high temp
    TEST_ASSERT_GREATER_OR_EQUAL(150.0f, temp);
}

/**
 * Boundary test: ADC=4095 (open circuit, very low temp) → NTC_OPEN fault
 * Should return NTC_TEMP_OPEN_C (-40.0f) or lower
 */
void test_adc_to_temp_boundary_adc_4095(void) {
    float temp = ntc_adc_to_temp(4095);
    // ADC=4095 means V_ntc=Vref, R_ntc=∞ → open circuit → very low temp
    TEST_ASSERT_LESS_OR_EQUAL(-40.0f, temp);
}

/**
 * Temperature should decrease as ADC value increases
 * (Higher ADC = higher voltage = lower NTC resistance = higher temperature... wait)
 * Actually: Higher ADC = higher V_ntc = lower R_ntc (since pullup is on top)
 * Wait: V_ntc = Vref * R_ntc / (R_pullup + R_ntc)
 * Higher V_ntc → higher R_ntc → lower temperature
 * So: higher ADC → lower temperature (monotonic)
 */
void test_temperature_monotonically_decreases_with_adc(void) {
    float temp_low_adc  = ntc_adc_to_temp(500);   // low ADC → low V → low R → high temp
    float temp_mid_adc  = ntc_adc_to_temp(2047);  // mid ADC → ~25°C
    float temp_high_adc = ntc_adc_to_temp(3500);  // high ADC → high V → high R → low temp
    TEST_ASSERT_GREATER_THAN(temp_mid_adc, temp_low_adc);
    TEST_ASSERT_GREATER_THAN(temp_high_adc, temp_mid_adc);
}

int main(int argc, char **argv) {
    UNITY_BEGIN();
    RUN_TEST(test_beta_25c_returns_25_degrees);
    RUN_TEST(test_beta_55c_monotonic_decrease);
    RUN_TEST(test_adc_to_temp_boundary_adc_0);
    RUN_TEST(test_adc_to_temp_boundary_adc_4095);
    RUN_TEST(test_temperature_monotonically_decreases_with_adc);
    return UNITY_END();
}
