#include <unity.h>
#include "app/config_manager.h"

void setUp(void) {
    config_manager::reset_to_defaults();
}

void tearDown(void) {}

void test_default_values(void) {
    TEST_ASSERT_EQUAL_FLOAT(35.0f, config_manager::get_fan_temp_low());
    TEST_ASSERT_EQUAL_FLOAT(45.0f, config_manager::get_fan_temp_mid());
    TEST_ASSERT_EQUAL_FLOAT(55.0f, config_manager::get_fan_temp_high());
    TEST_ASSERT_EQUAL_FLOAT(60.0f, config_manager::get_fan_temp_force());
    TEST_ASSERT_EQUAL_UINT8(20, config_manager::get_fan_pwm_min_percent());
    TEST_ASSERT_EQUAL_UINT8(60, config_manager::get_fan_pwm_mid_percent());
    TEST_ASSERT_EQUAL_FLOAT(2.0f, config_manager::get_fan_hysteresis());
    TEST_ASSERT_EQUAL_FLOAT(65.0f, config_manager::get_temp_warning_threshold());
    TEST_ASSERT_EQUAL_FLOAT(75.0f, config_manager::get_temp_shutdown_threshold());
    TEST_ASSERT_EQUAL_UINT8(80, config_manager::get_brightness_percent());
    TEST_ASSERT_EQUAL_UINT16(750, config_manager::get_design_power_w());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, config_manager::get_ntc_temp_offset());
}

void test_getter_setter_fan_temp(void) {
    config_manager::set_fan_temp_low(40.0f);
    TEST_ASSERT_EQUAL_FLOAT(40.0f, config_manager::get_fan_temp_low());

    config_manager::set_fan_temp_mid(50.0f);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, config_manager::get_fan_temp_mid());

    config_manager::set_fan_temp_high(60.0f);
    TEST_ASSERT_EQUAL_FLOAT(60.0f, config_manager::get_fan_temp_high());

    config_manager::set_fan_temp_force(70.0f);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, config_manager::get_fan_temp_force());
}

void test_getter_setter_fan_pwm(void) {
    config_manager::set_fan_pwm_min_percent(30);
    TEST_ASSERT_EQUAL_UINT8(30, config_manager::get_fan_pwm_min_percent());

    config_manager::set_fan_pwm_mid_percent(70);
    TEST_ASSERT_EQUAL_UINT8(70, config_manager::get_fan_pwm_mid_percent());

    config_manager::set_fan_hysteresis(3.0f);
    TEST_ASSERT_EQUAL_FLOAT(3.0f, config_manager::get_fan_hysteresis());
}

void test_getter_setter_temp_protection(void) {
    config_manager::set_temp_warning_threshold(70.0f);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, config_manager::get_temp_warning_threshold());

    config_manager::set_temp_shutdown_threshold(80.0f);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, config_manager::get_temp_shutdown_threshold());
}

void test_getter_setter_display_power_sensor(void) {
    config_manager::set_brightness_percent(50);
    TEST_ASSERT_EQUAL_UINT8(50, config_manager::get_brightness_percent());

    config_manager::set_design_power_w(550);
    TEST_ASSERT_EQUAL_UINT16(550, config_manager::get_design_power_w());

    config_manager::set_ntc_temp_offset(2.5f);
    TEST_ASSERT_EQUAL_FLOAT(2.5f, config_manager::get_ntc_temp_offset());
}

void test_boundary_clamping_fan_temp_low(void) {
    config_manager::set_fan_temp_low(10.0f);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, config_manager::get_fan_temp_low());

    config_manager::set_fan_temp_low(100.0f);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, config_manager::get_fan_temp_low());

    config_manager::set_fan_temp_low(20.0f);
    TEST_ASSERT_EQUAL_FLOAT(20.0f, config_manager::get_fan_temp_low());

    config_manager::set_fan_temp_low(50.0f);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, config_manager::get_fan_temp_low());
}

void test_boundary_clamping_fan_temp_mid(void) {
    config_manager::set_fan_temp_mid(20.0f);
    TEST_ASSERT_EQUAL_FLOAT(30.0f, config_manager::get_fan_temp_mid());

    config_manager::set_fan_temp_mid(100.0f);
    TEST_ASSERT_EQUAL_FLOAT(60.0f, config_manager::get_fan_temp_mid());
}

void test_boundary_clamping_fan_temp_high(void) {
    config_manager::set_fan_temp_high(30.0f);
    TEST_ASSERT_EQUAL_FLOAT(40.0f, config_manager::get_fan_temp_high());

    config_manager::set_fan_temp_high(100.0f);
    TEST_ASSERT_EQUAL_FLOAT(70.0f, config_manager::get_fan_temp_high());
}

void test_boundary_clamping_fan_temp_force(void) {
    config_manager::set_fan_temp_force(40.0f);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, config_manager::get_fan_temp_force());

    config_manager::set_fan_temp_force(100.0f);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, config_manager::get_fan_temp_force());
}

void test_boundary_clamping_fan_pwm(void) {
    config_manager::set_fan_pwm_min_percent(5);
    TEST_ASSERT_EQUAL_UINT8(10, config_manager::get_fan_pwm_min_percent());

    config_manager::set_fan_pwm_min_percent(200);
    TEST_ASSERT_EQUAL_UINT8(50, config_manager::get_fan_pwm_min_percent());

    config_manager::set_fan_pwm_mid_percent(20);
    TEST_ASSERT_EQUAL_UINT8(40, config_manager::get_fan_pwm_mid_percent());

    config_manager::set_fan_pwm_mid_percent(200);
    TEST_ASSERT_EQUAL_UINT8(90, config_manager::get_fan_pwm_mid_percent());
}

void test_boundary_clamping_hysteresis(void) {
    config_manager::set_fan_hysteresis(0.1f);
    TEST_ASSERT_EQUAL_FLOAT(0.5f, config_manager::get_fan_hysteresis());

    config_manager::set_fan_hysteresis(10.0f);
    TEST_ASSERT_EQUAL_FLOAT(5.0f, config_manager::get_fan_hysteresis());
}

void test_boundary_clamping_temp_protection(void) {
    config_manager::set_temp_warning_threshold(40.0f);
    TEST_ASSERT_EQUAL_FLOAT(50.0f, config_manager::get_temp_warning_threshold());

    config_manager::set_temp_warning_threshold(100.0f);
    TEST_ASSERT_EQUAL_FLOAT(80.0f, config_manager::get_temp_warning_threshold());

    config_manager::set_temp_shutdown_threshold(50.0f);
    TEST_ASSERT_EQUAL_FLOAT(60.0f, config_manager::get_temp_shutdown_threshold());

    config_manager::set_temp_shutdown_threshold(100.0f);
    TEST_ASSERT_EQUAL_FLOAT(90.0f, config_manager::get_temp_shutdown_threshold());
}

void test_boundary_clamping_brightness(void) {
    config_manager::set_brightness_percent(5);
    TEST_ASSERT_EQUAL_UINT8(10, config_manager::get_brightness_percent());

    config_manager::set_brightness_percent(200);
    TEST_ASSERT_EQUAL_UINT8(100, config_manager::get_brightness_percent());
}

void test_boundary_clamping_design_power(void) {
    TEST_ASSERT_EQUAL_UINT16(750, config_manager::get_design_power_w());

    config_manager::set_design_power_w(100);
    TEST_ASSERT_EQUAL_UINT16(750, config_manager::get_design_power_w());

    config_manager::set_design_power_w(600);
    TEST_ASSERT_EQUAL_UINT16(750, config_manager::get_design_power_w());

    config_manager::set_design_power_w(350);
    TEST_ASSERT_EQUAL_UINT16(350, config_manager::get_design_power_w());

    config_manager::set_design_power_w(450);
    TEST_ASSERT_EQUAL_UINT16(450, config_manager::get_design_power_w());

    config_manager::set_design_power_w(550);
    TEST_ASSERT_EQUAL_UINT16(550, config_manager::get_design_power_w());
}

void test_boundary_clamping_ntc_offset(void) {
    config_manager::set_ntc_temp_offset(-20.0f);
    TEST_ASSERT_EQUAL_FLOAT(-10.0f, config_manager::get_ntc_temp_offset());

    config_manager::set_ntc_temp_offset(20.0f);
    TEST_ASSERT_EQUAL_FLOAT(10.0f, config_manager::get_ntc_temp_offset());

    config_manager::set_ntc_temp_offset(-5.0f);
    TEST_ASSERT_EQUAL_FLOAT(-5.0f, config_manager::get_ntc_temp_offset());
}

void test_reset_to_defaults(void) {
    config_manager::set_fan_temp_low(40.0f);
    config_manager::set_fan_temp_mid(50.0f);
    config_manager::set_fan_pwm_min_percent(30);
    config_manager::set_brightness_percent(50);
    config_manager::set_design_power_w(550);
    config_manager::set_ntc_temp_offset(5.0f);

    TEST_ASSERT_EQUAL_FLOAT(40.0f, config_manager::get_fan_temp_low());
    TEST_ASSERT_EQUAL_FLOAT(50.0f, config_manager::get_fan_temp_mid());
    TEST_ASSERT_EQUAL_UINT8(30, config_manager::get_fan_pwm_min_percent());
    TEST_ASSERT_EQUAL_UINT8(50, config_manager::get_brightness_percent());
    TEST_ASSERT_EQUAL_UINT16(550, config_manager::get_design_power_w());
    TEST_ASSERT_EQUAL_FLOAT(5.0f, config_manager::get_ntc_temp_offset());

    config_manager::reset_to_defaults();

    TEST_ASSERT_EQUAL_FLOAT(35.0f, config_manager::get_fan_temp_low());
    TEST_ASSERT_EQUAL_FLOAT(45.0f, config_manager::get_fan_temp_mid());
    TEST_ASSERT_EQUAL_FLOAT(55.0f, config_manager::get_fan_temp_high());
    TEST_ASSERT_EQUAL_FLOAT(60.0f, config_manager::get_fan_temp_force());
    TEST_ASSERT_EQUAL_UINT8(20, config_manager::get_fan_pwm_min_percent());
    TEST_ASSERT_EQUAL_UINT8(60, config_manager::get_fan_pwm_mid_percent());
    TEST_ASSERT_EQUAL_FLOAT(2.0f, config_manager::get_fan_hysteresis());
    TEST_ASSERT_EQUAL_FLOAT(65.0f, config_manager::get_temp_warning_threshold());
    TEST_ASSERT_EQUAL_FLOAT(75.0f, config_manager::get_temp_shutdown_threshold());
    TEST_ASSERT_EQUAL_UINT8(80, config_manager::get_brightness_percent());
    TEST_ASSERT_EQUAL_UINT16(750, config_manager::get_design_power_w());
    TEST_ASSERT_EQUAL_FLOAT(0.0f, config_manager::get_ntc_temp_offset());
}

int main(void) {
    UNITY_BEGIN();

    RUN_TEST(test_default_values);

    RUN_TEST(test_getter_setter_fan_temp);
    RUN_TEST(test_getter_setter_fan_pwm);
    RUN_TEST(test_getter_setter_temp_protection);
    RUN_TEST(test_getter_setter_display_power_sensor);

    RUN_TEST(test_boundary_clamping_fan_temp_low);
    RUN_TEST(test_boundary_clamping_fan_temp_mid);
    RUN_TEST(test_boundary_clamping_fan_temp_high);
    RUN_TEST(test_boundary_clamping_fan_temp_force);
    RUN_TEST(test_boundary_clamping_fan_pwm);
    RUN_TEST(test_boundary_clamping_hysteresis);
    RUN_TEST(test_boundary_clamping_temp_protection);
    RUN_TEST(test_boundary_clamping_brightness);
    RUN_TEST(test_boundary_clamping_design_power);
    RUN_TEST(test_boundary_clamping_ntc_offset);

    RUN_TEST(test_reset_to_defaults);

    return UNITY_END();
}
