#if 0  // legacy UI - replaced by ui/ subproject
#pragma once
/**
 * @file ui_menu.h
 * @brief 设置菜单界面
 *
 * 职责：
 *  - 创建设置菜单屏幕（风扇曲线参数查看、系统信息）
 *  - 提供菜单导航接口（上/下/确认）
 *
 * 约束：
 *  - 仅在 esp32dev env 编译
 *  - 菜单项静态定义，不动态增删
 */

#include <lvgl.h>

namespace ui_menu {

/**
 * @brief 初始化设置菜单界面
 *
 * 必须在 lvgl_port::init() 之后调用。
 */
void init();

/**
 * @brief 菜单向上导航
 */
void navigate_up();

/**
 * @brief 菜单向下导航
 */
void navigate_down();

/**
 * @brief 确认当前菜单项
 */
void confirm();

/**
 * @brief 获取菜单屏幕对象（供 ui_events 切换屏幕使用）
 */
lv_obj_t* get_screen();

} // namespace ui_menu

#endif // legacy UI
