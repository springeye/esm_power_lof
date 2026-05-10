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

#include <cmath>
#include <cstdio>

namespace {

static uint32_t g_start_ms = 0;
static uint32_t g_last_low_freq_ms = 0;
static uint32_t g_last_chart_refresh_ms = 0;
static constexpr uint32_t LOW_FREQ_INTERVAL_MS = 1000;
static constexpr uint32_t CHART_REFRESH_INTERVAL_MS = 500;

struct UiSnapshot {
    bool initialized;
    float temp_c;
    uint16_t ch_mv[3];
    float ch_a[3];
    float ch_w[3];
    float total_w;
    uint32_t rpm;
    int32_t power_percent;
    uint16_t design_power_w;
    uint8_t psu_state;
    bool fault;
};

static UiSnapshot g_snapshot = {};

bool float_changed(float a, float b, float threshold) {
    return std::fabs(a - b) >= threshold;
}

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
    uint32_t now_ms = lv_tick_get();

    float temp_c = app_state::get_temp_c();
    uint16_t ch_mv[3] = {
        app_state::get_ch1_mv(),
        app_state::get_ch2_mv(),
        app_state::get_ch3_mv()
    };
    float ch_a[3] = {
        app_state::get_ch1_a(),
        app_state::get_ch2_a(),
        app_state::get_ch3_a()
    };
    float ch_w[3];
    float total_w = 0.0f;
    for (int i = 0; i < 3; ++i) {
        ch_w[i] = (ch_mv[i] / 1000.0f) * ch_a[i];
        total_w += ch_w[i];
    }
    uint32_t rpm = app_state::get_rpm();
    uint8_t psu_st = app_state::psu_state_id.load();
    bool fault = app_state::is_fault();
    uint16_t design_power_w = config_manager::get_design_power_w();
    int32_t percent = design_power_w > 0
        ? static_cast<int32_t>(total_w * 100 / static_cast<float>(design_power_w))
        : 0;

    bool force_update = !g_snapshot.initialized;

    if (g_start_ms == 0) {
        g_start_ms = now_ms;
    }

    if (!g_snapshot.initialized) {
        g_snapshot.temp_c = temp_c;
        for (int i = 0; i < 3; ++i) {
            g_snapshot.ch_mv[i] = ch_mv[i];
            g_snapshot.ch_a[i] = ch_a[i];
            g_snapshot.ch_w[i] = ch_w[i];
        }
        g_snapshot.total_w = total_w;
        g_snapshot.rpm = rpm;
        g_snapshot.power_percent = percent;
        g_snapshot.design_power_w = design_power_w;
        g_snapshot.psu_state = psu_st;
        g_snapshot.fault = fault;
        g_snapshot.initialized = true;
    }

    bool temp_changed = float_changed(temp_c, g_snapshot.temp_c, 0.1f);
    bool mv_changed[3], a_changed[3], w_changed[3];
    bool any_channel_changed = false;
    for (int i = 0; i < 3; ++i) {
        mv_changed[i] = (ch_mv[i] != g_snapshot.ch_mv[i]);
        a_changed[i] = float_changed(ch_a[i], g_snapshot.ch_a[i], 0.001f);
        w_changed[i] = float_changed(ch_w[i], g_snapshot.ch_w[i], 0.001f);
        if (mv_changed[i] || a_changed[i] || w_changed[i]) any_channel_changed = true;
    }
    bool total_changed = float_changed(total_w, g_snapshot.total_w, 0.01f);
    bool rpm_changed = (rpm != g_snapshot.rpm);
    bool percent_changed = (percent != g_snapshot.power_percent);
    bool design_changed = (design_power_w != g_snapshot.design_power_w);
    bool state_changed = (psu_st != g_snapshot.psu_state) || (fault != g_snapshot.fault);

    if (force_update || temp_changed) {
        fmt_temp(buf, sizeof(buf));
        lv_subject_copy_string(&device_temp, buf);
        g_snapshot.temp_c = temp_c;
    }

    if (force_update || state_changed) {
        const char* state_text;
        if (fault) {
            state_text = "故障";
        } else if (psu_st == PSU_ON) {
            state_text = "运行中";
        } else {
            state_text = "停止";
        }
        lv_subject_copy_string(&system_state, state_text);
        g_snapshot.psu_state = psu_st;
        g_snapshot.fault = fault;
    }

    bool low_freq_due = (now_ms - g_last_low_freq_ms) >= LOW_FREQ_INTERVAL_MS;
    if (force_update || low_freq_due) {
        uint32_t elapsed_s = (now_ms - g_start_ms) / 1000;
        uint32_t h = elapsed_s / 3600;
        uint32_t m = (elapsed_s % 3600) / 60;
        uint32_t s = elapsed_s % 60;
        std::snprintf(buf, sizeof(buf), "%02d:%02d:%02d", h, m, s);
        lv_subject_copy_string(&uptime, buf);

        float wh_val = total_w * elapsed_s / 3600.0f;
        std::snprintf(buf, sizeof(buf), "%.2fWh", wh_val);
        lv_subject_copy_string(&wh, buf);

        g_last_low_freq_ms = now_ms;
    }

    for (int i = 0; i < 3; ++i) {
        lv_subject_t* voltage_subj = (i == 0) ? &ch1_voltage : (i == 1) ? &ch2_voltage : &ch3_voltage;
        lv_subject_t* current_subj = (i == 0) ? &ch1_current : (i == 1) ? &ch2_current : &ch3_current;
        lv_subject_t* power_subj = (i == 0) ? &ch1_pwer : (i == 1) ? &ch2_pwer : &ch3_pwer;

        if (force_update || mv_changed[i]) {
            fmt_voltage(buf, sizeof(buf), ch_mv[i]);
            lv_subject_copy_string(voltage_subj, buf);
            g_snapshot.ch_mv[i] = ch_mv[i];
        }
        if (force_update || a_changed[i]) {
            fmt_current(buf, sizeof(buf), ch_a[i]);
            lv_subject_copy_string(current_subj, buf);
            g_snapshot.ch_a[i] = ch_a[i];
        }
        if (force_update || w_changed[i]) {
            fmt_power(buf, sizeof(buf), ch_w[i]);
            lv_subject_copy_string(power_subj, buf);
            g_snapshot.ch_w[i] = ch_w[i];
        }

        power_history_push(i, now_ms, ch_w[i]);
    }

    if (force_update || total_changed) {
        std::snprintf(buf, sizeof(buf), "%.2fW", total_w);
        lv_subject_copy_string(&device_current_power, buf);
        g_snapshot.total_w = total_w;
    }

    if (force_update || percent_changed || design_changed) {
        lv_subject_set_int(&device_power_percent, percent);
        std::snprintf(buf, sizeof(buf), "%d%%", percent);
        lv_subject_copy_string(&device_power_percent_txt, buf);
        g_snapshot.power_percent = percent;
    }

    if (force_update || design_changed) {
        std::snprintf(buf, sizeof(buf), "%u", static_cast<unsigned>(design_power_w));
        lv_subject_copy_string(&device_power, buf);
        g_snapshot.design_power_w = design_power_w;
    }

    if (force_update || rpm_changed) {
        int fan_pct = 0;
        if (rpm > 0) {
            fan_pct = static_cast<int>(rpm * 100 / 3000);
            if (fan_pct > 100) fan_pct = 100;
        }
        std::snprintf(buf, sizeof(buf), "%d%%", fan_pct);
        lv_subject_copy_string(&fan_percent, buf);

        std::snprintf(buf, sizeof(buf), "%lu RPM", (unsigned long)rpm);
        lv_subject_copy_string(&fan_rpm_txt, buf);
        g_snapshot.rpm = rpm;
    }

    HomeView current_view = view_manager::view_manager_get_current();
    if (current_view >= VIEW_CHART_CH1 && current_view <= VIEW_CHART_CH3) {
        bool chart_due = (now_ms - g_last_chart_refresh_ms) >= CHART_REFRESH_INTERVAL_MS;
        if (force_update || any_channel_changed || chart_due) {
            chart_view::chart_view_update(current_view - VIEW_CHART_CH1);
            g_last_chart_refresh_ms = now_ms;
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
