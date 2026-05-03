#include "screen_manager.h"
#include "splash_anim.h"
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
}

namespace ui_bridge {
    void screen_manager_init(uint32_t splash_duration_ms) {
        g_home = home_create();

        g_splash = splash_create();
        lv_screen_load(g_splash);
        ui_bridge::splash_play_intro(g_splash);

        lv_timer_t* t = lv_timer_create(splash_timer_cb, splash_duration_ms, nullptr);
        lv_timer_set_repeat_count(t, 1);
    }

    lv_obj_t* screen_manager_get_home() {
        return g_home;
    }
}
