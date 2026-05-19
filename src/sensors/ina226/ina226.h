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

/**
 * @brief 设置某路的线性校准增益。
 *
 * 修正后电流 = 原始电流 × gain + offset。增益由调用方（config_manager）clamp
 * 后传入，默认 1.0。线程安全：仅写单个 atomic，传感器任务读取无需加锁。
 *
 * @param rail 通道索引（INA_CH1/CH2/CH3）
 * @param gain 校准增益系数
 */
void ina226_set_gain(Ina226Rail rail, float gain);

/**
 * @brief 读取某路当前生效的校准增益。
 *
 * @param rail 通道索引（INA_CH1/CH2/CH3）
 * @return 当前增益，rail 越界返回 1.0
 */
float ina226_get_gain(Ina226Rail rail);

/**
 * @brief 设置某路的线性校准偏移（A）。
 *
 * 修正后电流 = 原始电流 × gain + offset。偏移由调用方 clamp 后传入，默认 0.0。
 *
 * @param rail   通道索引（INA_CH1/CH2/CH3）
 * @param offset 校准偏移（A）
 */
void ina226_set_offset(Ina226Rail rail, float offset);

/**
 * @brief 读取某路当前生效的校准偏移（A）。
 *
 * @param rail 通道索引（INA_CH1/CH2/CH3）
 * @return 当前偏移，rail 越界返回 0.0
 */
float ina226_get_offset(Ina226Rail rail);

/**
 * @brief 读取某路最近一次采样的未校准原始电流（A）。
 *
 * 由 ina226_read() 在每次成功采样时缓存（应用增益/偏移之前的值）。
 * 双点校准采集校准点时使用，避免把已校准值再次代入解算。
 *
 * @param rail 通道索引（INA_CH1/CH2/CH3）
 * @return 最近一次原始电流，rail 越界或尚无采样返回 0.0
 */
float ina226_get_raw_current(Ina226Rail rail);
