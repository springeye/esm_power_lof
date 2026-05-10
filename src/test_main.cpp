#include <unity.h>
#include <cstdlib>
#include "../../src/ui_bridge/power_history.h"
#include "../../src/ui_bridge/view_manager.h"
#include "../../src/app/config_manager.h"

void setUp(void) {}
void tearDown(void) {}

void test_power_history_init(void) {
    power_history_init();
    PowerHistorySample points[10];
    uint32_t count = power_history_sample_window(0, 600000, 10, points, 10);
    TEST_ASSERT_EQUAL_UINT32(0, count);
}

void test_power_history_push_single(void) {
    power_history_init();
    power_history_push(0, 1000, 50.0f);
    PowerHistorySample points[10];
    uint32_t count = power_history_sample_window(0, 600000, 10, points, 10);
    TEST_ASSERT_EQUAL_UINT32(1, count);
    TEST_ASSERT_EQUAL_UINT32(1000, points[0].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 50.0f, points[0].power_w);
}

void test_power_history_push_multiple(void) {
    power_history_init();
    for (int i = 0; i < 100; ++i) {
        power_history_push(0, i * 200, static_cast<float>(i));
    }
    PowerHistorySample points[200];
    uint32_t count = power_history_sample_window(0, 600000, 200, points, 200);
    TEST_ASSERT_EQUAL_UINT32(100, count);
    TEST_ASSERT_EQUAL_UINT32(0, points[0].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, points[0].power_w);
    TEST_ASSERT_EQUAL_UINT32(99 * 200, points[99].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 99.0f, points[99].power_w);
}

void test_power_history_overflow(void) {
    power_history_init();
    for (uint32_t i = 0; i < 3500; ++i) {
        power_history_push(0, i * 200, static_cast<float>(i));
    }
    PowerHistorySample points[3500];
    uint32_t count = power_history_sample_window(0, 600000, 3500, points, 3500);
    TEST_ASSERT_EQUAL_UINT32(3000, count);
    TEST_ASSERT_EQUAL_UINT32(500 * 200, points[0].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 500.0f, points[0].power_w);
}

void test_power_history_time_window(void) {
    power_history_init();
    uint32_t now = 600000;
    for (uint32_t i = 0; i < 3000; ++i) {
        power_history_push(0, now - (3000 - i) * 200, static_cast<float>(i));
    }
    PowerHistorySample points[500];
    uint32_t count = power_history_sample_window(0, 60000, 500, points, 500);
    TEST_ASSERT_EQUAL_UINT32(300, count);
}

void test_power_history_channels(void) {
    power_history_init();
    power_history_push(0, 1000, 10.0f);
    power_history_push(1, 1000, 20.0f);
    power_history_push(2, 1000, 30.0f);
    PowerHistorySample points0[10], points1[10], points2[10];
    uint32_t count0 = power_history_sample_window(0, 600000, 10, points0, 10);
    uint32_t count1 = power_history_sample_window(1, 600000, 10, points1, 10);
    uint32_t count2 = power_history_sample_window(2, 600000, 10, points2, 10);
    TEST_ASSERT_EQUAL_UINT32(1, count0);
    TEST_ASSERT_EQUAL_UINT32(1, count1);
    TEST_ASSERT_EQUAL_UINT32(1, count2);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 10.0f, points0[0].power_w);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 20.0f, points1[0].power_w);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 30.0f, points2[0].power_w);
}

void test_power_history_time_window_bounds(void) {
    power_history_init();
    for (uint32_t i = 0; i < 100; ++i) {
        power_history_push(0, i * 200, static_cast<float>(i));
    }
    PowerHistorySample points[200];
    uint32_t count = power_history_sample_window(0, 60000, 200, points, 200);
    TEST_ASSERT_EQUAL_UINT32(100, count);
    TEST_ASSERT_EQUAL_UINT32(0, points[0].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 0.0f, points[0].power_w);
    TEST_ASSERT_EQUAL_UINT32(99 * 200, points[99].timestamp_ms);
    TEST_ASSERT_FLOAT_WITHIN(0.01f, 99.0f, points[99].power_w);
}

void test_power_history_query_is_stable(void) {
    power_history_init();
    for (uint32_t i = 0; i < 50; ++i) {
        power_history_push(0, i * 200, static_cast<float>(i));
    }
    PowerHistorySample points1[100];
    PowerHistorySample points2[100];
    uint32_t count1 = power_history_sample_window(0, 600000, 100, points1, 100);
    uint32_t count2 = power_history_sample_window(0, 600000, 100, points2, 100);
    TEST_ASSERT_EQUAL_UINT32(count1, count2);
    for (uint32_t i = 0; i < count1; ++i) {
        TEST_ASSERT_EQUAL_UINT32(points1[i].timestamp_ms, points2[i].timestamp_ms);
        TEST_ASSERT_FLOAT_WITHIN(0.001f, points1[i].power_w, points2[i].power_w);
    }
}

void test_power_history_overflow_keeps_order(void) {
    power_history_init();
    for (uint32_t i = 0; i < 3500; ++i) {
        power_history_push(0, i * 200, static_cast<float>(i));
    }
    PowerHistorySample points[3500];
    uint32_t count = power_history_sample_window(0, 600000, 3500, points, 3500);
    TEST_ASSERT_EQUAL_UINT32(3000, count);
    for (uint32_t i = 1; i < count; ++i) {
        TEST_ASSERT_TRUE(points[i].timestamp_ms > points[i - 1].timestamp_ms);
    }
}

void test_power_history_sample_window_caps_points(void) {
    power_history_init();
    for (uint32_t i = 0; i < 3500; ++i) {
        power_history_push(0, i * 200, static_cast<float>(i));
    }
    PowerHistorySample points[300];
    uint32_t count = power_history_sample_window(0, 600000, 300, points, 300);
    TEST_ASSERT_TRUE(count <= 300);
    for (uint32_t i = 1; i < count; ++i) {
        TEST_ASSERT_TRUE(points[i].timestamp_ms > points[i - 1].timestamp_ms);
    }
}

void test_power_history_sample_window_prefers_recent_range(void) {
    power_history_init();
    uint32_t now = 100000;
    for (uint32_t i = 0; i < 100; ++i) {
        power_history_push(0, now - (100 - i) * 1000, static_cast<float>(i));
    }
    PowerHistorySample points[20];
    uint32_t count = power_history_sample_window(0, 10000, 20, points, 20);
    TEST_ASSERT_EQUAL_UINT32(10, count);
    TEST_ASSERT_TRUE(points[0].timestamp_ms >= now - 10000);
    TEST_ASSERT_EQUAL_UINT32(now, points[count - 1].timestamp_ms);
}

void test_power_history_sample_window_zero_capacity(void) {
    power_history_init();
    power_history_push(0, 1000, 50.0f);

    PowerHistorySample points[10];
    TEST_ASSERT_EQUAL_UINT32(0, power_history_sample_window(0, 600000, 0, points, 10));

    TEST_ASSERT_EQUAL_UINT32(0, power_history_sample_window(0, 600000, 10, nullptr, 10));

    power_history_init();
    TEST_ASSERT_EQUAL_UINT32(0, power_history_sample_window(0, 600000, 10, points, 10));

    TEST_ASSERT_EQUAL_UINT32(0, power_history_sample_window(99, 600000, 10, points, 10));
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

void test_config_theme_mode_set_get_without_autosave(void) {
    config_manager::init();
    config_manager::set_theme_mode(0);
    TEST_ASSERT_EQUAL_UINT8(0, config_manager::get_theme_mode());
    config_manager::set_theme_mode(1);
    TEST_ASSERT_EQUAL_UINT8(1, config_manager::get_theme_mode());
}

void test_config_design_power_set_get_without_autosave(void) {
    config_manager::init();
    config_manager::set_design_power_w(350);
    TEST_ASSERT_EQUAL_UINT16(350, config_manager::get_design_power_w());
    config_manager::set_design_power_w(750);
    TEST_ASSERT_EQUAL_UINT16(750, config_manager::get_design_power_w());
    config_manager::set_design_power_w(999);
    TEST_ASSERT_EQUAL_UINT16(750, config_manager::get_design_power_w());
}

void test_config_save_to_nvs_callable_after_setters(void) {
    config_manager::init();
    config_manager::set_brightness_percent(50);
    config_manager::save_to_nvs();
    TEST_ASSERT_EQUAL_UINT8(50, config_manager::get_brightness_percent());
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
    RUN_TEST(test_power_history_time_window_bounds);
    RUN_TEST(test_power_history_query_is_stable);
    RUN_TEST(test_power_history_overflow_keeps_order);
    RUN_TEST(test_power_history_sample_window_caps_points);
    RUN_TEST(test_power_history_sample_window_prefers_recent_range);
    RUN_TEST(test_power_history_sample_window_zero_capacity);
    RUN_TEST(test_config_yaxis_default);
    RUN_TEST(test_config_yaxis_set_get);
    RUN_TEST(test_config_yaxis_clamp);
    RUN_TEST(test_config_theme_mode_set_get_without_autosave);
    RUN_TEST(test_config_design_power_set_get_without_autosave);
    RUN_TEST(test_config_save_to_nvs_callable_after_setters);
    RUN_TEST(test_view_manager_cycle_forward);
    RUN_TEST(test_view_manager_cycle_backward);
    RUN_TEST(test_view_manager_switch_to);
    UNITY_END();
    exit(0);
}

void loop() {}
