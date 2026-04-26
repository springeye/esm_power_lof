#if 0  // legacy UI - replaced by ui/ subproject
/**
 * @file ui_main.cpp
 * @brief 主仪表盘界面实现
 *
 * 布局（240×280 竖屏）：
 *  ┌─────────────────────────┐
 *  │  TEMP: 45.2°C           │  行1 (y=10)
 *  │  FAN:  1200 RPM         │  行2 (y=50)
 *  │  PSU:  ON               │  行3 (y=90)
 *  │─────────────────────────│
 *  │  LOAD: 12.5A            │  行4 (y=140)
 *  │  12V:   3.2A            │  行5 (y=175)
 *  │   5V:   1.8A            │  行6 (y=210)
 *  └─────────────────────────┘
 *
 * 样式：黑底白字，LV_FONT_MONTSERRAT_20（需在 lv_conf.h 启用）
 */

#include "ui_main.h"
#include <lvgl.h>
#include <cstdio>

// ── 全局样式 ─────────────────────────────────────────────────────────────────
static lv_style_t s_style_label;
static bool       s_style_inited = false;

// ── 屏幕和标签对象 ───────────────────────────────────────────────────────────
static lv_obj_t* s_screen    = nullptr;
static lv_obj_t* s_lbl_temp  = nullptr;
static lv_obj_t* s_lbl_fan   = nullptr;
static lv_obj_t* s_lbl_psu   = nullptr;
static lv_obj_t* s_lbl_load  = nullptr;
static lv_obj_t* s_lbl_12v   = nullptr;
static lv_obj_t* s_lbl_5v    = nullptr;

// ── 内部辅助 ─────────────────────────────────────────────────────────────────
static void init_styles() {
    if (s_style_inited) return;
    lv_style_init(&s_style_label);
    lv_style_set_text_color(&s_style_label, lv_color_white());
    lv_style_set_bg_color(&s_style_label, lv_color_black());
    s_style_inited = true;
}

static lv_obj_t* create_label(lv_obj_t* parent, int32_t x, int32_t y,
                               const char* init_text) {
    lv_obj_t* lbl = lv_label_create(parent);
    lv_obj_add_style(lbl, &s_style_label, 0);
    lv_obj_set_pos(lbl, x, y);
    lv_label_set_text(lbl, init_text);
    return lbl;
}

namespace ui_main {

void init() {
    init_styles();

    // 创建屏幕
    s_screen = lv_obj_create(nullptr);
    lv_obj_set_style_bg_color(s_screen, lv_color_black(), 0);
    lv_obj_set_size(s_screen, 240, 280);

    // 分隔线
    lv_obj_t* line = lv_obj_create(s_screen);
    lv_obj_set_size(line, 220, 2);
    lv_obj_set_pos(line, 10, 120);
    lv_obj_set_style_bg_color(line, lv_color_make(0x40, 0x40, 0x40), 0);

    // 创建标签
    s_lbl_temp = create_label(s_screen, 10,  10, "TEMP: --.-C");
    s_lbl_fan  = create_label(s_screen, 10,  50, "FAN:  ---- RPM");
    s_lbl_psu  = create_label(s_screen, 10,  90, "PSU:  ---");
    s_lbl_load = create_label(s_screen, 10, 140, "LOAD: --.--A");
    s_lbl_12v  = create_label(s_screen, 10, 175, " 12V: --.--A");
    s_lbl_5v   = create_label(s_screen, 10, 210, "  5V: --.--A");

    lv_scr_load(s_screen);
}

void update_temperature(float temp_c) {
    if (!s_lbl_temp) return;
    char buf[32];
    snprintf(buf, sizeof(buf), "TEMP: %.1fC", static_cast<double>(temp_c));
    lv_label_set_text(s_lbl_temp, buf);
}

void update_fan_rpm(uint32_t rpm) {
    if (!s_lbl_fan) return;
    char buf[32];
    snprintf(buf, sizeof(buf), "FAN:  %4lu RPM", static_cast<unsigned long>(rpm));
    lv_label_set_text(s_lbl_fan, buf);
}

void update_psu_state(const char* state_str) {
    if (!s_lbl_psu) return;
    char buf[32];
    snprintf(buf, sizeof(buf), "PSU:  %s", state_str);
    lv_label_set_text(s_lbl_psu, buf);
}

void update_current(float load_a, float v12_a, float v5_a) {
    if (!s_lbl_load) return;
    char buf[32];
    snprintf(buf, sizeof(buf), "LOAD: %.2fA", static_cast<double>(load_a));
    lv_label_set_text(s_lbl_load, buf);
    snprintf(buf, sizeof(buf), " 12V: %.2fA", static_cast<double>(v12_a));
    lv_label_set_text(s_lbl_12v, buf);
    snprintf(buf, sizeof(buf), "  5V: %.2fA", static_cast<double>(v5_a));
    lv_label_set_text(s_lbl_5v, buf);
}

lv_obj_t* get_screen() {
    return s_screen;
}

} // namespace ui_main
#endif // legacy UI
