#if 0  // legacy UI - replaced by ui/ subproject
#pragma once
/**
 * @file ui_main.h
 * @brief 主仪表盘界面 + 全局样式定义
 *
 * 职责：
 *  - 创建主仪表盘屏幕（温度、风扇转速、电源状态、电流）
 *  - 定义全局 LVGL 样式（颜色、字体、间距）
 *  - 提供数据刷新接口（由 lvglTask 调用）
 *
 * 约束：
 *  - 仅在 esp32dev env 编译
 *  - 所有 LVGL 对象在 ui_main::init() 中一次性创建，不动态增删
 *  - 不使用中文字体
 */

#include <lvgl.h>
#include <cstdint>

namespace ui_main {

/**
 * @brief 初始化主仪表盘界面
 *
 * 创建所有 LVGL 对象并应用样式。
 * 必须在 lvgl_port::init() 之后调用。
 */
void init();

/**
 * @brief 刷新温度显示
 * @param temp_c 温度（°C）
 */
void update_temperature(float temp_c);

/**
 * @brief 刷新风扇转速显示
 * @param rpm 转速（RPM）
 */
void update_fan_rpm(uint32_t rpm);

/**
 * @brief 刷新电源状态显示
 * @param state_str 状态字符串（如 "ON", "OFF", "FAULT"）
 */
void update_psu_state(const char* state_str);

/**
 * @brief 刷新电流显示（3路）
 * @param load_a  负载电流（A）
 * @param v12_a   12V 电流（A）
 * @param v5_a    5V 电流（A）
 */
void update_current(float load_a, float v12_a, float v5_a);

/**
 * @brief 获取主仪表盘屏幕对象（供 ui_events 切换屏幕使用）
 */
lv_obj_t* get_screen();

} // namespace ui_main

#endif // legacy UI
