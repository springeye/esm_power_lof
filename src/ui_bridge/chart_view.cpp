#include "chart_view.h"
#include "view_manager.h"
#include "power_history.h"
#include "../app/config_manager.h"
#include "../app/app_state.h"
#include <lvgl.h>
#include <cstdio>

extern lv_font_t* hos_14;
extern lv_font_t* hos_regular;

namespace {

static constexpr uint32_t WINDOW_1MIN_MS = 60000;
static constexpr uint32_t WINDOW_5MIN_MS = 300000;
static constexpr uint32_t WINDOW_10MIN_MS = 600000;
static constexpr uint32_t WINDOWS[] = {WINDOW_1MIN_MS, WINDOW_5MIN_MS, WINDOW_10MIN_MS};
static constexpr uint8_t WINDOW_COUNT = 3;
static constexpr uint16_t CHART_POINT_COUNT = 240;

static const lv_color_t CH_COLORS[3] = {
    lv_color_hex(0x00e68a),
    lv_color_hex(0xffb020),
    lv_color_hex(0x3d9eff)
};

struct ChartState {
    lv_obj_t* chart;
    lv_chart_series_t* series;
    lv_obj_t* title_label;
    lv_obj_t* voltage_label;
    lv_obj_t* current_label;
    lv_obj_t* power_label;
};

static ChartState g_charts[3] = {};
static uint8_t g_window_index = 0;
static bool g_initialized = false;

void create_chart_ui(uint8_t ch) {
    lv_obj_t* container = view_manager::view_manager_get_chart_container(ch);
    if (!container) return;

    lv_obj_t* title_row = lv_obj_create(container);
    lv_obj_set_width(title_row, lv_pct(100));
    lv_obj_set_height(title_row, 24);
    lv_obj_set_style_layout(title_row, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(title_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(title_row, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(title_row, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(title_row, 0, 0);
    lv_obj_set_style_border_width(title_row, 2, 0);
    lv_obj_set_style_border_color(title_row, CH_COLORS[ch], 0);
    lv_obj_set_style_border_side(title_row, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_pad_left(title_row, 4, 0);
    lv_obj_set_style_pad_right(title_row, 4, 0);
    lv_obj_set_style_pad_top(title_row, 0, 0);
    lv_obj_set_style_pad_bottom(title_row, 0, 0);

    char ch_name[8];
    std::snprintf(ch_name, sizeof(ch_name), "CH%u", ch + 1);
    g_charts[ch].title_label = lv_label_create(title_row);
    lv_label_set_text(g_charts[ch].title_label, ch_name);
    lv_obj_set_style_text_color(g_charts[ch].title_label, CH_COLORS[ch], 0);
    lv_obj_set_style_text_font(g_charts[ch].title_label, hos_14, 0);

    g_charts[ch].voltage_label = lv_label_create(title_row);
    lv_label_set_text(g_charts[ch].voltage_label, "-- V");
    lv_obj_set_style_text_color(g_charts[ch].voltage_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(g_charts[ch].voltage_label, hos_14, 0);

    g_charts[ch].current_label = lv_label_create(title_row);
    lv_label_set_text(g_charts[ch].current_label, "-- A");
    lv_obj_set_style_text_color(g_charts[ch].current_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(g_charts[ch].current_label, hos_14, 0);

    g_charts[ch].power_label = lv_label_create(title_row);
    lv_label_set_text(g_charts[ch].power_label, "-- W");
    lv_obj_set_style_text_color(g_charts[ch].power_label, CH_COLORS[ch], 0);
    lv_obj_set_style_text_font(g_charts[ch].power_label, hos_14, 0);

    g_charts[ch].chart = lv_chart_create(container);
    lv_obj_set_width(g_charts[ch].chart, lv_pct(100));
    lv_obj_set_style_flex_grow(g_charts[ch].chart, 1, 0);
    lv_chart_set_type(g_charts[ch].chart, LV_CHART_TYPE_LINE);
    lv_chart_set_update_mode(g_charts[ch].chart, LV_CHART_UPDATE_MODE_SHIFT);
    lv_chart_set_point_count(g_charts[ch].chart, CHART_POINT_COUNT);
    lv_chart_set_div_line_count(g_charts[ch].chart, 3, 4);
    lv_obj_set_style_bg_color(g_charts[ch].chart, lv_color_hex(0x111820), 0);
    lv_obj_set_style_bg_opa(g_charts[ch].chart, 255, 0);
    lv_obj_set_style_border_width(g_charts[ch].chart, 1, 0);
    lv_obj_set_style_border_color(g_charts[ch].chart, lv_color_hex(0x1a2533), 0);
    lv_obj_set_style_line_color(g_charts[ch].chart, lv_color_hex(0x2a3545), 0);
    lv_obj_set_style_line_width(g_charts[ch].chart, 1, 0);
    lv_obj_set_style_size(g_charts[ch].chart, 0, 0, LV_PART_INDICATOR);

    g_charts[ch].series = lv_chart_add_series(g_charts[ch].chart, CH_COLORS[ch], LV_CHART_AXIS_PRIMARY_Y);
    lv_chart_set_all_values(g_charts[ch].chart, g_charts[ch].series, LV_CHART_POINT_NONE);
}

void update_title_labels(uint8_t ch) {
    if (!g_charts[ch].voltage_label) return;

    char buf[16];
    uint16_t mv = 0;
    float a = 0.0f;

    switch (ch) {
        case 0: mv = app_state::get_ch1_mv(); a = app_state::get_ch1_a(); break;
        case 1: mv = app_state::get_ch2_mv(); a = app_state::get_ch2_a(); break;
        case 2: mv = app_state::get_ch3_mv(); a = app_state::get_ch3_a(); break;
        default: return;
    }

    if (mv == 0) {
        lv_label_set_text(g_charts[ch].voltage_label, "-- V");
    } else {
        std::snprintf(buf, sizeof(buf), "%.3fV", mv / 1000.0f);
        lv_label_set_text(g_charts[ch].voltage_label, buf);
    }

    if (a < 0.0f) {
        lv_label_set_text(g_charts[ch].current_label, "-- A");
    } else {
        std::snprintf(buf, sizeof(buf), "%.3fA", a);
        lv_label_set_text(g_charts[ch].current_label, buf);
    }

    float w = (mv / 1000.0f) * a;
    if (w < 0.0f) {
        lv_label_set_text(g_charts[ch].power_label, "-- W");
    } else {
        std::snprintf(buf, sizeof(buf), "%.2fW", w);
        lv_label_set_text(g_charts[ch].power_label, buf);
    }
}

void update_chart_data(uint8_t ch) {
    if (!g_charts[ch].chart || !g_charts[ch].series) return;

    uint32_t window_ms = WINDOWS[g_window_index];
    PowerHistorySample points[CHART_POINT_COUNT];
    uint32_t count = power_history_sample_window(
        ch,
        window_ms,
        CHART_POINT_COUNT,
        points,
        CHART_POINT_COUNT);

    if (count == 0) {
        lv_chart_set_all_values(g_charts[ch].chart, g_charts[ch].series, LV_CHART_POINT_NONE);
        return;
    }

    lv_chart_set_point_count(g_charts[ch].chart, count);

    uint8_t yaxis_mode = config_manager::get_chart_yaxis_mode();
    if (yaxis_mode == 1) {
        uint16_t design_w = config_manager::get_design_power_w();
        lv_chart_set_axis_range(g_charts[ch].chart, LV_CHART_AXIS_PRIMARY_Y, 0, design_w);
    }

    for (uint32_t i = 0; i < count; ++i) {
        int32_t val = static_cast<int32_t>(points[i].power_w);
        lv_chart_set_series_value_by_id(g_charts[ch].chart, g_charts[ch].series, i, val);
    }

    lv_chart_refresh(g_charts[ch].chart);
}

} // namespace

namespace chart_view {

void chart_view_update(uint8_t ch) {
    if (ch >= 3) return;

    // 懒创建：首次访问时才建 chart，一次只建一个
    if (g_charts[ch].chart == nullptr) {
        lv_obj_t* container = view_manager::view_manager_get_chart_container(ch);
        if (!container) return;
        create_chart_ui(ch);
    }

    update_title_labels(ch);
    update_chart_data(ch);
}

void chart_view_init() {
    if (g_initialized) return;
    g_window_index = 0;
    g_initialized = true;
}

void chart_view_set_window(uint8_t ch, uint32_t window_ms) {
    (void)ch;
    for (uint8_t i = 0; i < WINDOW_COUNT; ++i) {
        if (WINDOWS[i] == window_ms) {
            g_window_index = i;
            break;
        }
    }
}

void chart_view_cycle_window() {
    g_window_index = (g_window_index + 1) % WINDOW_COUNT;
}

} // namespace chart_view
