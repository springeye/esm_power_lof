/**
 * @file settings_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "settings_gen.h"
#include "../lof_power_system.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/***********************
 *  STATIC VARIABLES
 **********************/

/***********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * settings_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_main;
    static lv_style_t style_item;
    static lv_style_t style_no_pad;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_main);
        lv_style_set_bg_color(&style_main, lv_color_hex(0x0a0e14));
        lv_style_set_bg_opa(&style_main, 255);
        lv_style_set_text_color(&style_main, lv_color_hex(0xFFFFFF));
        lv_style_set_pad_all(&style_main, 0);
        lv_style_set_border_width(&style_main, 0);

        lv_style_init(&style_item);
        lv_style_set_bg_color(&style_item, lv_color_hex(0x111820));
        lv_style_set_border_color(&style_item, lv_color_hex(0x1a2533));
        lv_style_set_border_width(&style_item, 1);
        lv_style_set_radius(&style_item, 3);
        lv_style_set_pad_all(&style_item, 4);

        lv_style_init(&style_no_pad);
        lv_style_set_pad_all(&style_no_pad, 0);
        lv_style_set_pad_row(&style_no_pad, 0);
        lv_style_set_pad_column(&style_no_pad, 0);
        lv_style_set_margin_all(&style_no_pad, 0);
        lv_style_set_border_width(&style_no_pad, 0);
        lv_style_set_radius(&style_no_pad, 0);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_name_static(lv_obj_0, "settings_#");
    lv_obj_set_width(lv_obj_0, lv_pct(100));
    lv_obj_set_height(lv_obj_0, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_0, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(lv_obj_0, lv_color_hex(0x0a0e14), 0);
    lv_obj_set_style_bg_opa(lv_obj_0, 255, 0);
    lv_obj_set_style_pad_all(lv_obj_0, 4, 0);
    lv_obj_set_style_pad_row(lv_obj_0, 0, 0);
    lv_obj_set_style_pad_column(lv_obj_0, 0, 0);

    lv_obj_add_style(lv_obj_0, &style_main, 0);
    lv_obj_add_style(lv_obj_0, &style_no_pad, 0);
    lv_obj_t * lv_obj_1 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_1, lv_pct(100));
    lv_obj_set_height(lv_obj_1, 22);
    lv_obj_set_style_layout(lv_obj_1, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_1, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_1, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_1, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(lv_obj_1, lv_color_hex(0x111820), 0);
    lv_obj_set_style_bg_opa(lv_obj_1, 255, 0);
    lv_obj_set_style_border_width(lv_obj_1, 1, 0);
    lv_obj_set_style_border_color(lv_obj_1, lv_color_hex(0x1a2533), 0);
    lv_obj_set_style_radius(lv_obj_1, 3, 0);
    lv_obj_set_style_pad_left(lv_obj_1, 6, 0);
    lv_obj_set_style_pad_right(lv_obj_1, 6, 0);
    lv_obj_set_style_pad_top(lv_obj_1, 0, 0);
    lv_obj_set_style_pad_bottom(lv_obj_1, 0, 0);
    lv_obj_add_style(lv_obj_1, &style_no_pad, 0);
    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_0, "<");
    lv_obj_set_style_text_font(lv_label_0, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_0, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_1 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_1, "设置");
    lv_obj_set_style_text_font(lv_label_1, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_1, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_2 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_2, "1/5");
    lv_obj_set_style_text_font(lv_label_2, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_2, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_2 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_2, lv_pct(100));
    lv_obj_set_height(lv_obj_2, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_2, 1, 0);
    lv_obj_set_style_layout(lv_obj_2, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_2, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_opa(lv_obj_2, 0, 0);
    lv_obj_set_style_border_width(lv_obj_2, 0, 0);
    lv_obj_set_style_pad_all(lv_obj_2, 0, 0);
    lv_obj_set_style_pad_row(lv_obj_2, 4, 0);
    lv_obj_set_style_pad_column(lv_obj_2, 0, 0);
    lv_obj_add_style(lv_obj_2, &style_no_pad, 0);
    
    lv_obj_t * lv_obj_3 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_3, lv_pct(100));
    lv_obj_set_height(lv_obj_3, 18);
    lv_obj_set_style_layout(lv_obj_3, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_3, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_3, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_3, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_3, 0, 0);
    lv_obj_set_style_border_width(lv_obj_3, 0, 0);
    lv_obj_set_style_pad_all(lv_obj_3, 0, 0);
    lv_obj_add_style(lv_obj_3, &style_no_pad, 0);
    lv_obj_t * lv_label_3 = lv_label_create(lv_obj_3);
    lv_label_set_text(lv_label_3, "K1/K3:移动  K2:编辑  长按K2:返回");
    lv_obj_set_style_text_font(lv_label_3, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_3, lv_color_hex(0x6b7d8e), 0);

    LV_TRACE_OBJ_CREATE("finished");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

