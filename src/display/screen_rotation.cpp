#include "screen_rotation.h"
#include "tft_driver.h"
#include "lvgl_port.h"
#include "app/config_manager.h"
#include <lvgl.h>

static ScreenRotation s_current = ROT_0;
static lv_display_t* s_disp = nullptr;

static lv_display_rotation_t to_lvgl_rotation(ScreenRotation rot) {
    switch (rot) {
        case ROT_0:   return LV_DISPLAY_ROTATION_0;
        case ROT_90:  return LV_DISPLAY_ROTATION_90;
        case ROT_180: return LV_DISPLAY_ROTATION_180;
        case ROT_270: return LV_DISPLAY_ROTATION_270;
        default:      return LV_DISPLAY_ROTATION_0;
    }
}

namespace screen_rotation {

void init() {
    s_disp = lv_display_get_default();
    uint8_t saved = config_manager::get_screen_rotation();
    if (saved <= 3) {
        s_current = static_cast<ScreenRotation>(saved);
        tft_driver::get_tft().setRotation(s_current);
        if (s_disp) {
            lv_display_set_rotation(s_disp, to_lvgl_rotation(s_current));
        }
    } else {
        s_current = ROT_0;
    }
}

void rotate_cw() {
    ScreenRotation next = static_cast<ScreenRotation>((s_current + 1) % 4);
    set_rotation(next);
}

void rotate_ccw() {
    ScreenRotation next = static_cast<ScreenRotation>((s_current + 3) % 4);
    set_rotation(next);
}

ScreenRotation get_current() {
    return s_current;
}

void set_rotation(ScreenRotation rot) {
    if (rot == s_current) return;

    tft_driver::get_tft().setRotation(rot);
    if (s_disp) {
        lv_display_set_rotation(s_disp, to_lvgl_rotation(rot));
        lv_obj_invalidate(lv_screen_active());
    }
    s_current = rot;
    config_manager::set_screen_rotation(static_cast<uint8_t>(rot));
}

} // namespace screen_rotation
