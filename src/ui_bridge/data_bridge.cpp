#include "data_bridge.h"
#include "app/app_state.h"
#include "lvgl/lvgl.h"
#include <cmath>
#include <cstdio>

namespace {
    lv_obj_t* g_temp = nullptr;
    lv_obj_t* g_rpm  = nullptr;
    lv_obj_t* g_i[3] = {nullptr, nullptr, nullptr};

    // Widget 索引路径（来自 T10 home_gen.c 层级分析）
    // 温度 label: home->child(1)->child(1)
    // 转速 label: home->child(4)->child(0)->child(0)
    // CH1 电流 label: home->child(5)->child(0)->child(1)->child(0)
    // CH2 电流 label: home->child(5)->child(1)->child(1)->child(0)
    // CH3 电流 label: home->child(5)->child(2)->child(1)->child(0)

    void fmt(char* b, size_t n, float v, const char* unit) {
        if (std::isnan(v) || std::isinf(v)) {
            std::snprintf(b, n, "--%s", unit);
        } else {
            std::snprintf(b, n, "%.1f%s", v, unit);
        }
    }

    void refresh_cb(lv_timer_t*) {
        char buf[16];
        if (g_temp) {
            fmt(buf, sizeof(buf), app_state::get_temp_c(), " C");
            lv_label_set_text(g_temp, buf);
        }
        if (g_rpm) {
            fmt(buf, sizeof(buf), static_cast<float>(app_state::get_rpm()), " RPM");
            lv_label_set_text(g_rpm, buf);
        }
        float ch[3] = {
            app_state::get_ch1_a(),
            app_state::get_ch2_a(),
            app_state::get_ch3_a()
        };
        for (int i = 0; i < 3; ++i) {
            if (g_i[i]) {
                fmt(buf, sizeof(buf), ch[i], " A");
                lv_label_set_text(g_i[i], buf);
            }
        }
    }
}

namespace ui_bridge {
    void data_bridge_attach(lv_obj_t* home) {
        if (!home) return;
        // 温度 label: home->child(1)->child(1)
        lv_obj_t* obj1 = lv_obj_get_child(home, 1);
        if (obj1) g_temp = lv_obj_get_child(obj1, 1);

        // 转速 label: home->child(4)->child(0)->child(0)
        lv_obj_t* obj4 = lv_obj_get_child(home, 4);
        if (obj4) {
            lv_obj_t* obj8 = lv_obj_get_child(obj4, 0);
            if (obj8) g_rpm = lv_obj_get_child(obj8, 0);
        }

        // 电流区: home->child(5)
        lv_obj_t* obj10 = lv_obj_get_child(home, 5);
        if (obj10) {
            // CH1: child(0)->child(1)->child(0)
            lv_obj_t* ch1_area = lv_obj_get_child(obj10, 0);
            if (ch1_area) {
                lv_obj_t* ch1_vals = lv_obj_get_child(ch1_area, 1);
                if (ch1_vals) g_i[0] = lv_obj_get_child(ch1_vals, 0);
            }
            // CH2: child(1)->child(1)->child(0)
            lv_obj_t* ch2_area = lv_obj_get_child(obj10, 1);
            if (ch2_area) {
                lv_obj_t* ch2_vals = lv_obj_get_child(ch2_area, 1);
                if (ch2_vals) g_i[1] = lv_obj_get_child(ch2_vals, 0);
            }
            // CH3: child(2)->child(1)->child(0)
            lv_obj_t* ch3_area = lv_obj_get_child(obj10, 2);
            if (ch3_area) {
                lv_obj_t* ch3_vals = lv_obj_get_child(ch3_area, 1);
                if (ch3_vals) g_i[2] = lv_obj_get_child(ch3_vals, 0);
            }
        }
    }

    void data_bridge_init() {
        lv_timer_create(refresh_cb, 200, nullptr);
    }
}
