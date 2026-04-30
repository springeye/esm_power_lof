#pragma once

#include "../../.pio/libdeps/native/lvgl/lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

void lv_style_set_margin_all(lv_style_t* style, int32_t value);
void lv_obj_set_name_static(lv_obj_t* obj, const char* name);

/* LVGL 9.5.x APIs not present in 9.2.2 (native build compat) */
void lv_subject_init_float(lv_subject_t * subject, float value);
void lv_subject_set_float(lv_subject_t * subject, float value);
lv_observer_t * lv_bar_bind_value(lv_obj_t * obj, lv_subject_t * subject);

#ifdef __cplusplus
}
#endif
