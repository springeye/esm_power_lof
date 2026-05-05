#pragma once
#include "lvgl/lvgl.h"

namespace ui_bridge {
/** Show splash, then auto-load home after splash_duration_ms. */
void screen_manager_init(uint32_t splash_duration_ms = 1500);
/** Returns the current home screen object (NULL if not yet loaded). */
lv_obj_t* screen_manager_get_home();
/** Show the settings page. */
void screen_manager_show_settings();
/** Returns true if the settings page is currently active. */
bool screen_manager_is_settings_active();
}
