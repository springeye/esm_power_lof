#if 0  // legacy UI - replaced by ui/ subproject
/**
 * @file ui_menu.cpp
 * @brief 设置菜单界面实现
 *
 * 菜单项（静态）：
 *  [0] Fan Curve: 30C->20% 50C->60% 70C->100%
 *  [1] Hysteresis: 2.0C
 *  [2] WDT Timeout: 5s
 *  [3] Back
 */

#include "ui_menu.h"
#include "app_config.h"
#include <lvgl.h>
#include <cstdio>

static constexpr int MENU_ITEMS = 4;

static lv_obj_t* s_screen      = nullptr;
static lv_obj_t* s_list        = nullptr;
static int        s_selected    = 0;

static const char* const MENU_LABELS[MENU_ITEMS] = {
    "Fan: 30C->20% 50C->60% 70C->100%",
    "Hysteresis: 2.0C",
    "WDT: 5s",
    "< Back"
};

static void highlight_item(int idx) {
    for (int i = 0; i < MENU_ITEMS; ++i) {
        lv_obj_t* btn = lv_obj_get_child(s_list, i);
        if (!btn) continue;
        if (i == idx) {
            lv_obj_set_style_bg_color(btn, lv_color_make(0x00, 0x80, 0xFF), 0);
        } else {
            lv_obj_set_style_bg_color(btn, lv_color_black(), 0);
        }
    }
}

namespace ui_menu {

void init() {
    s_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(s_screen, lv_color_black(), 0);
    lv_obj_set_size(s_screen, 240, 280);

    s_list = lv_obj_create(s_screen);
    lv_obj_set_size(s_list, 230, 260);
    lv_obj_set_pos(s_list, 5, 10);
    lv_obj_set_style_bg_color(s_list, lv_color_black(), 0);
    lv_obj_set_flex_flow(s_list, LV_FLEX_FLOW_COLUMN);

    for (int i = 0; i < MENU_ITEMS; ++i) {
        lv_obj_t* btn = lv_obj_create(s_list);
        lv_obj_set_size(btn, 220, 55);
        lv_obj_set_style_bg_color(btn, lv_color_black(), 0);

        lv_obj_t* lbl = lv_label_create(btn);
        lv_label_set_text(lbl, MENU_LABELS[i]);
        lv_obj_set_style_text_color(lbl, lv_color_white(), 0);
        lv_obj_center(lbl);
    }

    s_selected = 0;
    highlight_item(s_selected);
}

void navigate_up() {
    if (s_selected > 0) {
        --s_selected;
        highlight_item(s_selected);
    }
}

void navigate_down() {
    if (s_selected < MENU_ITEMS - 1) {
        ++s_selected;
        highlight_item(s_selected);
    }
}

void confirm() {
    // "Back" 项由 ui_events 处理屏幕切换
    // 其他项目当前为只读展示，无操作
}

lv_obj_t* get_screen() {
    return s_screen;
}

} // namespace ui_menu
#endif // legacy UI
