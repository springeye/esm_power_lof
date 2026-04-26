#pragma once
#include <cstdint>

/**
 * INA226 电源数据（design.md D6）
 * 三路 INA226 监测同一直流输出电压轨的三个并联输出接口
 */
typedef struct {
    float voltage_v;   // 输出电压（V），三路相同
    float current_a;   // 本路输出电流（A）
    float power_w;     // 本路输出功率（W）= voltage × current
    bool  valid;       // 本次读取是否成功，false 表示 I2C 通信失败
} Ina226Data;

/**
 * 通道索引，对应 INA226_ADDR_CH[] 中的地址
 * 三路并联同一电压轨，分别计量各输出接口电流
 */
typedef enum {
    INA_CH1 = 0,  // 输出接口 1（地址 0x40）
    INA_CH2 = 1,  // 输出接口 2（地址 0x41）
    INA_CH3 = 2   // 输出接口 3（地址 0x44）
} Ina226Rail;

/**
 * @brief 初始化全部 3 个 INA226 传感器。
 *
 * 为每个传感器配置分流电阻和最大电流量程。
 * 必须在 i2c_bus_init() 之后调用。
 */
void ina226_init_all(void);

/**
 * @brief 读取一路 INA226 的电源数据。
 *
 * @param rail   通道索引（INA_CH1/CH2/CH3）
 * @param out    输出数据结构体指针
 * @return 读取成功返回 true，I2C 出错返回 false
 */
bool ina226_read(Ina226Rail rail, Ina226Data *out);
