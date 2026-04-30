#pragma once
struct _lv_obj_t;
namespace ui_bridge {
void data_bridge_init();                              // 创建 200ms LVGL timer
void data_bridge_attach(struct _lv_obj_t* home);      // 保留兼容，不再缓存 label 指针
}
