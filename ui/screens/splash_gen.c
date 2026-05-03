/**
 * @file splash_gen.c
 * @brief Template source file for LVGL objects
 */

/*********************
 *      INCLUDES
 *********************/

#include "splash_gen.h"
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

lv_obj_t * splash_create(void)
{
    LV_TRACE_OBJ_CREATE("begin");

    static lv_style_t style_main;
    static lv_style_t style_bolt;
    static lv_style_t style_brand;
    static lv_style_t style_zap_line;
    static lv_style_t style_zap_line_ind;
    static lv_style_t style_slogan;

    static bool style_inited = false;

    if (!style_inited) {
        lv_style_init(&style_main);
        lv_style_set_bg_color(&style_main, lv_color_hex(0x000000));
        lv_style_set_bg_opa(&style_main, 255);
        lv_style_set_text_color(&style_main, lv_color_hex(0xFFFFFF));
        lv_style_set_pad_all(&style_main, 0);
        lv_style_set_border_width(&style_main, 0);

        lv_style_init(&style_bolt);
        lv_style_set_text_color(&style_bolt, lv_color_hex(0xffb020));
        lv_style_set_text_opa(&style_bolt, 255);

        lv_style_init(&style_brand);
        lv_style_set_text_color(&style_brand, lv_color_hex(0xFFFFFF));
        lv_style_set_text_opa(&style_brand, 255);
        lv_style_set_text_letter_space(&style_brand, 2);
        lv_style_set_text_align(&style_brand, LV_TEXT_ALIGN_CENTER);

        lv_style_init(&style_zap_line);
        lv_style_set_bg_color(&style_zap_line, lv_color_hex(0x1a1f26));
        lv_style_set_bg_opa(&style_zap_line, 255);
        lv_style_set_border_width(&style_zap_line, 0);
        lv_style_set_radius(&style_zap_line, 2);
        lv_style_set_pad_all(&style_zap_line, 0);

        lv_style_init(&style_zap_line_ind);
        lv_style_set_bg_color(&style_zap_line_ind, lv_color_hex(0xffb020));
        lv_style_set_bg_opa(&style_zap_line_ind, 255);
        lv_style_set_radius(&style_zap_line_ind, 2);

        lv_style_init(&style_slogan);
        lv_style_set_text_color(&style_slogan, lv_color_hex(0x6b7d8e));
        lv_style_set_text_opa(&style_slogan, 255);
        lv_style_set_text_letter_space(&style_slogan, 1);

        style_inited = true;
    }

    lv_obj_t * lv_obj_0 = lv_obj_create(NULL);
    lv_obj_set_name_static(lv_obj_0, "splash_#");
    lv_obj_set_width(lv_obj_0, lv_pct(100));
    lv_obj_set_height(lv_obj_0, lv_pct(100));
    lv_obj_set_align(lv_obj_0, LV_ALIGN_CENTER);
    lv_obj_set_style_layout(lv_obj_0, LV_LAYOUT_FLEX, 0);
    lv_obj_set_flex_flow(lv_obj_0, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_flex_main_place(lv_obj_0, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_cross_place(lv_obj_0, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_flex_track_place(lv_obj_0, LV_FLEX_ALIGN_CENTER, 0);
    lv_obj_set_style_pad_all(lv_obj_0, 0, 0);
    lv_obj_set_style_pad_row(lv_obj_0, 6, 0);

    lv_obj_add_style(lv_obj_0, &style_main, 0);
    lv_obj_t * bolt = lv_label_create(lv_obj_0);
    lv_obj_set_name(bolt, "bolt");
    lv_label_set_text(bolt, "");
    lv_obj_set_style_text_font(bolt, font_awesome_48, 0);
    lv_obj_add_style(bolt, &style_bolt, 0);
    
    lv_obj_t * brand = lv_label_create(lv_obj_0);
    lv_obj_set_name(brand, "brand");
    lv_label_set_text(brand, "MORNSUN");
    lv_obj_set_style_text_font(brand, hos_bold_splash, 0);
    lv_obj_add_style(brand, &style_brand, 0);
    
    lv_obj_t * zap_line = lv_bar_create(lv_obj_0);
    lv_obj_set_name(zap_line, "zap_line");
    lv_obj_set_width(zap_line, 180);
    lv_obj_set_height(zap_line, 4);
    lv_bar_set_min_value(zap_line, 0);
    lv_bar_set_max_value(zap_line, 100);
    lv_bar_set_value(zap_line, 0, false);
    lv_obj_add_style(zap_line, &style_zap_line, 0);
    
    lv_obj_t * slogan = lv_label_create(lv_obj_0);
    lv_obj_set_name(slogan, "slogan");
    lv_label_set_text(slogan, "Power System Inside");
    lv_obj_set_style_text_font(slogan, hos_14, 0);
    lv_obj_add_style(slogan, &style_slogan, 0);

    LV_TRACE_OBJ_CREATE("finished");

    return lv_obj_0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

