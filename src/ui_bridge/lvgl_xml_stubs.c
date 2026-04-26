/**
 * @file lvgl_xml_stubs.c
 * @brief Weak symbol stubs for LVGL XML runtime APIs.
 *
 * The ui/ subproject is exported by LVGL Editor and contains calls to
 * lv_xml_register_font() in lof_power_system_gen.c. We disable LV_USE_XML
 * (see include/lv_conf.h) but still need these symbols at link time.
 *
 * These stubs are __attribute__((weak)) so that if LV_USE_XML is later
 * enabled, the real implementations from LVGL automatically override them.
 */
#include "lvgl/lvgl.h"

typedef struct _lv_xml_component_scope_t lv_xml_component_scope_t;

__attribute__((weak))
int32_t lv_xml_register_font(lv_xml_component_scope_t * scope,
                             const char * name,
                             const lv_font_t * font)
{
    (void)scope; (void)name; (void)font;
    return 0;  /* LV_RESULT_OK equivalent; ui/ ignores return value */
}
