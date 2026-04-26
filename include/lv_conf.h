/**
 * @file lv_conf.h
 * LVGL 9.x Configuration for ESP32 Fan Controller
 * ST7789 240x280, 16-bit color, SPI
 */

#ifndef LV_CONF_H
#define LV_CONF_H

#if 1  /* Set this to "1" to enable content */

#ifndef __ASSEMBLY__
#include <stdint.h>
#endif

/*====================
   COLOR SETTINGS
 *====================*/
#define LV_COLOR_DEPTH 16
/* Swap the 2 bytes of RGB565 color. Useful if the display has an 8-bit interface (e.g. SPI) */
#define LV_COLOR_16_SWAP 1

/*=========================
   MEMORY SETTINGS
 *=========================*/
/* Size of the memory available for `lv_malloc()` in bytes (>= 2kB) */
#define LV_MEM_SIZE (32 * 1024U)  /* 32KB */

/* Set an address for the memory pool instead of allocating it as a global array */
#define LV_MEM_ADR 0  /* 0: unused */

/*====================
   HAL SETTINGS
 *====================*/
/* Default display refresh, input device read and animation step period. */
#define LV_DEF_REFR_PERIOD 33  /* [ms] ~30fps */

/* Input device read period in milliseconds */
#define LV_INDEV_DEF_READ_PERIOD 30  /* [ms] */

/* Use a custom tick source that tells the elapsed time in milliseconds. */
#define LV_TICK_CUSTOM 1
#if LV_TICK_CUSTOM
    #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif

/* Default Dot Per Inch. Used to initialize default sizes such as widgets sized, style paddings.
 * (Not so important, you can adjust it to modify default sizes and spaces) */
#define LV_DPI_DEF 130  /* [px/inch] */

/*=======================
 * FEATURE CONFIGURATION
 *=======================*/

/*-------------
 * Drawing
 *-----------*/
/* Enable complex draw engine.
 * Required to draw shadow, gradient, rounded corners, circles, arc, skew lines, image transformations or any masks */
#define LV_DRAW_COMPLEX 1
#if LV_DRAW_COMPLEX != 0

    /* Allow buffering some shadow calculation.
     * LV_SHADOW_CACHE_SIZE is the max. shadow size to buffer, where shadow size is `shadow_width + radius`
     * Caching has LV_SHADOW_CACHE_SIZE^2 RAM cost */
    #define LV_SHADOW_CACHE_SIZE 0

    /* Set number of maximally cached circle data.
     * The circumference of 1/4 circle are saved for anti-aliasing
     * radius * 4 bytes are used per circle (the most often used radiuses are saved)
     * 0: to disable caching */
    #define LV_CIRCLE_CACHE_SIZE 4
#endif /* LV_DRAW_COMPLEX */

/*-------------
 * GPU
 *-----------*/
/* Use ESP32's DMA for image copy */
#define LV_USE_GPU_ESP32_DMA 0

/*-------------
 * Logging
 *-----------*/
/* Enable the log module */
#define LV_USE_LOG 0
#if LV_USE_LOG
    /* How important log should be added:
     * LV_LOG_LEVEL_TRACE       A lot of logs to give detailed information
     * LV_LOG_LEVEL_INFO        Log important events
     * LV_LOG_LEVEL_WARN        Log if something unwanted happened but didn't cause a problem
     * LV_LOG_LEVEL_ERROR       Only critical issue, when the system may fail
     * LV_LOG_LEVEL_USER        Only logs added by the user
     * LV_LOG_LEVEL_NONE        Do not log anything */
    #define LV_LOG_LEVEL LV_LOG_LEVEL_WARN

    /* 1: Print the log with 'printf';
     * 0: User need to register a callback with `lv_log_register_print_cb()` */
    #define LV_LOG_PRINTF 0
#endif  /* LV_USE_LOG */

/*-------------
 * Asserts
 *-----------*/
/* Enable asserts if an operation is failed or an invalid data is found.
 * If LV_USE_LOG is enabled an error message will be printed on failure */
#define LV_USE_ASSERT_NULL          1   /* Check if the parameter is NULL. (Very fast, recommended) */
#define LV_USE_ASSERT_MALLOC        1   /* Checks is the memory is successfully allocated or no. (Very fast, recommended) */
#define LV_USE_ASSERT_STYLE         0   /* Check if the styles are properly initialized. (Very fast, recommended) */
#define LV_USE_ASSERT_MEM_INTEGRITY 0   /* Check the integrity of `lv_mem` after critical operations. (Slow) */
#define LV_USE_ASSERT_OBJ           0   /* Check the object's type and existence (e.g. not deleted). (Slow) */

/* Add a custom handler when assert happens e.g. to restart the MCU */
#define LV_ASSERT_HANDLER_INCLUDE <stdint.h>
#define LV_ASSERT_HANDLER while(1);   /* Halt by default */

/*-------------
 * Others
 *-----------*/
/* 1: Show CPU usage and FPS count */
#define LV_USE_PERF_MONITOR 0
#if LV_USE_PERF_MONITOR
    #define LV_USE_PERF_MONITOR_POS LV_ALIGN_BOTTOM_RIGHT
#endif

/* 1: Show the used memory and the memory fragmentation */
#define LV_USE_MEM_MONITOR 0

/* 1: Draw random colored rectangles over the redrawn areas */
#define LV_USE_REFR_DEBUG 0

/* Change the built in (v)snprintf functions */
#define LV_SPRINTF_CUSTOM 0
#if LV_SPRINTF_CUSTOM
    #define LV_SPRINTF_INCLUDE <stdio.h>
    #define lv_snprintf  snprintf
    #define lv_vsnprintf vsnprintf
#else   /* LV_SPRINTF_CUSTOM */
    #define LV_SPRINTF_USE_FLOAT 0
#endif  /* LV_SPRINTF_CUSTOM */

#define LV_USE_USER_DATA 1

/* Garbage Collector settings
 * Used if lvgl is bound to higher level language and the memory is managed by that language */
#define LV_ENABLE_GC 0
#if LV_ENABLE_GC != 0
    #define LV_GC_INCLUDE "gc.h"                           /* Include Garbage Collector related things */
#endif /* LV_ENABLE_GC */

/*=====================
 *  COMPILER SETTINGS
 *====================*/
/* For big endian systems set to 1 */
#define LV_BIG_ENDIAN_SYSTEM 0

/* Define a custom attribute to `lv_tick_inc` function */
#define LV_ATTRIBUTE_TICK_INC

/* Define a custom attribute to `lv_timer_handler` function */
#define LV_ATTRIBUTE_TIMER_HANDLER

/* Define a custom attribute to `lv_disp_flush_ready` function */
#define LV_ATTRIBUTE_FLUSH_READY

/* Required alignment size for buffers */
#define LV_ATTRIBUTE_MEM_ALIGN_SIZE 1

/* Will be added where memories needs to be aligned (with -Os data might not be aligned to boundary by default).
 * E.g. __attribute__((aligned(4))) */
#define LV_ATTRIBUTE_MEM_ALIGN

/* Attribute to mark large constant arrays for example font's bitmaps */
#define LV_ATTRIBUTE_LARGE_CONST

/* Compiler prefix for a big array declaration in RAM */
#define LV_ATTRIBUTE_LARGE_RAM_ARRAY

/* Place performance critical functions into a faster memory (e.g RAM) */
#define LV_ATTRIBUTE_FAST_MEM

/* Prefix variables that are used in GPU accelerated operations, often these need to be placed in RAM sections that are DMA accessible */
#define LV_ATTRIBUTE_DMA

/* Export integer constant to binding. This macro is used with constants in the form of LV_<CONST> that
 * should also appear on LVGL binding API such as Micropython. */
#define LV_EXPORT_CONST_INT(int_value) struct _silence_gcc_warning  /* The default value just prevents GCC warning */

/* Extend the default -32k..32k coordinate range to -4M..4M by using int32_t for coordinates instead of int16_t */
#define LV_USE_LARGE_COORD 0

/*==================
 *   FONT USAGE
 *===================*/
/* Montserrat fonts with ASCII range and some symbols using bpp = 4
 * https://fonts.google.com/specimen/Montserrat */
#define LV_FONT_MONTSERRAT_8  0
#define LV_FONT_MONTSERRAT_10 0
#define LV_FONT_MONTSERRAT_12 0
#define LV_FONT_MONTSERRAT_14 1
#define LV_FONT_MONTSERRAT_16 1
#define LV_FONT_MONTSERRAT_18 0
#define LV_FONT_MONTSERRAT_20 1
#define LV_FONT_MONTSERRAT_22 0
#define LV_FONT_MONTSERRAT_24 0
#define LV_FONT_MONTSERRAT_26 0
#define LV_FONT_MONTSERRAT_28 1
#define LV_FONT_MONTSERRAT_30 0
#define LV_FONT_MONTSERRAT_32 0
#define LV_FONT_MONTSERRAT_34 0
#define LV_FONT_MONTSERRAT_36 0
#define LV_FONT_MONTSERRAT_38 0
#define LV_FONT_MONTSERRAT_40 0
#define LV_FONT_MONTSERRAT_42 0
#define LV_FONT_MONTSERRAT_44 0
#define LV_FONT_MONTSERRAT_46 0
#define LV_FONT_MONTSERRAT_48 0

/* Demonstrate special features */
#define LV_FONT_MONTSERRAT_28_COMPRESSED 0  /* bpp = 3 */
#define LV_FONT_DEJAVU_16_PERSIAN_HEBREW 0  /* Hebrew, Arabic, Persian letters and all their forms */
#define LV_FONT_SIMSUN_16_CJK            0  /* 1000 most common CJK radicals */

/* Pixel perfect monospace fonts */
#define LV_FONT_UNSCII_8  0
#define LV_FONT_UNSCII_16 0

/* Optionally declare custom fonts here.
 * You can use these fonts as default font too and they will be available globally.
 * E.g. #define LV_FONT_CUSTOM_DECLARE  LV_FONT_DECLARE(my_font_1) LV_FONT_DECLARE(my_font_2) */
#define LV_FONT_CUSTOM_DECLARE

/* Always set a default font */
#define LV_FONT_DEFAULT &lv_font_montserrat_14

/* Enable handling large font and/or fonts with a lot of characters.
 * The limit depends on the font size, font face and bpp.
 * Compiler error will be triggered if a font needs it. */
#define LV_FONT_FMT_TXT_LARGE 0

/* Enables/disables support for compressed fonts. */
#define LV_USE_FONT_COMPRESSED 0

/* Enable subpixel rendering */
#define LV_USE_FONT_SUBPX 0
#if LV_USE_FONT_SUBPX
    /* Set the pixel order of the display. Physical order of RGB channels. Doesn't matter with "normal" fonts. */
    #define LV_FONT_SUBPX_BGR 0  /* 0: RGB; 1:BGR order */
#endif

/*=================
 *  TEXT SETTINGS
 *=================*/
/**
 * Select a character encoding for strings.
 * Your IDE or editor should have the same character encoding
 * - LV_TXT_ENC_UTF8
 * - LV_TXT_ENC_ASCII
 */
#define LV_TXT_ENC LV_TXT_ENC_UTF8

/* Can break (wrap) texts on these chars */
#define LV_TXT_BREAK_CHARS " ,.;:-_"

/* If a word is at least this long, will break wherever "prettiest"
 * To disable, set to a value <= 0 */
#define LV_TXT_LINE_BREAK_LONG_LEN 0

/* Minimum number of characters in a long word to put on a line before a break.
 * Depends on LV_TXT_LINE_BREAK_LONG_LEN. */
#define LV_TXT_LINE_BREAK_LONG_PRE_MIN_LEN 3

/* Minimum number of characters in a long word to put on a line after a break.
 * Depends on LV_TXT_LINE_BREAK_LONG_LEN. */
#define LV_TXT_LINE_BREAK_LONG_POST_MIN_LEN 3

/* The control character to use for signalling text recoloring. */
#define LV_TXT_COLOR_CMD "#"

/* Support bidirectional texts. Allows mixing Left-to-Right and Right-to-Left texts.
 * The direction will be processed according to the Unicode Bidirectional Algorithm:
 * https://www.w3.org/International/articles/inline-bidi-markup/uba-basics*/
#define LV_USE_BIDI 0
#if LV_USE_BIDI
    /* Set the default direction. Supported values:
     * `LV_BASE_DIR_LTR` Left-to-Right
     * `LV_BASE_DIR_RTL` Right-to-Left
     * `LV_BASE_DIR_AUTO` detect texts base direction */
    #define LV_BIDI_BASE_DIR_DEF LV_BASE_DIR_AUTO
#endif

/* Enable Arabic/Persian processing
 * In these languages characters should be replaced with an other form based on their position in the text */
#define LV_USE_ARABIC_PERSIAN_CHARS 0

/*==================
 *  WIDGET USAGE
 *================*/
#define LV_USE_ARC        1
#define LV_USE_BAR        1
#define LV_USE_BTN        1
#define LV_USE_BTNMATRIX  0
#define LV_USE_CANVAS     0
#define LV_USE_CHECKBOX   0
#define LV_USE_CHART      1
#define LV_USE_DROPDOWN   0
#define LV_USE_IMG        0
#define LV_USE_LABEL      1
#define LV_USE_LINE       0
#define LV_USE_ROLLER     0
#define LV_USE_SLIDER     1
#define LV_USE_SWITCH     1
#define LV_USE_TEXTAREA   0
#define LV_USE_TABLE      0

/*==================
 * EXTRA COMPONENTS
 *==================*/
/*-----------
 * Widgets
 *----------*/
#define LV_USE_ANIMIMG    0
#define LV_USE_CALENDAR   0
#define LV_USE_COLORWHEEL 0
#define LV_USE_IMGBTN     0
#define LV_USE_KEYBOARD   0
#define LV_USE_LED        0
#define LV_USE_LIST       1
#define LV_USE_MENU       0
#define LV_USE_METER      0
#define LV_USE_MSGBOX     0
#define LV_USE_SPAN       0
#define LV_USE_SPINBOX    0
#define LV_USE_SPINNER    0
#define LV_USE_TABVIEW    0
#define LV_USE_TILEVIEW   0
#define LV_USE_WIN        0

/*-----------
 * Themes
 *----------*/
/* A simple, impressive and very complete theme */
#define LV_USE_THEME_DEFAULT 1
#if LV_USE_THEME_DEFAULT
    /* 0: Light mode; 1: Dark mode */
    #define LV_THEME_DEFAULT_DARK 1
    /* 1: Enable grow on press */
    #define LV_THEME_DEFAULT_GROW 0
    /* Default transition time in [ms] */
    #define LV_THEME_DEFAULT_TRANSITION_TIME 80
#endif /* LV_USE_THEME_DEFAULT */

/* A very simple theme that is a good starting point for a custom theme */
#define LV_USE_THEME_BASIC 0

/* A theme designed for monochrome displays */
#define LV_USE_THEME_MONO 0

/*-----------
 * Layouts
 *----------*/
/* A layout similar to Flexbox in CSS. */
#define LV_USE_FLEX 1

/* A layout similar to Grid in CSS. */
#define LV_USE_GRID 0

/*---------------------
 * 3rd party libraries
 *--------------------*/
/* File system interfaces for common APIs */
#define LV_USE_FS_STDIO 0
#define LV_USE_FS_POSIX 0
#define LV_USE_FS_WIN32 0
#define LV_USE_FS_FATFS 0

/* PNG decoder library */
#define LV_USE_PNG 0

/* BMP decoder library */
#define LV_USE_BMP 0

/* JPG + split JPG decoder library */
#define LV_USE_SJPG 0

/* GIF decoder library */
#define LV_USE_GIF 0

/* QR code library */
#define LV_USE_QRCODE 0

/* FreeType library */
#define LV_USE_FREETYPE 0

/* Rlottie library */
#define LV_USE_RLOTTIE 0

/* Enable API to take snapshot for object */
#define LV_USE_SNAPSHOT 0

/* Enable Monkey test */
#define LV_USE_MONKEY 0

/* Enable grid navigation */
#define LV_USE_GRIDNAV 0

/* Enable lv_obj fragment */
#define LV_USE_FRAGMENT 0

/* Enable an object property API */
#define LV_USE_OBJ_PROPERTY 0

/* Enable IME (Input Method Editor) */
#define LV_USE_IME_PINYIN 0

/*==================
* EXAMPLES
*==================*/
/* Enable the examples to be built with the library */
#define LV_BUILD_EXAMPLES 0

/*===================
 * TEST USAGE
 *==================*/
#define LV_USE_TEST 0

#define LV_USE_XML 0

#endif /* End of "Content enable" */

#endif /* LV_CONF_H */
