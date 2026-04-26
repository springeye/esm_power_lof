#pragma once
/**
 * @file spi_bus.h
 * @brief ESP32 风扇控制器的 SPI 硬件抽象层
 *
 * SPI 总线由 TFT_eSPI 库内部管理。
 * 本模块提供文档说明、互斥锁预留，
 * 以及未来新增 SPI 设备的统一初始化入口。
 *
 * 注意：不要在此处重新初始化 SPI；TFT_eSPI 会管理 VSPI/HSPI。
 */

#include <Arduino.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 初始化 SPI 硬件抽象层（仅创建互斥锁；总线由 TFT_eSPI 管理）
 *
 * 创建 FreeRTOS 互斥锁，供未来多 SPI 设备仲裁使用。
 * 必须在 tft_driver_init() 之前调用。
 */
void spi_bus_init(void);

/**
 * @brief 获取 SPI 总线互斥锁（阻塞等待）
 *
 * 预留给未来多设备 SPI 仲裁使用。
 * 当前仅 TFT_eSPI 使用该 SPI 总线。
 *
 * @param timeout_ms 最大等待时间，单位毫秒
 * @return 成功获取互斥锁返回 true，超时返回 false
 */
bool spi_bus_take(uint32_t timeout_ms);

/**
 * @brief 释放 SPI 总线互斥锁
 */
void spi_bus_give(void);

#ifdef __cplusplus
}
#endif
