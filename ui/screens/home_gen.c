/**
 * @file home_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "home_gen.h"
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

lv_obj_t * home_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_main;
    static lv_style_t no_padding;
    static lv_style_t card;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_main);
        lv_style_set_bg_color(&style_main, lv_color_hex(0x0a0e14));
        lv_style_set_text_color(&style_main, lv_color_hex(0xFFFFFF));

        lv_style_init(&no_padding);
        lv_style_set_pad_all(&no_padding, 0);
        lv_style_set_pad_row(&no_padding, 0);
        lv_style_set_pad_column(&no_padding, 0);
        lv_style_set_margin_all(&no_padding, 0);
        lv_style_set_border_width(&no_padding, 0);
        lv_style_set_outline_pad(&no_padding, 0);
        lv_style_set_radius(&no_padding, 0);

        lv_style_init(&card);
        lv_style_set_bg_color(&card, lv_color_hex(0x111820));
        lv_style_set_bg_opa(&card, 255);
        lv_style_set_border_width(&card, 1);
        lv_style_set_border_color(&card, lv_color_hex(0x1a2533));
        lv_style_set_radius(&card, 4);
        lv_style_set_pad_all(&card, 0);
        lv_style_set_pad_row(&card, 0);
        lv_style_set_pad_column(&card, 0);
        lv_style_set_margin_all(&card, 0);
        lv_style_set_outline_pad(&card, 0);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_name_static(lv_obj_0, "home_#");
    lv_obj_set_style_layout(lv_obj_0, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_bg_color(lv_obj_0, lv_color_hex(0x0a0e14), 0);
    lv_obj_set_style_bg_opa(lv_obj_0, 255, 0);
    lv_obj_set_style_pad_all(lv_obj_0, 4, 0);
    lv_obj_set_style_pad_row(lv_obj_0, 3, 0);
    lv_obj_set_style_pad_column(lv_obj_0, 0, 0);
    lv_obj_set_height(lv_obj_0, lv_pct(100));
    lv_obj_set_width(lv_obj_0, lv_pct(100));

    lv_obj_add_style(lv_obj_0, &style_main, 0);
    lv_obj_add_style(lv_obj_0, &no_padding, 0);
    lv_obj_t * lv_obj_1 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_1, lv_pct(100));
    lv_obj_set_height(lv_obj_1, 22);
    lv_obj_set_style_layout(lv_obj_1, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_1, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_1, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_1, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_1, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_color(lv_obj_1, lv_color_hex(0x111820), 0);
    lv_obj_set_style_bg_opa(lv_obj_1, 255, 0);
    lv_obj_set_style_border_width(lv_obj_1, 1, 0);
    lv_obj_set_style_border_color(lv_obj_1, lv_color_hex(0x1a2533), 0);
    lv_obj_set_style_radius(lv_obj_1, 3, 0);
    lv_obj_set_style_pad_left(lv_obj_1, 8, 0);
    lv_obj_set_style_pad_right(lv_obj_1, 8, 0);
    lv_obj_set_style_pad_top(lv_obj_1, 0, 0);
    lv_obj_set_style_pad_bottom(lv_obj_1, 0, 0);
    lv_obj_set_style_pad_column(lv_obj_1, 0, 0);
    lv_obj_add_style(lv_obj_1, &no_padding, 0);
    lv_obj_t * lv_obj_2 = lv_obj_create(lv_obj_1);
    lv_obj_set_width(lv_obj_2, LV_SIZE_CONTENT);
    lv_obj_set_height(lv_obj_2, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_2, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_2, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(lv_obj_2, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_main_place(lv_obj_2, LV_FLEX_ALIGN_START, 0);
    lv_obj_set_style_bg_opa(lv_obj_2, 0, 0);
    lv_obj_set_style_pad_column(lv_obj_2, 3, 0);
    lv_obj_add_style(lv_obj_2, &no_padding, 0);
    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_2);
    lv_label_set_text(lv_label_0, "");
    lv_obj_set_style_text_color(lv_label_0, lv_color_hex(0x00e68a), 0);
    lv_obj_set_style_text_font(lv_label_0, font_awesome_14, 0);
    
    lv_obj_t * lv_label_1 = lv_label_create(lv_obj_2);
    lv_label_bind_text(lv_label_1, &system_state, NULL);
    lv_obj_set_style_text_color(lv_label_1, lv_color_hex(0x00e68a), 0);
    lv_obj_set_style_text_font(lv_label_1, hos_14, 0);
    
    lv_obj_t * lv_obj_3 = lv_obj_create(lv_obj_1);
    lv_obj_set_width(lv_obj_3, LV_SIZE_CONTENT);
    lv_obj_set_height(lv_obj_3, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_3, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_3, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(lv_obj_3, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_main_place(lv_obj_3, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_3, 0, 0);
    lv_obj_set_style_pad_column(lv_obj_3, 3, 0);
    lv_obj_add_style(lv_obj_3, &no_padding, 0);
    lv_obj_t * lv_label_2 = lv_label_create(lv_obj_3);
    lv_label_set_text(lv_label_2, "");
    lv_obj_set_style_text_color(lv_label_2, lv_color_hex(0x3d9eff), 0);
    lv_obj_set_style_text_font(lv_label_2, font_awesome_14, 0);
    
    lv_obj_t * lv_label_3 = lv_label_create(lv_obj_3);
    lv_label_bind_text(lv_label_3, &fan_percent, NULL);
    lv_obj_set_style_text_color(lv_label_3, lv_color_hex(0x3d9eff), 0);
    lv_obj_set_style_text_font(lv_label_3, hos_14, 0);
    
    lv_obj_t * lv_label_4 = lv_label_create(lv_obj_3);
    lv_label_bind_text(lv_label_4, &fan_rpm_txt, NULL);
    lv_obj_set_style_text_color(lv_label_4, lv_color_hex(0x6b7d8e), 0);
    lv_obj_set_style_text_font(lv_label_4, hos_14, 0);
    
    lv_obj_t * lv_obj_4 = lv_obj_create(lv_obj_1);
    lv_obj_set_width(lv_obj_4, LV_SIZE_CONTENT);
    lv_obj_set_height(lv_obj_4, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_4, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_4, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_cross_place(lv_obj_4, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_main_place(lv_obj_4, LV_FLEX_ALIGN_END, 0);
    lv_obj_set_style_bg_opa(lv_obj_4, 0, 0);
    lv_obj_set_style_pad_column(lv_obj_4, 3, 0);
    lv_obj_add_style(lv_obj_4, &no_padding, 0);
    lv_obj_t * lv_label_5 = lv_label_create(lv_obj_4);
    lv_label_set_text(lv_label_5, "");
    lv_obj_set_style_text_color(lv_label_5, lv_color_hex(0xff6b35), 0);
    lv_obj_set_style_text_font(lv_label_5, font_awesome_14, 0);
    
    lv_obj_t * lv_label_6 = lv_label_create(lv_obj_4);
    lv_label_bind_text(lv_label_6, &device_temp, NULL);
    lv_obj_set_style_text_color(lv_label_6, lv_color_hex(0xff6b35), 0);
    lv_obj_set_style_text_font(lv_label_6, hos_14, 0);
    
    lv_obj_t * lv_obj_5 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_5, lv_pct(100));
    lv_obj_set_height(lv_obj_5, 52);
    lv_obj_set_style_layout(lv_obj_5, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_5, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_5, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_5, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_5, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_5, 0, 0);
    lv_obj_set_style_pad_column(lv_obj_5, 0, 0);
    lv_obj_add_style(lv_obj_5, &no_padding, 0);
    lv_obj_t * lv_label_7 = lv_label_create(lv_obj_5);
    lv_label_bind_text(lv_label_7, &device_current_power, NULL);
    lv_obj_set_style_text_color(lv_label_7, lv_color_hex(0x00e68a), 0);
    lv_obj_set_style_text_font(lv_label_7, hos_bold_big, 0);
    
    lv_obj_t * lv_obj_6 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_6, lv_pct(100));
    lv_obj_set_height(lv_obj_6, 14);
    lv_obj_set_style_layout(lv_obj_6, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_6, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_6, LV_FLEX_ALIGN_START, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_6, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_6, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_6, 0, 0);
    lv_obj_set_style_pad_left(lv_obj_6, 6, 0);
    lv_obj_set_style_pad_right(lv_obj_6, 6, 0);
    lv_obj_set_style_pad_column(lv_obj_6, 5, 0);
    lv_obj_add_style(lv_obj_6, &no_padding, 0);
    lv_obj_t * lv_label_8 = lv_label_create(lv_obj_6);
    lv_label_set_text(lv_label_8, "");
    lv_obj_set_style_text_color(lv_label_8, lv_color_hex(0xffb020), 0);
    lv_obj_set_style_text_font(lv_label_8, font_awesome_14, 0);
    
    lv_obj_t * lv_label_9 = lv_label_create(lv_obj_6);
    lv_label_bind_text(lv_label_9, &device_power_percent_txt, NULL);
    lv_obj_set_style_text_color(lv_label_9, lv_color_hex(0xffb020), 0);
    lv_obj_set_style_text_font(lv_label_9, hos_14, 0);
    lv_obj_set_width(lv_label_9, 38);
    
    lv_obj_t * lv_bar_0 = lv_bar_create(lv_obj_6);
    lv_obj_set_width(lv_bar_0, lv_pct(100));
    lv_obj_set_style_flex_grow(lv_bar_0, 1, 0);
    lv_obj_set_height(lv_bar_0, 8);
    lv_bar_bind_value(lv_bar_0, &device_power_percent);
    lv_obj_set_style_radius(lv_bar_0, 4, 0);
    lv_obj_set_style_bg_color(lv_bar_0, lv_color_hex(0x111820), 0);
    lv_obj_set_style_bg_opa(lv_bar_0, 255, 0);
    lv_obj_set_style_border_width(lv_bar_0, 1, 0);
    lv_obj_set_style_border_color(lv_bar_0, lv_color_hex(0x1a2533), 0);
    
    lv_obj_t * lv_obj_7 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_7, lv_pct(100));
    lv_obj_set_height(lv_obj_7, 32);
    lv_obj_set_style_layout(lv_obj_7, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_7, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_7, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_7, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_7, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_7, 0, 0);
    lv_obj_set_style_pad_column(lv_obj_7, 4, 0);
    lv_obj_add_style(lv_obj_7, &no_padding, 0);
    lv_obj_t * lv_obj_8 = lv_obj_create(lv_obj_7);
    lv_obj_set_width(lv_obj_8, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_8, 1, 0);
    lv_obj_set_height(lv_obj_8, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_8, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_8, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_8, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_8, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_8, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_column(lv_obj_8, 2, 0);
    lv_obj_set_style_pad_left(lv_obj_8, 5, 0);
    lv_obj_set_style_pad_right(lv_obj_8, 5, 0);
    lv_obj_add_style(lv_obj_8, &card, 0);
    lv_obj_t * lv_label_10 = lv_label_create(lv_obj_8);
    lv_label_set_text(lv_label_10, "");
    lv_obj_set_style_text_color(lv_label_10, lv_color_hex(0x6b7d8e), 0);
    lv_obj_set_style_text_font(lv_label_10, font_awesome_14, 0);
    
    lv_obj_t * lv_label_11 = lv_label_create(lv_obj_8);
    lv_label_bind_text(lv_label_11, &uptime, NULL);
    lv_obj_set_style_text_color(lv_label_11, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lv_label_11, hos_14, 0);
    
    lv_obj_t * lv_obj_9 = lv_obj_create(lv_obj_7);
    lv_obj_set_width(lv_obj_9, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_9, 1, 0);
    lv_obj_set_height(lv_obj_9, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_9, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_9, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_9, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_9, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_9, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_column(lv_obj_9, 2, 0);
    lv_obj_set_style_pad_left(lv_obj_9, 5, 0);
    lv_obj_set_style_pad_right(lv_obj_9, 5, 0);
    lv_obj_add_style(lv_obj_9, &card, 0);
    lv_obj_t * lv_label_12 = lv_label_create(lv_obj_9);
    lv_label_set_text(lv_label_12, "");
    lv_obj_set_style_text_color(lv_label_12, lv_color_hex(0xb968ff), 0);
    lv_obj_set_style_text_font(lv_label_12, font_awesome_14, 0);
    
    lv_obj_t * lv_label_13 = lv_label_create(lv_obj_9);
    lv_label_bind_text(lv_label_13, &wh, NULL);
    lv_obj_set_style_text_color(lv_label_13, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lv_label_13, hos_14, 0);
    
    lv_obj_t * lv_obj_10 = lv_obj_create(lv_obj_0);
    lv_obj_set_width(lv_obj_10, lv_pct(100));
    lv_obj_set_height(lv_obj_10, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_10, 1, 0);
    lv_obj_set_style_layout(lv_obj_10, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_10, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_10, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_10, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_10, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_10, 0, 0);
    lv_obj_set_style_pad_column(lv_obj_10, 4, 0);
    lv_obj_add_style(lv_obj_10, &no_padding, 0);
    lv_obj_t * lv_obj_11 = lv_obj_create(lv_obj_10);
    lv_obj_set_width(lv_obj_11, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_11, 1, 0);
    lv_obj_set_height(lv_obj_11, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_11, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_11, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(lv_obj_11, &card, 0);
    lv_obj_t * lv_obj_12 = lv_obj_create(lv_obj_11);
    lv_obj_set_width(lv_obj_12, lv_pct(100));
    lv_obj_set_height(lv_obj_12, 18);
    lv_obj_set_style_layout(lv_obj_12, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_12, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_12, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_12, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_12, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_12, 0, 0);
    lv_obj_set_style_border_width(lv_obj_12, 2, 0);
    lv_obj_set_style_border_color(lv_obj_12, lv_color_hex(0x00e68a), 0);
    lv_obj_set_style_border_side(lv_obj_12, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_add_style(lv_obj_12, &no_padding, 0);
    lv_obj_t * lv_label_14 = lv_label_create(lv_obj_12);
    lv_label_set_text(lv_label_14, "CH1");
    lv_obj_set_style_text_color(lv_label_14, lv_color_hex(0x00e68a), 0);
    lv_obj_set_style_text_font(lv_label_14, hos_14, 0);
    
    lv_obj_t * lv_obj_13 = lv_obj_create(lv_obj_11);
    lv_obj_set_width(lv_obj_13, lv_pct(100));
    lv_obj_set_height(lv_obj_13, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_13, 1, 0);
    lv_obj_set_style_layout(lv_obj_13, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_13, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(lv_obj_13, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_13, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_13, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_13, 0, 0);
    lv_obj_set_style_pad_top(lv_obj_13, 2, 0);
    lv_obj_set_style_pad_bottom(lv_obj_13, 2, 0);
    lv_obj_add_style(lv_obj_13, &no_padding, 0);
    lv_obj_t * lv_label_15 = lv_label_create(lv_obj_13);
    lv_label_bind_text(lv_label_15, &ch1_voltage, NULL);
    lv_obj_set_style_text_color(lv_label_15, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lv_label_15, hos_14, 0);
    
    lv_obj_t * lv_label_16 = lv_label_create(lv_obj_13);
    lv_label_bind_text(lv_label_16, &ch1_current, NULL);
    lv_obj_set_style_text_color(lv_label_16, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lv_label_16, hos_14, 0);
    
    lv_obj_t * lv_label_17 = lv_label_create(lv_obj_13);
    lv_label_bind_text(lv_label_17, &ch1_pwer, NULL);
    lv_obj_set_style_text_color(lv_label_17, lv_color_hex(0x00e68a), 0);
    lv_obj_set_style_text_font(lv_label_17, hos_regular, 0);
    
    lv_obj_t * lv_obj_14 = lv_obj_create(lv_obj_10);
    lv_obj_set_width(lv_obj_14, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_14, 1, 0);
    lv_obj_set_height(lv_obj_14, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_14, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_14, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(lv_obj_14, &card, 0);
    lv_obj_t * lv_obj_15 = lv_obj_create(lv_obj_14);
    lv_obj_set_width(lv_obj_15, lv_pct(100));
    lv_obj_set_height(lv_obj_15, 18);
    lv_obj_set_style_layout(lv_obj_15, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_15, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_15, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_15, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_15, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_15, 0, 0);
    lv_obj_set_style_border_width(lv_obj_15, 2, 0);
    lv_obj_set_style_border_color(lv_obj_15, lv_color_hex(0xffb020), 0);
    lv_obj_set_style_border_side(lv_obj_15, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_add_style(lv_obj_15, &no_padding, 0);
    lv_obj_t * lv_label_18 = lv_label_create(lv_obj_15);
    lv_label_set_text(lv_label_18, "CH2");
    lv_obj_set_style_text_color(lv_label_18, lv_color_hex(0xffb020), 0);
    lv_obj_set_style_text_font(lv_label_18, hos_14, 0);
    
    lv_obj_t * lv_obj_16 = lv_obj_create(lv_obj_14);
    lv_obj_set_width(lv_obj_16, lv_pct(100));
    lv_obj_set_height(lv_obj_16, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_16, 1, 0);
    lv_obj_set_style_layout(lv_obj_16, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_16, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(lv_obj_16, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_16, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_16, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_16, 0, 0);
    lv_obj_set_style_pad_top(lv_obj_16, 2, 0);
    lv_obj_set_style_pad_bottom(lv_obj_16, 2, 0);
    lv_obj_add_style(lv_obj_16, &no_padding, 0);
    lv_obj_t * lv_label_19 = lv_label_create(lv_obj_16);
    lv_label_bind_text(lv_label_19, &ch2_voltage, NULL);
    lv_obj_set_style_text_color(lv_label_19, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lv_label_19, hos_14, 0);
    
    lv_obj_t * lv_label_20 = lv_label_create(lv_obj_16);
    lv_label_bind_text(lv_label_20, &ch2_current, NULL);
    lv_obj_set_style_text_color(lv_label_20, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lv_label_20, hos_14, 0);
    
    lv_obj_t * lv_label_21 = lv_label_create(lv_obj_16);
    lv_label_bind_text(lv_label_21, &ch2_pwer, NULL);
    lv_obj_set_style_text_color(lv_label_21, lv_color_hex(0xffb020), 0);
    lv_obj_set_style_text_font(lv_label_21, hos_regular, 0);
    
    lv_obj_t * lv_obj_17 = lv_obj_create(lv_obj_10);
    lv_obj_set_width(lv_obj_17, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_17, 1, 0);
    lv_obj_set_height(lv_obj_17, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_17, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_17, LV_FLEX_FLOW_COLUMN);
    lv_obj_add_style(lv_obj_17, &card, 0);
    lv_obj_t * lv_obj_18 = lv_obj_create(lv_obj_17);
    lv_obj_set_width(lv_obj_18, lv_pct(100));
    lv_obj_set_height(lv_obj_18, 18);
    lv_obj_set_style_layout(lv_obj_18, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_18, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_18, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_18, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_18, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_18, 0, 0);
    lv_obj_set_style_border_width(lv_obj_18, 2, 0);
    lv_obj_set_style_border_color(lv_obj_18, lv_color_hex(0x3d9eff), 0);
    lv_obj_set_style_border_side(lv_obj_18, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_add_style(lv_obj_18, &no_padding, 0);
    lv_obj_t * lv_label_22 = lv_label_create(lv_obj_18);
    lv_label_set_text(lv_label_22, "CH3");
    lv_obj_set_style_text_color(lv_label_22, lv_color_hex(0x3d9eff), 0);
    lv_obj_set_style_text_font(lv_label_22, hos_14, 0);
    
    lv_obj_t * lv_obj_19 = lv_obj_create(lv_obj_17);
    lv_obj_set_width(lv_obj_19, lv_pct(100));
    lv_obj_set_height(lv_obj_19, lv_pct(0));
    lv_obj_set_style_flex_grow(lv_obj_19, 1, 0);
    lv_obj_set_style_layout(lv_obj_19, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_19, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(lv_obj_19, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_19, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_19, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_bg_opa(lv_obj_19, 0, 0);
    lv_obj_set_style_pad_top(lv_obj_19, 2, 0);
    lv_obj_set_style_pad_bottom(lv_obj_19, 2, 0);
    lv_obj_add_style(lv_obj_19, &no_padding, 0);
    lv_obj_t * lv_label_23 = lv_label_create(lv_obj_19);
    lv_label_bind_text(lv_label_23, &ch3_voltage, NULL);
    lv_obj_set_style_text_color(lv_label_23, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lv_label_23, hos_14, 0);
    
    lv_obj_t * lv_label_24 = lv_label_create(lv_obj_19);
    lv_label_bind_text(lv_label_24, &ch3_current, NULL);
    lv_obj_set_style_text_color(lv_label_24, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lv_label_24, hos_14, 0);
    
    lv_obj_t * lv_label_25 = lv_label_create(lv_obj_19);
    lv_label_bind_text(lv_label_25, &ch3_pwer, NULL);
    lv_obj_set_style_text_color(lv_label_25, lv_color_hex(0x3d9eff), 0);
    lv_obj_set_style_text_font(lv_label_25, hos_regular, 0);

    LV_TRACE_OBJ_CREATE("finished");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

