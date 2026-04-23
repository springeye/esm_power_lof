/**
 * @file lof_power_system_gen.c
 */

/*********************
 *      INCLUDES
 *********************/

#include "lof_power_system_gen.h"

#if LV_USE_XML
#endif /* LV_USE_XML */

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/*----------------
 * Translations
 *----------------*/

/**********************
 *  GLOBAL VARIABLES
 **********************/

/*--------------------
 *  Permanent screens
 *-------------------*/

/*----------------
 * Fonts
 *----------------*/

lv_font_t * hos_black;
extern lv_font_t hos_black_data;
lv_font_t * hos_bold;
extern lv_font_t hos_bold_data;
lv_font_t * hos_bold_big;
extern lv_font_t hos_bold_big_data;
lv_font_t * hos_light;
extern lv_font_t hos_light_data;
lv_font_t * hos_medium;
extern lv_font_t hos_medium_data;
lv_font_t * hos_regular;
extern lv_font_t hos_regular_data;
lv_font_t * hos_thin;
extern lv_font_t hos_thin_data;

/*----------------
 * Images
 *----------------*/

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Subjects
 *----------------*/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

void lof_power_system_init_gen(const char * asset_path)
{
    char buf[256];


    /*----------------
     * Fonts
     *----------------*/

    /* get font 'hos_black' from a C array */
    hos_black = &hos_black_data;
    /* get font 'hos_bold' from a C array */
    hos_bold = &hos_bold_data;
    /* get font 'hos_bold_big' from a C array */
    hos_bold_big = &hos_bold_big_data;
    /* get font 'hos_light' from a C array */
    hos_light = &hos_light_data;
    /* get font 'hos_medium' from a C array */
    hos_medium = &hos_medium_data;
    /* get font 'hos_regular' from a C array */
    hos_regular = &hos_regular_data;
    /* get font 'hos_thin' from a C array */
    hos_thin = &hos_thin_data;


    /*----------------
     * Images
     *----------------*/
    /*----------------
     * Global styles
     *----------------*/

    /*----------------
     * Subjects
     *----------------*/
    /*----------------
     * Translations
     *----------------*/

#if LV_USE_XML
    /* Register widgets */

    /* Register fonts */
    lv_xml_register_font(NULL, "hos_black", hos_black);
    lv_xml_register_font(NULL, "hos_bold", hos_bold);
    lv_xml_register_font(NULL, "hos_bold_big", hos_bold_big);
    lv_xml_register_font(NULL, "hos_light", hos_light);
    lv_xml_register_font(NULL, "hos_medium", hos_medium);
    lv_xml_register_font(NULL, "hos_regular", hos_regular);
    lv_xml_register_font(NULL, "hos_thin", hos_thin);

    /* Register subjects */

    /* Register callbacks */
#endif

    /* Register all the global assets so that they won't be created again when globals.xml is parsed.
     * While running in the editor skip this step to update the preview when the XML changes */
#if LV_USE_XML && !defined(LV_EDITOR_PREVIEW)
    /* Register images */
#endif

#if LV_USE_XML == 0
    /*--------------------
     *  Permanent screens
     *-------------------*/
    /* If XML is enabled it's assumed that the permanent screens are created
     * manaully from XML using lv_xml_create() */
#endif
}

/* Callbacks */

/**********************
 *   STATIC FUNCTIONS
 **********************/