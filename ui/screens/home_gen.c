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

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_main);
        lv_style_set_bg_color(&style_main, lv_color_hex(0x101214));
        lv_style_set_text_color(&style_main, lv_color_hex(0xFFFFFF));

        lv_style_init(&no_padding);
        lv_style_set_pad_all(&no_padding, 0);
        lv_style_set_pad_row(&no_padding, 0);
        lv_style_set_pad_column(&no_padding, 0);
        lv_style_set_margin_all(&no_padding, 0);
        lv_style_set_border_width(&no_padding, 0);
        lv_style_set_outline_pad(&no_padding, 0);
        lv_style_set_radius(&no_padding, 0);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_name_static(lv_obj_0, "home_#");
    lv_obj_set_style_layout(lv_obj_0, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(lv_obj_0, 4, 0);
    lv_obj_set_style_pad_row(lv_obj_0, 4, 0);
    lv_obj_set_style_pad_column(lv_obj_0, 4, 0);
    lv_obj_set_height(lv_obj_0, lv_pct(100));
    lv_obj_set_width(lv_obj_0, lv_pct(100));

    lv_obj_add_style(lv_obj_0, &style_main, 0);
    lv_obj_add_style(lv_obj_0, &no_padding, 0);
    lv_obj_t * lv_obj_1 = lv_obj_create(lv_obj_0);
    lv_obj_set_height(lv_obj_1, lv_pct(10));
    lv_obj_set_width(lv_obj_1, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_1, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_1, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_1, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_border_width(lv_obj_1, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_1, 0, 0);
    lv_obj_add_style(lv_obj_1, &no_padding, 0);
    lv_obj_t * lv_label_0 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_0, "LOF SYSTEM");
    lv_obj_set_style_text_color(lv_label_0, lv_color_hex(0xFF9900), 0);
    
    lv_obj_t * lv_label_1 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_1, "F");
    lv_obj_set_style_text_color(lv_label_1, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_2 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_2, "CAL");
    lv_obj_set_style_text_color(lv_label_2, lv_color_hex(0x00FFFF), 0);
    
    lv_obj_t * lv_label_3 = lv_label_create(lv_obj_1);
    lv_label_set_text(lv_label_3, "RDY");
    lv_obj_set_style_text_color(lv_label_3, lv_color_hex(0x00FFFF), 0);
    
    lv_obj_t * lv_obj_2 = lv_obj_create(lv_obj_0);
    lv_obj_set_height(lv_obj_2, lv_pct(10));
    lv_obj_set_width(lv_obj_2, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_2, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_2, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_2, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_bg_color(lv_obj_2, lv_color_hex(0x2C3E50), 0);
    lv_obj_set_style_bg_opa(lv_obj_2, 255, 0);
    lv_obj_set_style_border_width(lv_obj_2, 0, 0);
    lv_obj_set_style_pad_all(lv_obj_2, 4, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_2, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_add_style(lv_obj_2, &no_padding, 0);
    lv_obj_t * lv_label_4 = lv_label_create(lv_obj_2);
    lv_label_set_text(lv_label_4, "POWER PANEL");
    lv_obj_set_style_text_color(lv_label_4, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_5 = lv_label_create(lv_obj_2);
    lv_label_set_text(lv_label_5, "+30.6 ℃");
    lv_obj_set_style_text_color(lv_label_5, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_3 = lv_obj_create(lv_obj_0);
    lv_obj_set_height(lv_obj_3, lv_pct(20));
    lv_obj_set_width(lv_obj_3, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_3, LV_LAYOUT_FLEX, 0);
    lv_obj_set_style_flex_main_place(lv_obj_3, LV_FLEX_ALIGN_START, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_3, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(lv_obj_3, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_3, 0, 0);
    lv_obj_add_style(lv_obj_3, &no_padding, 0);
    lv_obj_t * lv_label_6 = lv_label_create(lv_obj_3);
    lv_label_set_text(lv_label_6, "Pt 000.00 w");
    lv_obj_set_style_text_color(lv_label_6, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_4 = lv_obj_create(lv_obj_0);
    lv_obj_set_height(lv_obj_4, lv_pct(15));
    lv_obj_set_width(lv_obj_4, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_4, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_4, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_4, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_border_width(lv_obj_4, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_4, 0, 0);
    lv_obj_add_style(lv_obj_4, &no_padding, 0);
    lv_obj_t * lv_obj_5 = lv_obj_create(lv_obj_4);
    lv_obj_set_width(lv_obj_5, lv_pct(48));
    lv_obj_set_height(lv_obj_5, lv_pct(100));
    lv_obj_set_style_border_color(lv_obj_5, lv_color_hex(0x00AAAA), 0);
    lv_obj_set_style_border_width(lv_obj_5, 2, 0);
    lv_obj_set_style_bg_opa(lv_obj_5, 0, 0);
    lv_obj_set_style_radius(lv_obj_5, 4, 0);
    lv_obj_set_style_layout(lv_obj_5, LV_LAYOUT_FLEX, 0);
    lv_obj_set_style_flex_main_place(lv_obj_5, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_5, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_t * lv_label_7 = lv_label_create(lv_obj_5);
    lv_label_set_text(lv_label_7, "H 02:33:57");
    lv_obj_set_style_text_color(lv_label_7, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_6 = lv_obj_create(lv_obj_4);
    lv_obj_set_width(lv_obj_6, lv_pct(48));
    lv_obj_set_height(lv_obj_6, lv_pct(100));
    lv_obj_set_style_border_color(lv_obj_6, lv_color_hex(0x00AAAA), 0);
    lv_obj_set_style_border_width(lv_obj_6, 2, 0);
    lv_obj_set_style_bg_opa(lv_obj_6, 0, 0);
    lv_obj_set_style_radius(lv_obj_6, 4, 0);
    lv_obj_set_style_layout(lv_obj_6, LV_LAYOUT_FLEX, 0);
    lv_obj_set_style_flex_main_place(lv_obj_6, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_6, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_t * lv_label_8 = lv_label_create(lv_obj_6);
    lv_label_set_text(lv_label_8, "0.0000 Wh");
    lv_obj_set_style_text_color(lv_label_8, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_7 = lv_obj_create(lv_obj_0);
    lv_obj_set_height(lv_obj_7, lv_pct(15));
    lv_obj_set_width(lv_obj_7, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_7, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_7, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_7, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_border_width(lv_obj_7, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_7, 0, 0);
    lv_obj_add_style(lv_obj_7, &no_padding, 0);
    lv_obj_t * lv_obj_8 = lv_obj_create(lv_obj_7);
    lv_obj_set_width(lv_obj_8, lv_pct(30));
    lv_obj_set_height(lv_obj_8, lv_pct(100));
    lv_obj_set_style_border_color(lv_obj_8, lv_color_hex(0x00AAAA), 0);
    lv_obj_set_style_border_width(lv_obj_8, 2, 0);
    lv_obj_set_style_bg_opa(lv_obj_8, 0, 0);
    lv_obj_set_style_radius(lv_obj_8, 4, 0);
    lv_obj_set_style_layout(lv_obj_8, LV_LAYOUT_FLEX, 0);
    lv_obj_set_style_flex_main_place(lv_obj_8, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_8, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_t * lv_label_9 = lv_label_create(lv_obj_8);
    lv_label_set_text(lv_label_9, "750");
    lv_obj_set_style_text_color(lv_label_9, lv_color_hex(0xCCFF00), 0);
    
    lv_obj_t * lv_obj_9 = lv_obj_create(lv_obj_7);
    lv_obj_set_width(lv_obj_9, lv_pct(65));
    lv_obj_set_height(lv_obj_9, lv_pct(100));
    lv_obj_set_style_border_color(lv_obj_9, lv_color_hex(0x00AAAA), 0);
    lv_obj_set_style_border_width(lv_obj_9, 2, 0);
    lv_obj_set_style_bg_opa(lv_obj_9, 0, 0);
    lv_obj_set_style_radius(lv_obj_9, 4, 0);
    lv_obj_set_style_layout(lv_obj_9, LV_LAYOUT_FLEX, 0);
    lv_obj_set_style_flex_main_place(lv_obj_9, LV_FLEX_ALIGN_END, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_9, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_right(lv_obj_9, 8, 0);
    lv_obj_t * lv_label_10 = lv_label_create(lv_obj_9);
    lv_label_set_text(lv_label_10, "0%");
    lv_obj_set_style_text_color(lv_label_10, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_10 = lv_obj_create(lv_obj_0);
    lv_obj_set_style_flex_grow(lv_obj_10, 1, 0);
    lv_obj_set_width(lv_obj_10, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_10, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_10, LV_FLEX_FLOW_ROW);
    lv_obj_set_style_flex_main_place(lv_obj_10, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
    lv_obj_set_style_border_width(lv_obj_10, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_10, 0, 0);
    lv_obj_add_style(lv_obj_10, &no_padding, 0);
    lv_obj_t * lv_obj_11 = lv_obj_create(lv_obj_10);
    lv_obj_set_width(lv_obj_11, lv_pct(32));
    lv_obj_set_height(lv_obj_11, lv_pct(100));
    lv_obj_set_style_border_color(lv_obj_11, lv_color_hex(0x00FF00), 0);
    lv_obj_set_style_border_width(lv_obj_11, 2, 0);
    lv_obj_set_style_radius(lv_obj_11, 6, 0);
    lv_obj_set_style_layout(lv_obj_11, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_11, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(lv_obj_11, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_11, 0, 0);
    lv_obj_t * lv_obj_12 = lv_obj_create(lv_obj_11);
    lv_obj_set_height(lv_obj_12, lv_pct(25));
    lv_obj_set_width(lv_obj_12, lv_pct(100));
    lv_obj_set_style_bg_color(lv_obj_12, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(lv_obj_12, 255, 0);
    lv_obj_set_style_border_width(lv_obj_12, 0, 0);
    lv_obj_set_style_layout(lv_obj_12, LV_LAYOUT_FLEX, 0);
    lv_obj_set_style_flex_main_place(lv_obj_12, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_12, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_t * lv_label_11 = lv_label_create(lv_obj_12);
    lv_label_set_text(lv_label_11, "CH1");
    lv_obj_set_style_text_color(lv_label_11, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_13 = lv_obj_create(lv_obj_11);
    lv_obj_set_style_flex_grow(lv_obj_13, 1, 0);
    lv_obj_set_width(lv_obj_13, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_13, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_13, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(lv_obj_13, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_13, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(lv_obj_13, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_13, 0, 0);
    lv_obj_t * lv_label_12 = lv_label_create(lv_obj_13);
    lv_label_set_text(lv_label_12, "0.000");
    lv_obj_set_style_text_color(lv_label_12, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_13 = lv_label_create(lv_obj_13);
    lv_label_set_text(lv_label_13, "0.000");
    lv_obj_set_style_text_color(lv_label_13, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_14 = lv_label_create(lv_obj_13);
    lv_label_set_text(lv_label_14, "0.000");
    lv_obj_set_style_text_color(lv_label_14, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_14 = lv_obj_create(lv_obj_10);
    lv_obj_set_width(lv_obj_14, lv_pct(32));
    lv_obj_set_height(lv_obj_14, lv_pct(100));
    lv_obj_set_style_border_color(lv_obj_14, lv_color_hex(0x00FFFF), 0);
    lv_obj_set_style_border_width(lv_obj_14, 2, 0);
    lv_obj_set_style_radius(lv_obj_14, 6, 0);
    lv_obj_set_style_layout(lv_obj_14, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_14, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(lv_obj_14, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_14, 0, 0);
    lv_obj_t * lv_obj_15 = lv_obj_create(lv_obj_14);
    lv_obj_set_height(lv_obj_15, lv_pct(25));
    lv_obj_set_width(lv_obj_15, lv_pct(100));
    lv_obj_set_style_bg_color(lv_obj_15, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(lv_obj_15, 255, 0);
    lv_obj_set_style_border_width(lv_obj_15, 0, 0);
    lv_obj_set_style_layout(lv_obj_15, LV_LAYOUT_FLEX, 0);
    lv_obj_set_style_flex_main_place(lv_obj_15, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_15, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_t * lv_label_15 = lv_label_create(lv_obj_15);
    lv_label_set_text(lv_label_15, "CH2");
    lv_obj_set_style_text_color(lv_label_15, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_16 = lv_obj_create(lv_obj_14);
    lv_obj_set_style_flex_grow(lv_obj_16, 1, 0);
    lv_obj_set_width(lv_obj_16, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_16, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_16, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(lv_obj_16, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_16, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(lv_obj_16, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_16, 0, 0);
    lv_obj_t * lv_label_16 = lv_label_create(lv_obj_16);
    lv_label_set_text(lv_label_16, "0.000");
    lv_obj_set_style_text_color(lv_label_16, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_17 = lv_label_create(lv_obj_16);
    lv_label_set_text(lv_label_17, "0.000");
    lv_obj_set_style_text_color(lv_label_17, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_18 = lv_label_create(lv_obj_16);
    lv_label_set_text(lv_label_18, "0.000");
    lv_obj_set_style_text_color(lv_label_18, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_17 = lv_obj_create(lv_obj_10);
    lv_obj_set_width(lv_obj_17, lv_pct(32));
    lv_obj_set_height(lv_obj_17, lv_pct(100));
    lv_obj_set_style_border_color(lv_obj_17, lv_color_hex(0xFFA500), 0);
    lv_obj_set_style_border_width(lv_obj_17, 2, 0);
    lv_obj_set_style_radius(lv_obj_17, 6, 0);
    lv_obj_set_style_layout(lv_obj_17, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_17, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_all(lv_obj_17, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_17, 0, 0);
    lv_obj_t * lv_obj_18 = lv_obj_create(lv_obj_17);
    lv_obj_set_height(lv_obj_18, lv_pct(25));
    lv_obj_set_width(lv_obj_18, lv_pct(100));
    lv_obj_set_style_bg_color(lv_obj_18, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(lv_obj_18, 255, 0);
    lv_obj_set_style_border_width(lv_obj_18, 0, 0);
    lv_obj_set_style_layout(lv_obj_18, LV_LAYOUT_FLEX, 0);
    lv_obj_set_style_flex_main_place(lv_obj_18, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_18, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_t * lv_label_19 = lv_label_create(lv_obj_18);
    lv_label_set_text(lv_label_19, "CH3");
    lv_obj_set_style_text_color(lv_label_19, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_obj_19 = lv_obj_create(lv_obj_17);
    lv_obj_set_style_flex_grow(lv_obj_19, 1, 0);
    lv_obj_set_width(lv_obj_19, lv_pct(100));
    lv_obj_set_style_layout(lv_obj_19, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_19, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(lv_obj_19, LV_FLEX_ALIGN_SPACE_EVENLY, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_19, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_border_width(lv_obj_19, 0, 0);
    lv_obj_set_style_bg_opa(lv_obj_19, 0, 0);
    lv_obj_t * lv_label_20 = lv_label_create(lv_obj_19);
    lv_label_set_text(lv_label_20, "0.000");
    lv_obj_set_style_text_color(lv_label_20, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_21 = lv_label_create(lv_obj_19);
    lv_label_set_text(lv_label_21, "0.000");
    lv_obj_set_style_text_color(lv_label_21, lv_color_hex(0xFFFFFF), 0);
    
    lv_obj_t * lv_label_22 = lv_label_create(lv_obj_19);
    lv_label_set_text(lv_label_22, "0.000");
    lv_obj_set_style_text_color(lv_label_22, lv_color_hex(0xFFFFFF), 0);

    LV_TRACE_OBJ_CREATE("finished");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

