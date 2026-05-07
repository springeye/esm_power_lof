#include "view_manager.h"
#include "../app/config_manager.h"
#include <lvgl.h>
#include <atomic>

namespace {

lv_obj_t* g_home = nullptr;
lv_obj_t* g_channels = nullptr;
lv_obj_t* g_chart_containers[3] = {nullptr, nullptr, nullptr};
std::atomic<uint8_t> g_current_view{VIEW_DEFAULT};

lv_obj_t* ensure_chart_container(uint8_t ch) {
    if (ch >= 3) return nullptr;
    if (g_chart_containers[ch] != nullptr) return g_chart_containers[ch];
    if (g_home == nullptr) return nullptr;

    lv_obj_t* c = lv_obj_create(g_home);
    lv_obj_set_width(c, lv_pct(100));
    lv_obj_set_height(c, lv_pct(0));
    lv_obj_set_style_flex_grow(c, 1, 0);
    lv_obj_set_style_layout(c, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(c, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(c, 0, 0);
    lv_obj_set_style_border_width(c, 0, 0);
    lv_obj_set_style_pad_all(c, 0, 0);
    lv_obj_set_style_pad_row(c, 0, 0);
    lv_obj_add_flag(c, LV_OBJ_FLAG_HIDDEN);

    g_chart_containers[ch] = c;
    return c;
}

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
    g_current_view.store(VIEW_DEFAULT);

    uint8_t default_view = config_manager::get_default_view();
    if (default_view >= 1 && default_view <= 3) {
        view_manager_switch_to(static_cast<HomeView>(default_view));
    }
}

void view_manager_switch_to(HomeView view) {
    if (view >= VIEW_COUNT) return;
    g_current_view.store(view);

    if (view >= VIEW_CHART_CH1 && view <= VIEW_CHART_CH3) {
        uint8_t ch = view - VIEW_CHART_CH1;
        ensure_chart_container(ch);
    }

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
