/**
 * @file ui_events.cpp
 * @brief 按键事件路由实现
 *
 * 屏幕切换逻辑：
 *  - 主仪表盘：KEY_K2 长按 → 切换到菜单
 *  - 菜单：KEY_K1/K3 导航，KEY_K2 确认，
 *           选中 "Back" 并 KEY_K2 → 返回主仪表盘
 */

#include "ui_events.h"
#include "ui_main.h"
#include "ui_menu.h"
#include "pins.h"
#include <lvgl.h>

namespace {
    enum class Screen { MAIN, MENU };
    Screen s_active = Screen::MAIN;
}

namespace ui_events {

void init() {
    s_active = Screen::MAIN;
}

void handle_key(uint8_t key, KeyState state) {
    if (s_active == Screen::MAIN) {
        // 主仪表盘：ENTER 长按进入菜单
        if (key == KEY_K2 && state.event == KEY_LONG) {
            s_active = Screen::MENU;
            lv_scr_load(ui_menu::get_screen());
        }
    } else {
        // 菜单界面
        if (state.event == KEY_SHORT) {
            if (key == KEY_K1) {
                ui_menu::navigate_up();
            } else if (key == KEY_K3) {
                ui_menu::navigate_down();
            } else if (key == KEY_K2) {
                ui_menu::confirm();
                // 简化：任何 ENTER 短按均返回主界面
                s_active = Screen::MAIN;
                lv_scr_load(ui_main::get_screen());
            }
        }
    }
}

} // namespace ui_events
