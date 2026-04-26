#include "lvgl/lvgl.h"

extern "C" {

void lv_style_set_margin_all(lv_style_t * style, int32_t value) {
    lv_style_set_margin_top(style, value);
    lv_style_set_margin_bottom(style, value);
    lv_style_set_margin_left(style, value);
    lv_style_set_margin_right(style, value);
}

void lv_obj_set_name_static(lv_obj_t * obj, const char * name) {
    (void)obj;
    (void)name;
}

}
