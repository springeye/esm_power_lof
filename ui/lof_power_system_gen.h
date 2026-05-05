/**
 * @file lof_power_system_gen.h
 */

#ifndef LOF_POWER_SYSTEM_GEN_H
#define LOF_POWER_SYSTEM_GEN_H

#ifndef UI_SUBJECT_STRING_LENGTH
#define UI_SUBJECT_STRING_LENGTH 256
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
    #include "src/core/lv_obj_class_private.h"
#else
    #include "lvgl/lvgl.h"
    #include "lvgl/src/core/lv_obj_class_private.h"
#endif



/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 * GLOBAL VARIABLES
 **********************/

/*-------------------
 * Permanent screens
 *------------------*/

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Fonts
 *----------------*/

extern lv_font_t * hos_bold_big;

extern lv_font_t * hos_regular;

extern lv_font_t * hos_14;

extern lv_font_t * font_awesome_14;

extern lv_font_t * font_awesome_48;

extern lv_font_t * hos_bold_splash;

/*----------------
 * Images
 *----------------*/

/*----------------
 * Subjects
 *----------------*/

extern lv_subject_t system_name;
extern lv_subject_t system_state;
extern lv_subject_t device_current_power;
extern lv_subject_t uptime;
extern lv_subject_t wh;
extern lv_subject_t device_power;
extern lv_subject_t device_power_percent;
extern lv_subject_t device_power_percent_txt;
extern lv_subject_t ch1_voltage;
extern lv_subject_t ch1_current;
extern lv_subject_t ch1_pwer;
extern lv_subject_t ch2_voltage;
extern lv_subject_t ch2_current;
extern lv_subject_t ch2_pwer;
extern lv_subject_t ch3_voltage;
extern lv_subject_t ch3_current;
extern lv_subject_t ch3_pwer;
extern lv_subject_t device_temp;
extern lv_subject_t fan_percent;
extern lv_subject_t fan_rpm_txt;

/**********************
 * GLOBAL PROTOTYPES
 **********************/

/*----------------
 * Event Callbacks
 *----------------*/

/**
 * Initialize the component library
 */

void lof_power_system_init_gen(const char * asset_path);

/**********************
 *      MACROS
 **********************/

/**********************
 *   POST INCLUDES
 **********************/

/*Include all the widgets, components and screens of this library*/
#include "screens/home_gen.h"
#include "screens/settings_gen.h"
#include "screens/splash_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LOF_POWER_SYSTEM_GEN_H*/