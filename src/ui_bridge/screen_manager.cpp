#include "screen_manager.h"
#include "splash_anim.h"
#include "settings_ui.h"
#include "lvgl/lvgl.h"

extern "C" lv_obj_t* splash_create(void);
extern "C" lv_obj_t* home_create(void);

namespace {
    lv_obj_t* g_home = nullptr;
    lv_obj_t* g_splash = nullptr;
    uint32_t g_splash_duration_ms = 1500;

    void splash_timer_cb(lv_timer_t* t) {
        lv_screen_load(g_home);
        if (g_splash) {
            lv_obj_delete(g_splash);
            g_splash = nullptr;
        }
        lv_timer_delete(t);
    }

    void async_splash_init(void*) {
        if (g_splash == nullptr) return;
        ui_bridge::splash_play_intro(g_splash);
        lv_timer_t* t = lv_timer_create(splash_timer_cb, g_splash_duration_ms, nullptr);
        lv_timer_set_repeat_count(t, 1);
    }
}

namespace ui_bridge {
    void screen_manager_init(uint32_t splash_duration_ms) {
        g_home = home_create();

        g_splash = splash_create();
        lv_screen_load(g_splash);
        g_splash_duration_ms = splash_duration_ms;

        // 延迟播放动画，等待 LVGL 任务启动
        lv_async_call(async_splash_init, nullptr);

        settings_ui::init();
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
