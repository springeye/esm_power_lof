#include "splash_anim.h"

namespace {

void set_text_opa(void* obj, int32_t v) {
    lv_obj_set_style_text_opa(static_cast<lv_obj_t*>(obj), static_cast<lv_opa_t>(v), 0);
}
void set_translate_y(void* obj, int32_t v) {
    lv_obj_set_style_translate_y(static_cast<lv_obj_t*>(obj), v, 0);
}
void set_bar_value(void* obj, int32_t v) {
    lv_bar_set_value(static_cast<lv_obj_t*>(obj), v, LV_ANIM_OFF);
}

void start_anim(lv_obj_t* target,
                lv_anim_exec_xcb_t exec_cb,
                int32_t v_start,
                int32_t v_end,
                uint32_t delay_ms,
                uint32_t duration_ms,
                lv_anim_path_cb_t path_cb = lv_anim_path_ease_out) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, target);
    lv_anim_set_exec_cb(&a, exec_cb);
    lv_anim_set_values(&a, v_start, v_end);
    lv_anim_set_delay(&a, delay_ms);
    lv_anim_set_duration(&a, duration_ms);
    lv_anim_set_path_cb(&a, path_cb);
    lv_anim_start(&a);
}

}

namespace ui_bridge {

void splash_play_intro(lv_obj_t* root) {
    if (root == nullptr) return;

    lv_obj_t* bolt     = lv_obj_get_child(root, 0);
    lv_obj_t* brand    = lv_obj_get_child(root, 1);
    lv_obj_t* zap_line = lv_obj_get_child(root, 2);
    lv_obj_t* slogan   = lv_obj_get_child(root, 3);

    if (!bolt || !brand || !zap_line || !slogan) return;

    lv_obj_set_style_text_opa(bolt, 0, 0);
    lv_obj_set_style_text_opa(brand, 0, 0);
    lv_obj_set_style_translate_y(brand, 20, 0);
    lv_bar_set_value(zap_line, 0, LV_ANIM_OFF);
    lv_obj_set_style_text_opa(slogan, 0, 0);

    // Logo + 品牌淡入（前 800ms）
    start_anim(bolt,     set_text_opa,    0,  255, 0,   400);
    start_anim(brand,    set_text_opa,    0,  255, 300, 500);
    start_anim(brand,    set_translate_y, 20, 0,   300, 500);
    start_anim(slogan,   set_text_opa,    0,  255, 300, 400);

    // 进度条 0→100，从 200ms 开始耗时 1300ms，1500ms 时刚好满格
    // 用 linear path 让进度匀速增长，更符合"加载完成"的视觉直觉
    start_anim(zap_line, set_bar_value,   0,  100, 200, 1300, lv_anim_path_linear);
}

}
