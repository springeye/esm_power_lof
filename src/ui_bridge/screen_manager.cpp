#include "screen_manager.h"
#include "splash_anim.h"
#include "settings_ui.h"
#include "lvgl/lvgl.h"
#include <Arduino.h>

extern "C" lv_obj_t* splash_create(void);
extern "C" lv_obj_t* home_create(void);

namespace {
    lv_obj_t* g_home = nullptr;
    lv_obj_t* g_splash = nullptr;
    uint32_t g_splash_duration_ms = 1500;

    void splash_timer_cb(lv_timer_t* t) {
        Serial.println("[Splash] Timer fired, loading home screen");
        lv_screen_load(g_home);
        if (g_splash) {
            lv_obj_delete(g_splash);
            g_splash = nullptr;
        }
        lv_timer_delete(t);
    }

    void async_splash_init(void*) {
        Serial.println("[Splash] async_splash_init called");
        if (g_splash == nullptr) {
            Serial.println("[Splash] ERROR: g_splash is null");
            return;
        }
        Serial.println("[Splash] Calling splash_play_intro");
        ui_bridge::splash_play_intro(g_splash);
        Serial.println("[Splash] Creating splash timer");
        lv_timer_t* t = lv_timer_create(splash_timer_cb, g_splash_duration_ms, nullptr);
        lv_timer_set_repeat_count(t, 1);
        Serial.println("[Splash] Timer created");
    }
}

namespace ui_bridge {
    void screen_manager_init(uint32_t splash_duration_ms) {
        Serial.println("[Splash] screen_manager_init start");
        g_home = home_create();
        Serial.println("[Splash] home_create done");

        g_splash = splash_create();
        Serial.println("[Splash] splash_create done");
        lv_screen_load(g_splash);
        Serial.println("[Splash] splash screen loaded");
        g_splash_duration_ms = splash_duration_ms;

        Serial.println("[Splash] Calling lv_async_call");
        lv_async_call(async_splash_init, nullptr);
        Serial.println("[Splash] lv_async_call returned");

        settings_ui::init();
        Serial.println("[Splash] screen_manager_init done");
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
