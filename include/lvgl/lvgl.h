#pragma once

#include "../../.pio/libdeps/native/lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void lv_style_set_margin_all(lv_style_t* style, int32_t value);
void lv_obj_set_name_static(lv_obj_t* obj, const char* name);

#ifdef __cplusplus
}
#endif
