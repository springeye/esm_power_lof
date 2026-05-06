#include "theme_manager.h"

#include "../app/config_manager.h"

namespace {

const theme_manager::ThemeColors kColorsDay = {
    .scr_bg             = lv_color_hex(0xF0F2F5),
    .card_bg            = lv_color_hex(0xFFFFFF),
    .card_border        = lv_color_hex(0xD1D5DB),
    .text_primary       = lv_color_hex(0x111827),
    .text_secondary     = lv_color_hex(0x374151),
    .row_default_bg     = lv_color_hex(0xFFFFFF),
    .row_focus_bg       = lv_color_hex(0xE5E7EB),
    .row_edit_bg        = lv_color_hex(0xF3F4F6),
    .row_default_border = lv_color_hex(0xD1D5DB),
    .row_focus_border   = lv_color_hex(0x3B82F6),
    .row_edit_border    = lv_color_hex(0xF59E0B),
};

const theme_manager::ThemeColors kColorsNight = {
    .scr_bg             = lv_color_hex(0x0a0e14),
    .card_bg            = lv_color_hex(0x111820),
    .card_border        = lv_color_hex(0x1a2533),
    .text_primary       = lv_color_hex(0xFFFFFF),
    .text_secondary     = lv_color_hex(0xFFFFFF),
    .row_default_bg     = lv_color_hex(0x111820),
    .row_focus_bg       = lv_color_hex(0x162536),
    .row_edit_bg        = lv_color_hex(0x1b2430),
    .row_default_border = lv_color_hex(0x1a2533),
    .row_focus_border   = lv_color_hex(0x3d9eff),
    .row_edit_border    = lv_color_hex(0xffb020),
};

theme_manager::ThemeVariant g_current_variant = theme_manager::Night;

const theme_manager::ThemeColors& current_colors() {
    return (g_current_variant == theme_manager::Day)
        ? kColorsDay
        : kColorsNight;
}

void apply_recursive(lv_obj_t* obj, const theme_manager::ThemeColors& c) {
    if (obj == nullptr) {
        return;
    }

    lv_opa_t bg_opa = lv_obj_get_style_bg_opa(obj, LV_PART_MAIN);
    if (bg_opa > 0) {
        lv_obj_set_style_bg_color(obj, c.card_bg, 0);
        lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);

        int32_t border_w = lv_obj_get_style_border_width(obj, LV_PART_MAIN);
        if (border_w > 0) {
            lv_obj_set_style_border_color(obj, c.card_border, 0);
        }
    }

    if (lv_obj_check_type(obj, &lv_label_class)) {
        lv_obj_set_style_text_color(obj, c.text_primary, 0);
    }

    const uint32_t child_cnt = lv_obj_get_child_cnt(obj);
    for (uint32_t i = 0; i < child_cnt; ++i) {
        apply_recursive(lv_obj_get_child(obj, i), c);
    }
}

} // namespace

namespace theme_manager {

void theme_init() {
    const uint8_t mode = config_manager::get_theme_mode();
    g_current_variant = (mode == 0) ? Day : Night;
}

void theme_apply(lv_obj_t* screen) {
    if (screen == nullptr) {
        return;
    }

    const ThemeColors& c = current_colors();

    lv_obj_set_style_bg_color(screen, c.scr_bg, 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    apply_recursive(screen, c);
}

void theme_toggle() {
    g_current_variant = (g_current_variant == Day) ? Night : Day;
    config_manager::set_theme_mode(static_cast<uint8_t>(g_current_variant));
    theme_apply_to_active_screen();
}

const ThemeColors& theme_current_colors() {
    return current_colors();
}

ThemeVariant theme_current_variant() {
    return g_current_variant;
}

void theme_apply_to_active_screen() {
    theme_apply(lv_screen_active());
}

} // namespace theme_manager
