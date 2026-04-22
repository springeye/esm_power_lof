#pragma once
/**
 * @file app_state.h
 * @brief 全局应用状态（atomic）+ 事件枚举
 *
 * 职责：
 *  - 定义跨任务共享的传感器/控制数据（atomic 保证无锁读写）
 *  - 定义 PsuEvent 枚举（合并自 events.h）
 *  - 提供 getter/setter 接口（内联，零开销）
 *
 * 约束：
 *  - 仅在 esp32dev env 编译（使用 std::atomic，FreeRTOS 环境）
 *  - 不包含任何 Arduino/ESP32 头文件（仅 C++ 标准库）
 *  - 所有字段使用 std::atomic 保证多核安全
 */

#include <atomic>
#include <cstdint>

// ── PsuEvent 枚举（合并自 events.h）────────────────────────────────────────
enum class PsuEvent : uint8_t {
    BOOT        = 0,  // 系统启动
    KEY_SHORT   = 1,  // 短按电源键
    KEY_LONG    = 2,  // 长按电源键（≥2s）
    PWOK_HIGH   = 3,  // PWOK 信号变高（电源就绪）
    PWOK_LOW    = 4,  // PWOK 信号变低（电源失稳）
    TIMEOUT_1S  = 5,  // 启动超时（1s）
    FAN_FAULT   = 6,  // 风扇故障
    OVER_TEMP   = 7,  // 过温
    OVER_CURR   = 8,  // 过流
};

namespace app_state {

// ── 传感器数据（原子，单位见注释）──────────────────────────────────────────
extern std::atomic<int32_t>  temp_cdeg;    // 温度 × 100（°C × 100，避免浮点）
extern std::atomic<uint32_t> fan_rpm;      // 风扇转速（RPM）
extern std::atomic<uint16_t> fan_duty;     // 风扇 PWM duty（0-1023）
extern std::atomic<int32_t>  ch1_ma;       // CH1 电流（mA）
extern std::atomic<int32_t>  ch2_ma;       // CH2 电流（mA）
extern std::atomic<int32_t>  ch3_ma;       // CH3 电流（mA）
extern std::atomic<uint8_t>  psu_state_id; // PsuState 枚举值（uint8_t）
extern std::atomic<bool>      fault_active; // 故障标志

// ── 便捷 getter（内联）──────────────────────────────────────────────────────
inline float get_temp_c()    { return temp_cdeg.load() / 100.0f; }
inline uint32_t get_rpm()    { return fan_rpm.load(); }
inline uint16_t get_duty()   { return fan_duty.load(); }
inline float get_ch1_a()     { return ch1_ma.load() / 1000.0f; }
inline float get_ch2_a()     { return ch2_ma.load() / 1000.0f; }
inline float get_ch3_a()     { return ch3_ma.load() / 1000.0f; }
inline bool  is_fault()      { return fault_active.load(); }

// ── 便捷 setter（内联）──────────────────────────────────────────────────────
inline void set_temp_c(float t)      { temp_cdeg.store(static_cast<int32_t>(t * 100)); }
inline void set_rpm(uint32_t r)      { fan_rpm.store(r); }
inline void set_duty(uint16_t d)     { fan_duty.store(d); }
inline void set_ch1_ma(int32_t ma)  { ch1_ma.store(ma); }
inline void set_ch2_ma(int32_t ma)  { ch2_ma.store(ma); }
inline void set_ch3_ma(int32_t ma)  { ch3_ma.store(ma); }
inline void set_fault(bool f)        { fault_active.store(f); }
inline void set_psu_state(uint8_t s) { psu_state_id.store(s); }

} // namespace app_state
