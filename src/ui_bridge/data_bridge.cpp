#include "data_bridge.h"
#include "power_history.h"
#include "chart_view.h"
#include "view_manager.h"
#include "../app/app_state.h"
#include "../app/config_manager.h"
#include "../power/psu_fsm.h"
#include "lvgl/lvgl.h"

extern "C" {
#include "../../ui/lof_power_system_gen.h"
}

#include <cstdio>

namespace {

// 开机时间戳 (ms)，首次 refresh 时记录
static uint32_t g_start_ms = 0;

void fmt_temp(char* buf, size_t n) {
    float t = app_state::get_temp_c();
    if (t < -50.0f || t > 200.0f) {
        std::snprintf(buf, n, "--C");
    } else {
        std::snprintf(buf, n, "%.1f℃", t);
    }
}

void fmt_voltage(char* buf, size_t n, uint16_t mv) {
    if (mv == 0) {
        std::snprintf(buf, n, "-- V");
    } else {
        std::snprintf(buf, n, "%.3fV", mv / 1000.0f);
    }
}

void fmt_current(char* buf, size_t n, float a) {
    if (a < 0.0f) {
        std::snprintf(buf, n, "-- A");
    } else {
        std::snprintf(buf, n, "%.3fA", a);
    }
}

void fmt_power(char* buf, size_t n, float w) {
    if (w < 0.0f) {
        std::snprintf(buf, n, "-- W");
    } else {
        std::snprintf(buf, n, "%.2fW", w);
    }
}

void refresh_cb(lv_timer_t*) {
    char buf[UI_SUBJECT_STRING_LENGTH];

    // ── 温度 ──
    fmt_temp(buf, sizeof(buf));
    lv_subject_copy_string(&device_temp, buf);

    // ── 运行状态 ──
    {
        uint8_t psu_st = app_state::psu_state_id.load();
        const char* state_text;
        if (app_state::is_fault()) {
            state_text = "故障";
        } else if (psu_st == PSU_ON) {
            state_text = "运行中";
        } else {
            state_text = "停止";
        }
        lv_subject_copy_string(&system_state, state_text);
    }

    // ── 开机时间 ──
    if (g_start_ms == 0) {
        g_start_ms = lv_tick_get();
    }
    uint32_t elapsed_s = (lv_tick_get() - g_start_ms) / 1000;
    uint32_t h = elapsed_s / 3600;
    uint32_t m = (elapsed_s % 3600) / 60;
    uint32_t s = elapsed_s % 60;
    std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
    lv_subject_copy_string(&uptime, buf);

    // ── 三通道电压/电流/功率 ──
    struct {
        lv_subject_t* voltage;
        lv_subject_t* current;
        lv_subject_t* power;
        float a;
        uint16_t mv;
    } ch[3] = {
        {&ch1_voltage, &ch1_current, &ch1_pwer,
         app_state::get_ch1_a(), app_state::get_ch1_mv()},
        {&ch2_voltage, &ch2_current, &ch2_pwer,
         app_state::get_ch2_a(), app_state::get_ch2_mv()},
        {&ch3_voltage, &ch3_current, &ch3_pwer,
         app_state::get_ch3_a(), app_state::get_ch3_mv()},
    };

    float total_w = 0.0f;
    for (int i = 0; i < 3; ++i) {
        fmt_voltage(buf, sizeof(buf), ch[i].mv);
        lv_subject_copy_string(ch[i].voltage, buf);

        fmt_current(buf, sizeof(buf), ch[i].a);
        lv_subject_copy_string(ch[i].current, buf);

        float ch_w = (ch[i].mv / 1000.0f) * ch[i].a;
        fmt_power(buf, sizeof(buf), ch_w);
        lv_subject_copy_string(ch[i].power, buf);

        power_history_push(i, lv_tick_get(), ch_w);

        total_w += ch_w;
    }

    HomeView current_view = view_manager::view_manager_get_current();
    if (current_view >= VIEW_CHART_CH1 && current_view <= VIEW_CHART_CH3) {
        chart_view::chart_view_update(current_view - VIEW_CHART_CH1);
    }

    // ── 总功率 ──
    std::snprintf(buf, sizeof(buf), "%.2fW", total_w);
    lv_subject_copy_string(&device_current_power, buf);

    // ── 功率百分比 ──
    uint16_t design_power_w = config_manager::get_design_power_w();
    int32_t percent = design_power_w > 0
        ? static_cast<int32_t>(total_w * 100 / static_cast<float>(design_power_w))
        : 0;
    lv_subject_set_int(&device_power_percent, percent);

    std::snprintf(buf, sizeof(buf), "%d%%", percent);
    lv_subject_copy_string(&device_power_percent_txt, buf);

    // ── 设计功率 ──
    std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(design_power_w));
    lv_subject_copy_string(&device_power, buf);

    // ── 瓦时累计 ──
    float wh_val = total_w * elapsed_s / 3600.0f;
    std::snprintf(buf, sizeof(buf), "%.2fWh", wh_val);
    lv_subject_copy_string(&wh, buf);

    // ── 风扇转速百分比 ──
    uint32_t rpm = app_state::get_rpm();
    int fan_pct = 0;
    if (rpm > 0) {
        // 假设最大转速 ~3000 RPM（可根据实际风扇参数调整）
        fan_pct = static_cast<int>(rpm * 100 / 3000);
        if (fan_pct > 100) fan_pct = 100;
    }
    std::snprintf(buf, sizeof(buf), "%d%%", fan_pct);
    lv_subject_copy_string(&fan_percent, buf);

    // ── 风扇转速 RPM ──
    std::snprintf(buf, sizeof(buf), "%lu RPM", (unsigned long)rpm);
    lv_subject_copy_string(&fan_rpm_txt, buf);

    // ── OTA 进度覆盖显示 ──
    {
        static lv_obj_t* s_ota_label = nullptr;
        int8_t ota_prog = app_state::get_ota_progress();
        if (ota_prog > 0) {
            if (s_ota_label == nullptr) {
                s_ota_label = lv_label_create(lv_layer_top());
                lv_obj_set_style_text_color(s_ota_label, lv_color_hex(0xFFFFFF), 0);
                lv_obj_set_style_text_font(s_ota_label, hos_bold_big, 0);
                lv_obj_center(s_ota_label);
            }
            char buf[32];
            std::snprintf(buf, sizeof(buf), "OTA %d%%", (int)ota_prog);
            lv_label_set_text(s_ota_label, buf);
        } else if (s_ota_label != nullptr) {
            lv_obj_delete(s_ota_label);
            s_ota_label = nullptr;
        }
    }
}

} // namespace

namespace ui_bridge {

void data_bridge_attach(lv_obj_t*) {
}

void data_bridge_init() {
    power_history_init();
    lv_timer_create(refresh_cb, 200, nullptr);
}

} // namespace ui_bridge
