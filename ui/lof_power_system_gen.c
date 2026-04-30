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

lv_font_t * hos_bold_big;
extern lv_font_t hos_bold_big_data;
lv_font_t * hos_regular;
extern lv_font_t hos_regular_data;
lv_font_t * hos_14;
extern lv_font_t hos_14_data;

/*----------------
 * Images
 *----------------*/

/*----------------
 * Global styles
 *----------------*/

/*----------------
 * Subjects
 *----------------*/

lv_subject_t system_name;
lv_subject_t system_state;
lv_subject_t device_current_power;
lv_subject_t uptime;
lv_subject_t wh;
lv_subject_t device_power;
lv_subject_t device_power_percent;
lv_subject_t device_power_percent_txt;
lv_subject_t ch1_voltage;
lv_subject_t ch1_current;
lv_subject_t ch1_pwer;
lv_subject_t ch2_voltage;
lv_subject_t ch2_current;
lv_subject_t ch2_pwer;
lv_subject_t ch3_voltage;
lv_subject_t ch3_current;
lv_subject_t ch3_pwer;
lv_subject_t device_temp;
lv_subject_t fan_percent;

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

    /* get font 'hos_bold_big' from a C array */
    hos_bold_big = &hos_bold_big_data;
    /* get font 'hos_regular' from a C array */
    hos_regular = &hos_regular_data;
    /* get font 'hos_14' from a C array */
    hos_14 = &hos_14_data;


    /*----------------
     * Images
     *----------------*/
    /*----------------
     * Global styles
     *----------------*/

    /*----------------
     * Subjects
     *----------------*/
    static char system_name_buf[UI_SUBJECT_STRING_LENGTH];
    static char system_name_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&system_name,
                           system_name_buf,
                           system_name_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "LOF电源系统"
                          );
    static char system_state_buf[UI_SUBJECT_STRING_LENGTH];
    static char system_state_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&system_state,
                           system_state_buf,
                           system_state_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "运行中"
                          );
    static char device_current_power_buf[UI_SUBJECT_STRING_LENGTH];
    static char device_current_power_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&device_current_power,
                           device_current_power_buf,
                           device_current_power_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "功率: 605.00w"
                          );
    static char uptime_buf[UI_SUBJECT_STRING_LENGTH];
    static char uptime_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&uptime,
                           uptime_buf,
                           uptime_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "运行: 00:00:00"
                          );
    static char wh_buf[UI_SUBJECT_STRING_LENGTH];
    static char wh_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&wh,
                           wh_buf,
                           wh_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "功耗：125.21Wh"
                          );
    lv_subject_init_int(&device_power, 750);
    lv_subject_init_int(&device_power_percent, 31);
    static char device_power_percent_txt_buf[UI_SUBJECT_STRING_LENGTH];
    static char device_power_percent_txt_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&device_power_percent_txt,
                           device_power_percent_txt_buf,
                           device_power_percent_txt_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "31%"
                          );
    static char ch1_voltage_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch1_voltage_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch1_voltage,
                           ch1_voltage_buf,
                           ch1_voltage_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char ch1_current_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch1_current_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch1_current,
                           ch1_current_buf,
                           ch1_current_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char ch1_pwer_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch1_pwer_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch1_pwer,
                           ch1_pwer_buf,
                           ch1_pwer_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char ch2_voltage_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch2_voltage_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch2_voltage,
                           ch2_voltage_buf,
                           ch2_voltage_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char ch2_current_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch2_current_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch2_current,
                           ch2_current_buf,
                           ch2_current_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char ch2_pwer_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch2_pwer_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch2_pwer,
                           ch2_pwer_buf,
                           ch2_pwer_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char ch3_voltage_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch3_voltage_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch3_voltage,
                           ch3_voltage_buf,
                           ch3_voltage_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char ch3_current_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch3_current_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch3_current,
                           ch3_current_buf,
                           ch3_current_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char ch3_pwer_buf[UI_SUBJECT_STRING_LENGTH];
    static char ch3_pwer_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&ch3_pwer,
                           ch3_pwer_buf,
                           ch3_pwer_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "0.000"
                          );
    static char device_temp_buf[UI_SUBJECT_STRING_LENGTH];
    static char device_temp_prev_buf[UI_SUBJECT_STRING_LENGTH];
    lv_subject_init_string(&device_temp,
                           device_temp_buf,
                           device_temp_prev_buf,
                           UI_SUBJECT_STRING_LENGTH,
                           "36.0℃"
                          );
    lv_subject_init_float(&fan_percent, 0.2);

    /*----------------
     * Translations
     *----------------*/

#if LV_USE_XML
    /* Register widgets */

    /* Register fonts */
    lv_xml_register_font(NULL, "hos_bold_big", hos_bold_big);
    lv_xml_register_font(NULL, "hos_regular", hos_regular);
    lv_xml_register_font(NULL, "hos_14", hos_14);

    /* Register subjects */
    lv_xml_register_subject(NULL, "system_name", &system_name);
    lv_xml_register_subject(NULL, "system_state", &system_state);
    lv_xml_register_subject(NULL, "device_current_power", &device_current_power);
    lv_xml_register_subject(NULL, "uptime", &uptime);
    lv_xml_register_subject(NULL, "wh", &wh);
    lv_xml_register_subject(NULL, "device_power", &device_power);
    lv_xml_register_subject(NULL, "device_power_percent", &device_power_percent);
    lv_xml_register_subject(NULL, "device_power_percent_txt", &device_power_percent_txt);
    lv_xml_register_subject(NULL, "ch1_voltage", &ch1_voltage);
    lv_xml_register_subject(NULL, "ch1_current", &ch1_current);
    lv_xml_register_subject(NULL, "ch1_pwer", &ch1_pwer);
    lv_xml_register_subject(NULL, "ch2_voltage", &ch2_voltage);
    lv_xml_register_subject(NULL, "ch2_current", &ch2_current);
    lv_xml_register_subject(NULL, "ch2_pwer", &ch2_pwer);
    lv_xml_register_subject(NULL, "ch3_voltage", &ch3_voltage);
    lv_xml_register_subject(NULL, "ch3_current", &ch3_current);
    lv_xml_register_subject(NULL, "ch3_pwer", &ch3_pwer);
    lv_xml_register_subject(NULL, "device_temp", &device_temp);
    lv_xml_register_subject(NULL, "fan_percent", &fan_percent);

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