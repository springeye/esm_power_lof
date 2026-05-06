#pragma once

#include "lvgl/lvgl.h"

namespace theme_manager {

enum ThemeVariant : uint8_t {
    Day   = 0,
    Night = 1,
};

struct ThemeColors {
    lv_color_t scr_bg;
    lv_color_t card_bg;
    lv_color_t card_border;
    lv_color_t text_primary;
    lv_color_t text_secondary;
    lv_color_t row_default_bg;
    lv_color_t row_focus_bg;
    lv_color_t row_edit_bg;
    lv_color_t row_default_border;
    lv_color_t row_focus_border;
    lv_color_t row_edit_border;
};

/** 从 config_manager 读取保存的偏好，设置当前主题 */
void theme_init();

/** 对指定屏幕根对象应用当前主题颜色（递归遍历子控件） */
void theme_apply(lv_obj_t* screen);

/** 切换 Day/Night，写入 config_manager，刷新活动屏幕 */
void theme_toggle();

/** 返回当前主题颜色（只读） */
const ThemeColors& theme_current_colors();

/** 返回当前主题类型 */
ThemeVariant theme_current_variant();

/** 对当前活动屏幕应用主题 */
void theme_apply_to_active_screen();

} // namespace theme_manager
