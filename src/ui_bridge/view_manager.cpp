#include "view_manager.h"
#include "power_history.h"
#include "../app/config_manager.h"
#include <lvgl.h>
#include <atomic>

namespace {

lv_obj_t* g_home = nullptr;
lv_obj_t* g_channels = nullptr;
lv_obj_t* g_chart_containers[3] = {nullptr, nullptr, nullptr};
std::atomic<uint8_t> g_current_view{VIEW_DEFAULT};

void show_view(HomeView view) {
    if (g_channels) {
        if (view == VIEW_DEFAULT) {
            lv_obj_clear_flag(g_channels, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_channels, LV_OBJ_FLAG_HIDDEN);
        }
    }

    for (int i = 0; i < 3; ++i) {
        if (g_chart_containers[i]) {
            if (view == static_cast<HomeView>(VIEW_CHART_CH1 + i)) {
                lv_obj_clear_flag(g_chart_containers[i], LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_chart_containers[i], LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

} // namespace

namespace view_manager {

void view_manager_init(lv_obj_t* home_root) {
    g_home = home_root;
    g_channels = lv_obj_get_child(g_home, 4);

    for (int i = 0; i < 3; ++i) {
        g_chart_containers[i] = lv_obj_create(g_home);
        lv_obj_set_width(g_chart_containers[i], lv_pct(100));
        lv_obj_set_height(g_chart_containers[i], lv_pct(0));
        lv_obj_set_style_flex_grow(g_chart_containers[i], 1, 0);
        lv_obj_set_style_layout(g_chart_containers[i], LV_LAYOUT_FLEX, 0);
        lv_obj_set_flex_flow(g_chart_containers[i], LV_FLEX_FLOW_COLUMN);
        lv_obj_set_style_bg_opa(g_chart_containers[i], 0, 0);
        lv_obj_set_style_border_width(g_chart_containers[i], 0, 0);
        lv_obj_set_style_pad_all(g_chart_containers[i], 0, 0);
        lv_obj_set_style_pad_row(g_chart_containers[i], 0, 0);
        lv_obj_add_flag(g_chart_containers[i], LV_OBJ_FLAG_HIDDEN);
    }

    g_current_view.store(VIEW_DEFAULT);

    uint8_t default_view = config_manager::get_default_view();
    if (default_view >= 1 && default_view <= 3) {
        view_manager_switch_to(static_cast<HomeView>(default_view));
    }
}

void view_manager_switch_to(HomeView view) {
    if (view >= VIEW_COUNT) return;
    g_current_view.store(view);
    show_view(view);
}

HomeView view_manager_get_current() {
    return static_cast<HomeView>(g_current_view.load());
}

void view_manager_cycle(int8_t delta) {
    uint8_t cur = g_current_view.load();
    uint8_t next = static_cast<uint8_t>((static_cast<int>(cur) + delta + VIEW_COUNT) % VIEW_COUNT);
    view_manager_switch_to(static_cast<HomeView>(next));
}

lv_obj_t* view_manager_get_chart_container(uint8_t ch) {
    if (ch >= 3) return nullptr;
    return g_chart_containers[ch];
}

lv_obj_t* view_manager_get_channels_container() {
    return g_channels;
}

} // namespace view_manager
