#pragma once

/**
 * 电源状态机（design.md D7）
 * 状态和事件均按计划定义。
 */

typedef enum {
    PSU_OFF = 0,      // 电源关闭状态
    PSU_STANDBY,      // 待机状态
    PSU_STARTING,     // 启动中状态
    PSU_ON,           // 已上电运行状态
    PSU_STOPPING,     // 关机过程中状态
    PSU_FAULT         // 故障锁定状态
} PsuState;

typedef enum {
    EVT_BOOT = 0,         // 系统上电启动事件
    EVT_KEY_SHORT,        // 电源键短按事件
    EVT_KEY_LONG,         // 电源键长按事件
    EVT_PWOK_HIGH,        // PWOK 变高事件，表示电源正常
    EVT_PWOK_LOW,         // PWOK 变低事件，表示电源异常
    EVT_PWOK_LOST_100MS,   // PWOK 持续丢失 100ms 事件
    EVT_TIMEOUT_1S,       // 启动超时 1s 事件
    EVT_FAULT_RESET       // 故障复位事件
} PsuFsmEvent;

/**
 * @brief 根据当前状态和事件计算下一步 PSU 状态。
 *
 * 非法状态迁移会保持当前状态不变。
 *
 * @param state  当前 PSU 状态
 * @param event  输入事件
 * @return 下一步 PSU 状态
 */
PsuState psu_fsm_transition(PsuState state, PsuFsmEvent event);
