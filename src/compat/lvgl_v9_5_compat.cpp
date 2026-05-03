/**
 * @file lvgl_v9_5_compat.cpp
 * @brief LVGL 9.5.x compat shims for LVGL 9.2.2 (native build).
 *
 * The UI sub-project (exported from LVGL Editor 9.5.x) uses APIs that do not
 * exist in LVGL 9.2.2:
 *   - lv_subject_init_float / lv_subject_set_float
 *   - lv_bar_bind_value
 *
 * These shims provide working implementations compatible with 9.2.2 internals.
 */

#include "lvgl/lvgl.h"

#ifdef LV_USE_OBSERVER

#include <string.h>

extern "C" {

/* Internal lv_observer_t layout (stable across LVGL 9.x). */
struct lv_observer_shim {
    lv_subject_t * subject;
    lv_observer_cb_t cb;
    void * target;
    void * user_data;
    uint32_t auto_free_user_data : 1;
    uint32_t notified : 1;
    uint32_t for_obj : 1;
};
#define LV_OBSERVER_SZ sizeof(struct lv_observer_shim)

void lv_subject_init_float(lv_subject_t * subject, float value)
{
    lv_memzero(subject, sizeof(lv_subject_t));
    subject->type = LV_SUBJECT_TYPE_INT;
    union { float f; int32_t i; } u = { .f = value };
    subject->value.num = u.i;
    subject->prev_value.num = u.i;
    lv_ll_init(&(subject->subs_ll), LV_OBSERVER_SZ);
}

void lv_subject_set_float(lv_subject_t * subject, float value)
{
    if (subject->type != LV_SUBJECT_TYPE_INT) {
        LV_LOG_WARN("Subject type is not LV_SUBJECT_TYPE_INT");
        return;
    }
    subject->prev_value.num = subject->value.num;
    union { float f; int32_t i; } u = { .f = value };
    subject->value.num = u.i;
    lv_subject_notify(subject);
}

#if LV_USE_BAR

static void bar_value_changed_event_cb(lv_event_t * e)
{
    lv_obj_t * bar = (lv_obj_t *)lv_event_get_current_target(e);
    lv_subject_t * subject = (lv_subject_t *)lv_event_get_user_data(e);
    lv_subject_set_int(subject, lv_bar_get_value(bar));
}

static void bar_value_observer_cb(lv_observer_t * observer, lv_subject_t * subject)
{
    struct lv_observer_shim * obs = (struct lv_observer_shim *)observer;
    lv_bar_set_value((lv_obj_t *)obs->target, subject->value.num, LV_ANIM_OFF);
}

lv_observer_t * lv_bar_bind_value(lv_obj_t * obj, lv_subject_t * subject)
{
    if (subject->type != LV_SUBJECT_TYPE_INT) {
        LV_LOG_WARN("Incompatible subject type: %d", (int)subject->type);
        return NULL;
    }

    lv_obj_add_event_cb(obj, bar_value_changed_event_cb, LV_EVENT_VALUE_CHANGED, subject);

    lv_observer_t * observer = lv_subject_add_observer_obj(subject, bar_value_observer_cb, obj, NULL);
    return observer;
}

#endif /* LV_USE_BAR */

} /* extern "C" */

#endif /* LV_USE_OBSERVER */

/* lv_style_set_margin_all: LVGL 9.5.x API, not in 9.2.2.
 * Forward-declared in include/lvgl/lvgl.h, implemented here. */
extern "C" void lv_style_set_margin_all(lv_style_t * style, int32_t value)
{
    lv_style_set_pad_all(style, value);
}

/* lv_obj_set_name: LVGL 9.5.x (LV_USE_OBJ_NAME) API, not in 9.2.2.
 * No-op shim — name lookup not used at runtime; child access uses idx. */
extern "C" void lv_obj_set_name(lv_obj_t * obj, const char * name)
{
    (void)obj;
    (void)name;
}
