#pragma once
#include <cstdint>

// ── NTC 温度传感器（specs/temperature-sensing, design.md D4） ──
static constexpr float    NTC_R25_OHM       = 10000.0f;  // NTC 在 25°C 时的标称阻值，用于 β 公式计算温度
static constexpr float    NTC_B_VALUE       = 3950.0f;   // NTC 材料的 β 系数，用于温度换算
static constexpr float    NTC_PULLUP_OHM    = 10000.0f;  // 上拉电阻阻值，用于分压采样电路
static constexpr float    NTC_T0_K          = 298.15f;   // 25°C 对应的开尔文基准温度，用于公式计算
static constexpr float    NTC_VREF          = 3.3f;      // ADC 参考电压，用于把采样值换算成电压
static constexpr uint16_t NTC_ADC_MAX       = 4095;      // 12 位 ADC 的最大采样值，用于归一化
static constexpr uint8_t  NTC_SAMPLES       = 16;        // 中值滤波采样次数，用于抑制温度抖动
static constexpr float    NTC_TEMP_SHORT_C  = 150.0f;   // 传感器短路时返回的异常温度阈值，用于故障判断
static constexpr float    NTC_TEMP_OPEN_C   = -40.0f;   // 传感器开路时返回的异常温度阈值，用于故障判断

// ── 风扇控制（specs/fan-control, design.md D4-D5） ──
static constexpr uint32_t FAN_PWM_FREQ_HZ      = 25000;  // 风扇 PWM 频率，4 线风扇标准频率，可避免可听噪声
static constexpr uint8_t  FAN_PWM_RES_BITS     = 10;     // 风扇 PWM 分辨率，用于限定占空比精度
static constexpr uint8_t  FAN_LEDC_CH          = 0;      // 风扇 PWM 使用的 LEDC 通道编号
static constexpr uint16_t FAN_PWM_MIN          = 205;    // 风扇最低占空比（约 20%），用于防止低速失速
static constexpr uint16_t FAN_PWM_MAX          = 1023;   // 风扇最高占空比（100%），用于满速输出
static constexpr float    FAN_TEMP_LOW         = 30.0f;  // 低温起调点（°C），用于开始提升风扇转速
static constexpr float    FAN_TEMP_MID         = 50.0f;  // 中温拐点（°C），用于切换到中段斜率
static constexpr float    FAN_TEMP_HIGH        = 70.0f;  // 高温满速点（°C），用于输出全速
static constexpr float    FAN_TEMP_FORCE       = 75.0f;  // 强制满速温度（°C），用于紧急散热保护
static constexpr float    FAN_HYSTERESIS       = 2.0f;   // 温控滞回带宽（°C），用于减少频繁抖动
static constexpr uint16_t FAN_STALL_DUTY_THRESH = 307;  // 堵转判定占空比阈值（约 30%），用于检测低速异常
static constexpr uint32_t FAN_STALL_TIMEOUT_MS  = 3000; // 堵转持续判定时间（3s），用于确认风扇失速
static constexpr uint32_t FAN_FAULT_SHUTDOWN_MS = 5000; // 风扇故障后的关机延时（5s），用于保护电源

// ── INA226 电源轨监测（specs/power-rail-monitoring, design.md D6） ──
// 三路 INA226 监测同一直流输出电压轨的三个并联输出接口，电压相同，分别计量各接口电流
static constexpr uint32_t I2C_FREQ_HZ          = 400000; // I2C 总线速率（400kHz 快速模式），用于 INA226 通信
// 7 位地址：{0x40, 0x41, 0x44}  8 位写地址：{0x80, 0x82, 0x88}  8 位读地址：{0x81, 0x83, 0x89}
static constexpr uint8_t  INA226_ADDR_CH[3]  = {0x40, 0x41, 0x44};
static constexpr float    INA226_SHUNT_OHMS     = 0.002f; // 分流电阻阻值（2mΩ），用于电流计算
static constexpr float    INA226_MAX_CURRENT_A  = 40.0f;  // 单路最大电流（40A），用于量程配置
static constexpr uint32_t INA226_POLL_PERIOD_MS = 750;    // 三路总轮询周期（3 × 250ms），用于采样调度

// ── 按键（specs/user-input, design.md D8） ──
static constexpr uint32_t KEYS_DEBOUNCE_MS  = 5;    // 按键去抖采样间隔（5ms），用于稳定按键状态
static constexpr uint32_t KEYS_LONGPRESS_MS = 800;  // 长按判定时间（≥800ms），用于区分短按与长按
static constexpr uint32_t KEYS_POLL_MS      = 5;    // 按键任务轮询周期（5ms），用于定时扫描输入
static constexpr uint32_t PSU_LONGPRESS_MS  = 2000; // 电源键长按关机阈值（≥2s），用于触发关机

// ── 电源时序（specs/power-switch-control, design.md D7） ──
static constexpr uint32_t PSU_START_TIMEOUT_MS = 1000; // 电源启动超时时间（1s），用于等待供电就绪
static constexpr uint32_t PSU_PWOK_LOST_MS     = 100;  // PWOK 失稳判定时间（100ms），用于过滤短暂毛刺

// ── FreeRTOS 任务栈（design.md D9） ──
static constexpr uint32_t TASK_STACK_LVGL   = 8192;  // LVGL 渲染任务栈大小（字节），LVGL 内存消耗较大
static constexpr uint32_t TASK_STACK_SENSOR = 4096;  // 传感器采样任务栈大小（字节）
static constexpr uint32_t TASK_STACK_CTRL   = 4096;  // 温控/风扇控制任务栈大小（字节）
static constexpr uint32_t TASK_STACK_INPUT  = 3072;  // 按键输入任务栈大小（字节）
static constexpr uint32_t TASK_STACK_POWER  = 3072;  // 电源状态机任务栈大小（字节）

// ── 看门狗（design.md D9） ──
static constexpr uint32_t TASK_WDT_TIMEOUT_S = 5;    // 任务看门狗超时时间（秒），超时未喂狗则复位

// ── 显示调试开关 ──
// true  : 仅运行 TFT 底层测试图案（彩条 / 文本 / 闪烁），不初始化 LVGL、UI、任务
//         用于排查屏幕接线、SPI 时序、引脚配置等底层问题
// false : 正常运行业务 UI（LVGL + 风扇控制 + 电源管理 + 传感器）
static constexpr bool USE_DISPLAY_DEMO = true;
