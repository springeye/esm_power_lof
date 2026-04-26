#pragma once
#include "lvgl/lvgl.h"

namespace ui_bridge {
/** Show splash, then auto-load home after splash_duration_ms. */
void screen_manager_init(uint32_t splash_duration_ms = 2000);
/** Returns the current home screen object (NULL if not yet loaded). */
lv_obj_t* screen_manager_get_home();
}
