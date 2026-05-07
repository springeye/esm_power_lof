#pragma once

#include <cstdint>

struct _lv_obj_t;

/**
 * 主页视图枚举
 * VIEW_DEFAULT: 默认三通道显示
 * VIEW_CHART_CH1/CH2/CH3: 对应通道的功率折线图
 */
enum HomeView : uint8_t {
    VIEW_DEFAULT = 0,
    VIEW_CHART_CH1 = 1,
    VIEW_CHART_CH2 = 2,
    VIEW_CHART_CH3 = 3,
    VIEW_COUNT = 4
};

namespace view_manager {

/**
 * @brief 初始化视图管理器
 * @param home_root 主页根对象（由 screen_manager 提供）
 * @note 在 screen_manager_init 中 g_home 创建后调用
 */
void view_manager_init(struct _lv_obj_t* home_root);

/**
 * @brief 切换到指定视图
 * @param view 目标视图
 */
void view_manager_switch_to(HomeView view);

/**
 * @brief 获取当前视图
 * @return 当前视图枚举值
 */
HomeView view_manager_get_current();

/**
 * @brief 循环切换视图
 * @param delta +1 向右循环（默认→CH1→CH2→CH3→默认），-1 向左循环
 */
void view_manager_cycle(int8_t delta);

/**
 * @brief 获取指定通道的图表容器（供 chart_view 使用）
 * @param ch 通道索引 (0-2)
 * @return 图表容器对象指针，无效索引返回 nullptr
 */
struct _lv_obj_t* view_manager_get_chart_container(uint8_t ch);

/**
 * @brief 获取三通道容器（默认视图使用）
 * @return 三通道容器对象指针
 */
struct _lv_obj_t* view_manager_get_channels_container();

} // namespace view_manager
