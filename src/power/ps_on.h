#pragma once

/**
 * @brief 初始化 PS_ON 和 PW_OK GPIO 引脚。
 *
 * PS_ON（GPIO27）：输出，HIGH 表示关闭（ATX 约定）
 * PW_OK（GPIO34）：输入，无内部上拉（仅输入引脚）
 */
void ps_on_init(void);

/**
 * @brief 置位 PS_ON（开启 ATX 电源）。
 * 将 PS_ON 拉低。
 */
void ps_on_assert(void);

/**
 * @brief 取消置位 PS_ON（关闭 ATX 电源）。
 * 将 PS_ON 拉高。
 */
void ps_on_deassert(void);

/**
 * @brief 读取 PW_OK 信号。
 * @return PW_OK 为高电平时返回 true（电源正常）
 */
bool ps_on_pwok_read(void);
