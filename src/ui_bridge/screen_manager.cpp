#include "screen_manager.h"
#include "splash_anim.h"
#include "settings_ui.h"
#include "theme_manager.h"
#include "lvgl/lvgl.h"

extern "C" lv_obj_t* splash_create(void);
extern "C" lv_obj_t* home_create(void);

namespace {
    lv_obj_t* g_home = nullptr;
    lv_obj_t* g_splash = nullptr;

    void splash_timer_cb(lv_timer_t* t) {
        lv_screen_load(g_home);
        if (g_splash) {
            lv_obj_delete(g_splash);
            g_splash = nullptr;
        }
        lv_timer_delete(t);
    }

    void test_settings_timer_cb(lv_timer_t* t) {
        ui_bridge::screen_manager_show_settings();
        lv_timer_delete(t);
    }
}

namespace ui_bridge {
    void screen_manager_init(uint32_t splash_duration_ms) {
        theme_manager::theme_init();

        g_home = home_create();
        theme_manager::theme_apply(g_home);

        g_splash = splash_create();
        theme_manager::theme_apply(g_splash);
        lv_screen_load(g_splash);

        ui_bridge::splash_play_intro(g_splash);
        lv_timer_t* t = lv_timer_create(splash_timer_cb, splash_duration_ms, nullptr);
        lv_timer_set_repeat_count(t, 1);

        settings_ui::init();

        lv_timer_t* test_t = lv_timer_create(test_settings_timer_cb, 3000, nullptr);
        lv_timer_set_repeat_count(test_t, 1);
    }

    lv_obj_t* screen_manager_get_home() {
        return g_home;
    }

    void screen_manager_show_settings() {
        settings_ui::show();
    }

    bool screen_manager_is_settings_active() {
        return settings_ui::is_active();
    }
}
