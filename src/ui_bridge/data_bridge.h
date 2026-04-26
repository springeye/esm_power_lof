#pragma once
struct _lv_obj_t;
namespace ui_bridge {
void data_bridge_init();                              // 创建 200ms LVGL timer
void data_bridge_attach(struct _lv_obj_t* home);      // 缓存 5 个 label 句柄
}
