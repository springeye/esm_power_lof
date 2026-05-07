#include <unity.h>
#include "../../src/ui_bridge/power_history.h"
#include "../../src/ui_bridge/view_manager.h"
#include "../../src/app/config_manager.h"

void setUp(void) {}
void tearDown(void) {}

void test_power_history_init(void) {
    power_history_init();
    uint32_t count = 0;
    const PowerPoint* data = nullptr;
    power_history_get_range(0, 600000, &count, &data);
    TEST_ASSERT_EQUAL_UINT32(0, count);
    TEST_ASSERT_NULL(data);
}

void test_power_history_push_single(void) {
    power_history_init();
    power_history_push(0, 1000, 50.0f);
    uint32_t count = 0;
    const PowerPoint* data = nullptr;
    power_history_get_range(0, 600000, &count, &data);
    TEST_ASSERT_EQUAL_UINT32(1, count);
    TEST_ASSERT_NOT_NULL(data);
    TEST_ASSERT_EQUAL_UINT32(1000, data[0].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, data[0].power_w);
}

void test_power_history_push_multiple(void) {
    power_history_init();
    for (int i = 0; i < 100; ++i) {
        power_history_push(0, i * 200, static_cast<float>(i));
    }
    uint32_t count = 0;
    const PowerPoint* data = nullptr;
    power_history_get_range(0, 600000, &count, &data);
    TEST_ASSERT_EQUAL_UINT32(100, count);
    TEST_ASSERT_EQUAL_UINT32(0, data[0].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, data[0].power_w);
    TEST_ASSERT_EQUAL_UINT32(99 * 200, data[99].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 99.0f, data[99].power_w);
}

void test_power_history_overflow(void) {
    power_history_init();
    for (uint32_t i = 0; i < 3500; ++i) {
        power_history_push(0, i * 200, static_cast<float>(i));
    }
    uint32_t count = 0;
    const PowerPoint* data = nullptr;
    power_history_get_range(0, 600000, &count, &data);
    TEST_ASSERT_EQUAL_UINT32(3000, count);
    TEST_ASSERT_EQUAL_UINT32(500 * 200, data[0].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 500.0f, data[0].power_w);
}

void test_power_history_time_window(void) {
    power_history_init();
    uint32_t now = 600000;
    for (uint32_t i = 0; i < 3000; ++i) {
        power_history_push(0, now - (3000 - i) * 200, static_cast<float>(i));
    }
    uint32_t count = 0;
    const PowerPoint* data = nullptr;
    power_history_get_range(0, 60000, &count, &data);
    TEST_ASSERT_EQUAL_UINT32(300, count);
}

void test_power_history_channels(void) {
    power_history_init();
    power_history_push(0, 1000, 10.0f);
    power_history_push(1, 1000, 20.0f);
    power_history_push(2, 1000, 30.0f);
    uint32_t count0 = 0, count1 = 0, count2 = 0;
    const PowerPoint* data0 = nullptr;
    const PowerPoint* data1 = nullptr;
    const PowerPoint* data2 = nullptr;
    power_history_get_range(0, 600000, &count0, &data0);
    power_history_get_range(1, 600000, &count1, &data1);
    power_history_get_range(2, 600000, &count2, &data2);
    TEST_ASSERT_EQUAL_UINT32(1, count0);
    TEST_ASSERT_EQUAL_UINT32(1, count1);
    TEST_ASSERT_EQUAL_UINT32(1, count2);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, data0[0].power_w);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, data1[0].power_w);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 30.0f, data2[0].power_w);
}

void test_config_yaxis_default(void) {
    config_manager::init();
    uint8_t mode = config_manager::get_chart_yaxis_mode();
    TEST_ASSERT_EQUAL_UINT8(0, mode);
}

void test_config_yaxis_set_get(void) {
    config_manager::init();
    config_manager::set_chart_yaxis_mode(1);
    TEST_ASSERT_EQUAL_UINT8(1, config_manager::get_chart_yaxis_mode());
    config_manager::set_chart_yaxis_mode(0);
    TEST_ASSERT_EQUAL_UINT8(0, config_manager::get_chart_yaxis_mode());
}

void test_config_yaxis_clamp(void) {
    config_manager::init();
    config_manager::set_chart_yaxis_mode(99);
    TEST_ASSERT_EQUAL_UINT8(1, config_manager::get_chart_yaxis_mode());
    config_manager::set_chart_yaxis_mode(255);
    TEST_ASSERT_EQUAL_UINT8(1, config_manager::get_chart_yaxis_mode());
}

void test_view_manager_cycle_forward(void) {
    TEST_ASSERT_EQUAL(VIEW_DEFAULT, view_manager::view_manager_get_current());
    view_manager::view_manager_cycle(1);
    TEST_ASSERT_EQUAL(VIEW_CHART_CH1, view_manager::view_manager_get_current());
    view_manager::view_manager_cycle(1);
    TEST_ASSERT_EQUAL(VIEW_CHART_CH2, view_manager::view_manager_get_current());
    view_manager::view_manager_cycle(1);
    TEST_ASSERT_EQUAL(VIEW_CHART_CH3, view_manager::view_manager_get_current());
    view_manager::view_manager_cycle(1);
    TEST_ASSERT_EQUAL(VIEW_DEFAULT, view_manager::view_manager_get_current());
}

void test_view_manager_cycle_backward(void) {
    view_manager::view_manager_switch_to(VIEW_DEFAULT);
    TEST_ASSERT_EQUAL(VIEW_DEFAULT, view_manager::view_manager_get_current());
    view_manager::view_manager_cycle(-1);
    TEST_ASSERT_EQUAL(VIEW_CHART_CH3, view_manager::view_manager_get_current());
    view_manager::view_manager_cycle(-1);
    TEST_ASSERT_EQUAL(VIEW_CHART_CH2, view_manager::view_manager_get_current());
    view_manager::view_manager_cycle(-1);
    TEST_ASSERT_EQUAL(VIEW_CHART_CH1, view_manager::view_manager_get_current());
    view_manager::view_manager_cycle(-1);
    TEST_ASSERT_EQUAL(VIEW_DEFAULT, view_manager::view_manager_get_current());
}

void test_view_manager_switch_to(void) {
    view_manager::view_manager_switch_to(VIEW_CHART_CH2);
    TEST_ASSERT_EQUAL(VIEW_CHART_CH2, view_manager::view_manager_get_current());
    view_manager::view_manager_switch_to(VIEW_DEFAULT);
    TEST_ASSERT_EQUAL(VIEW_DEFAULT, view_manager::view_manager_get_current());
}

void setup() {
    UNITY_BEGIN();
    RUN_TEST(test_power_history_init);
    RUN_TEST(test_power_history_push_single);
    RUN_TEST(test_power_history_push_multiple);
    RUN_TEST(test_power_history_overflow);
    RUN_TEST(test_power_history_time_window);
    RUN_TEST(test_power_history_channels);
    RUN_TEST(test_config_yaxis_default);
    RUN_TEST(test_config_yaxis_set_get);
    RUN_TEST(test_config_yaxis_clamp);
    RUN_TEST(test_view_manager_cycle_forward);
    RUN_TEST(test_view_manager_cycle_backward);
    RUN_TEST(test_view_manager_switch_to);
    UNITY_END();
}

void loop() {}
