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
    lv_obj_t * lv_obj_3 = lv_obj_create(lv_obj_2);
    lv_obj_set_width(lv_obj_3, lv_pct(100));
    lv_obj_set_height(lv_obj_3, 30);
    lv_obj_set_style_layout(lv_obj_3, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_3, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_3, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_3, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(lv_obj_3, lv_color_hex(0x111820), 0);
    lv_obj_set_style_bg_opa(lv_obj_3, 255, 0);
    lv_obj_set_style_border_color(lv_obj_3, lv_color_hex(0x1a2533), 0);
    lv_obj_set_style_border_width(lv_obj_3, 1, 0);
    lv_obj_set_style_radius(lv_obj_3, 3, 0);
    lv_obj_set_style_pad_left(lv_obj_3, 8, 0);
    lv_obj_set_style_pad_right(lv_obj_3, 8, 0);
    lv_obj_set_style_pad_top(lv_obj_3, 4, 0);
    lv_obj_set_style_pad_bottom(lv_obj_3, 4, 0);
    lv_obj_t * lv_label_3 = lv_label_create(lv_obj_3);
    lv_label_set_text(lv_label_3, "低温阈值");
    lv_obj_set_style_text_font(lv_label_3, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_3, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_4 = lv_label_create(lv_obj_3);
    lv_label_set_text(lv_label_4, "35.0°C");
    lv_obj_set_style_text_font(lv_label_4, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_4, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_4 = lv_obj_create(lv_obj_2);
    lv_obj_set_width(lv_obj_4, lv_pct(100));
    lv_obj_set_height(lv_obj_4, 30);
    lv_obj_set_style_layout(lv_obj_4, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_4, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_4, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_4, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(lv_obj_4, lv_color_hex(0x111820), 0);
    lv_obj_set_style_bg_opa(lv_obj_4, 255, 0);
    lv_obj_set_style_border_color(lv_obj_4, lv_color_hex(0x1a2533), 0);
    lv_obj_set_style_border_width(lv_obj_4, 1, 0);
    lv_obj_set_style_radius(lv_obj_4, 3, 0);
    lv_obj_set_style_pad_left(lv_obj_4, 8, 0);
    lv_obj_set_style_pad_right(lv_obj_4, 8, 0);
    lv_obj_set_style_pad_top(lv_obj_4, 4, 0);
    lv_obj_set_style_pad_bottom(lv_obj_4, 4, 0);
    lv_obj_t * lv_label_5 = lv_label_create(lv_obj_4);
    lv_label_set_text(lv_label_5, "中温阈值");
    lv_obj_set_style_text_font(lv_label_5, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_5, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_6 = lv_label_create(lv_obj_4);
    lv_label_set_text(lv_label_6, "45.0°C");
    lv_obj_set_style_text_font(lv_label_6, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_6, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_5 = lv_obj_create(lv_obj_2);
    lv_obj_set_width(lv_obj_5, lv_pct(100));
    lv_obj_set_height(lv_obj_5, 30);
    lv_obj_set_style_layout(lv_obj_5, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_5, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_5, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_5, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(lv_obj_5, lv_color_hex(0x162536), 0);
    lv_obj_set_style_bg_opa(lv_obj_5, 255, 0);
    lv_obj_set_style_border_color(lv_obj_5, lv_color_hex(0x3d9eff), 0);
    lv_obj_set_style_border_width(lv_obj_5, 1, 0);
    lv_obj_set_style_radius(lv_obj_5, 3, 0);
    lv_obj_set_style_pad_left(lv_obj_5, 8, 0);
    lv_obj_set_style_pad_right(lv_obj_5, 8, 0);
    lv_obj_set_style_pad_top(lv_obj_5, 4, 0);
    lv_obj_set_style_pad_bottom(lv_obj_5, 4, 0);
    lv_obj_t * lv_label_7 = lv_label_create(lv_obj_5);
    lv_label_set_text(lv_label_7, "低温阈值 >");
    lv_obj_set_style_text_font(lv_label_7, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_7, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_8 = lv_label_create(lv_obj_5);
    lv_label_set_text(lv_label_8, "35.0°C");
    lv_obj_set_style_text_font(lv_label_8, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_8, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_6 = lv_obj_create(lv_obj_2);
    lv_obj_set_width(lv_obj_6, lv_pct(100));
    lv_obj_set_height(lv_obj_6, 30);
    lv_obj_set_style_layout(lv_obj_6, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_6, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_6, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_6, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(lv_obj_6, lv_color_hex(0x1b2430), 0);
    lv_obj_set_style_bg_opa(lv_obj_6, 255, 0);
    lv_obj_set_style_border_color(lv_obj_6, lv_color_hex(0xffb020), 0);
    lv_obj_set_style_border_width(lv_obj_6, 1, 0);
    lv_obj_set_style_radius(lv_obj_6, 3, 0);
    lv_obj_set_style_pad_left(lv_obj_6, 8, 0);
    lv_obj_set_style_pad_right(lv_obj_6, 8, 0);
    lv_obj_set_style_pad_top(lv_obj_6, 4, 0);
    lv_obj_set_style_pad_bottom(lv_obj_6, 4, 0);
    lv_obj_t * lv_label_9 = lv_label_create(lv_obj_6);
    lv_label_set_text(lv_label_9, "低温阈值 *");
    lv_obj_set_style_text_font(lv_label_9, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_9, lv_color_hex(0xffd166), 0);
    
    lv_obj_t * lv_label_10 = lv_label_create(lv_obj_6);
    lv_label_set_text(lv_label_10, "40.0°C");
    lv_obj_set_style_text_font(lv_label_10, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_10, lv_color_hex(0xffd166), 0);
    
    lv_obj_t * lv_obj_7 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_7, lv_pct(100));
    lv_obj_set_height(lv_obj_7, 18);
    lv_obj_set_style_layout(lv_obj_7, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_7, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_7, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_7, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_7, 0, 0);
    lv_obj_set_style_border_width(lv_obj_7, 0, 0);
    lv_obj_set_style_pad_all(lv_obj_7, 0, 0);
    lv_obj_add_style(lv_obj_7, &style_no_pad, 0);
    lv_obj_t * lv_label_11 = lv_label_create(lv_obj_7);
    lv_label_set_text(lv_label_11, "K1/K3:移动  K2:编辑  长按K2:返回");
    lv_obj_set_style_text_font(lv_label_11, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_11, lv_color_hex(0x6b7d8e), 0);
    
    lv_obj_t * lv_obj_8 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_8, lv_pct(100));
    lv_obj_set_height(lv_obj_8, 18);
    lv_obj_set_style_layout(lv_obj_8, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_8, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_8, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_8, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_8, 0, 0);
    lv_obj_set_style_border_width(lv_obj_8, 0, 0);
    lv_obj_set_style_pad_all(lv_obj_8, 0, 0);
    lv_obj_add_style(lv_obj_8, &style_no_pad, 0);
    lv_obj_t * lv_label_12 = lv_label_create(lv_obj_8);
    lv_label_set_text(lv_label_12, "K1/K3:调整  K2:确认  长按K2:取消");
    lv_obj_set_style_text_font(lv_label_12, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_12, lv_color_hex(0x6b7d8e), 0);
    
    lv_obj_t * lv_obj_9 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_9, lv_pct(100));
    lv_obj_set_height(lv_obj_9, 18);
    lv_obj_set_style_layout(lv_obj_9, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_9, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_9, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_9, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_9, 0, 0);
    lv_obj_set_style_border_width(lv_obj_9, 0, 0);
    lv_obj_set_style_pad_all(lv_obj_9, 0, 0);
    lv_obj_add_style(lv_obj_9, &style_no_pad, 0);
    lv_obj_t * lv_label_13 = lv_label_create(lv_obj_9);
    lv_label_set_text(lv_label_13, "长按K1/K3:切换页面");
    lv_obj_set_style_text_font(lv_label_13, hos_14, 0);
    lv_obj_set_style_text_color(lv_label_13, lv_color_hex(0x6b7d8e), 0);

    LV_TRACE_OBJ_CREATE("finished");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

