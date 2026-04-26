#pragma once
/**
 * @file i2c_bus.h
 * @brief ESP32 风扇控制器的 I2C 硬件抽象层
 *
 * 对 Wire 库进行封装，并通过互斥锁保护多任务访问。
 * 提供初始化、扫描以及互斥锁加解锁接口。
 */

#include <Arduino.h>
#include <Wire.h>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 使用 app_config.h 中配置的引脚和频率初始化 I2C 总线
 *
 * 调用 Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ_HZ)。
 * 创建 FreeRTOS 互斥锁，用于线程安全访问。
 * 必须在任何 I2C 设备初始化之前调用。
 */
void i2c_bus_init(void);

/**
 * @brief 扫描 I2C 总线并把发现的设备地址输出到串口
 *
 * 调试工具。打印所有响应设备的地址。
 * 该函数本身不具备线程安全性，仅应在初始化阶段调用。
 */
void i2c_scan(void);

/**
 * @brief 获取 I2C 总线互斥锁（阻塞等待）
 *
 * 在任何直接 Wire 事务前必须调用。
 * 使用后必须与 i2c_bus_give() 配对释放。
 *
 * @param timeout_ms 最大等待时间，单位毫秒
 * @return 成功获取互斥锁返回 true，超时返回 false
 */
bool i2c_bus_take(uint32_t timeout_ms);

/**
 * @brief 释放 I2C 总线互斥锁
 *
 * 在 i2c_bus_take() 后完成一次传输时必须调用。
 */
void i2c_bus_give(void);

/**
 * @brief 获取 Wire 实例（仅在持有互斥锁时使用）
 *
 * @return 返回 Wire 的 TwoWire 实例引用
 */
TwoWire& i2c_get_wire(void);

#ifdef __cplusplus
}
#endif
