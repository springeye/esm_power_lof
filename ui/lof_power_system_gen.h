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

extern lv_font_t * hos_black;

extern lv_font_t * hos_bold;

extern lv_font_t * hos_bold_big;

extern lv_font_t * hos_light;

extern lv_font_t * hos_medium;

extern lv_font_t * hos_regular;

extern lv_font_t * hos_thin;

/*----------------
 * Images
 *----------------*/

/*----------------
 * Subjects
 *----------------*/

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
#include "screens/splash_gen.h"

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LOF_POWER_SYSTEM_GEN_H*/