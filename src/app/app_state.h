#pragma once
/**
 * @file app_state.h
 * @brief 全局应用状态（原子）+ 事件枚举
 *
 * 职责：
 *  - 定义跨任务共享的传感器/控制数据（使用原子保证无锁读写）
 *  - 定义 PsuEvent 枚举（合并自 events.h）
 *  - 提供取值/设值接口（内联，零开销）
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
    BOOT        = 0,  // 系统启动事件
    KEY_SHORT   = 1,  // 电源键短按事件
    KEY_LONG    = 2,  // 电源键长按事件（≥2s）
    PWOK_HIGH   = 3,  // PWOK 信号变高事件（电源就绪）
    PWOK_LOW    = 4,  // PWOK 信号变低事件（电源失稳）
    TIMEOUT_1S  = 5,  // 启动超时 1s 事件
    FAN_FAULT   = 6,  // 风扇故障事件
    OVER_TEMP   = 7,  // 过温事件
    OVER_CURR   = 8,  // 过流事件
};

namespace app_state {

// ── 传感器数据（原子，单位见注释）──────────────────────────────────────────
extern std::atomic<int32_t>  temp_cdeg;    // 温度 × 100（°C × 100，避免浮点）
extern std::atomic<uint32_t> fan_rpm;      // 风扇转速（RPM）
extern std::atomic<uint16_t> fan_duty;     // 风扇 PWM 占空比（0-1023）
extern std::atomic<int32_t>  ch1_ma;       // 输出接口 1 电流（mA）
extern std::atomic<int32_t>  ch2_ma;       // 输出接口 2 电流（mA）
extern std::atomic<int32_t>  ch3_ma;       // 输出接口 3 电流（mA）
extern std::atomic<uint16_t> ch1_mv;       // 输出接口 1 电压（mV）
extern std::atomic<uint16_t> ch2_mv;       // 输出接口 2 电压（mV）
extern std::atomic<uint16_t> ch3_mv;       // 输出接口 3 电压（mV）
extern std::atomic<uint8_t>  psu_state_id; // PsuState 枚举值（存储为 uint8_t）
extern std::atomic<bool>     fault_active; // 故障标志
extern std::atomic<int8_t>   ota_progress; // OTA 进度（-1=无OTA，0-100=百分比）
extern char                  wifi_ap_password[7]; // AP 密码（MAC 后6位hex + \0）
extern std::atomic<bool>     wifi_ap_password_ready; // 密码已就绪标志（写后置 true，读前检查）
extern std::atomic<uint8_t>  psu_event_request;     // 跨任务 PSU 事件请求（0=无，非0=PsuFsmEvent 值）

// ── 便捷取值函数（内联）──────────────────────────────────────────────────────
inline float get_temp_c()    { return temp_cdeg.load() / 100.0f; }
inline uint32_t get_rpm()    { return fan_rpm.load(); }
inline uint16_t get_duty()   { return fan_duty.load(); }
inline float get_ch1_a()     { return ch1_ma.load() / 1000.0f; }
inline float get_ch2_a()     { return ch2_ma.load() / 1000.0f; }
inline float get_ch3_a()     { return ch3_ma.load() / 1000.0f; }
inline uint16_t get_ch1_mv() { return ch1_mv.load(); }
inline uint16_t get_ch2_mv() { return ch2_mv.load(); }
inline uint16_t get_ch3_mv() { return ch3_mv.load(); }
inline bool    is_fault()       { return fault_active.load(); }
inline int8_t  get_ota_progress() { return ota_progress.load(); }

// ── 便捷设值函数（内联）──────────────────────────────────────────────────────
inline void set_temp_c(float t)       { temp_cdeg.store(static_cast<int32_t>(t * 100)); }
inline void set_rpm(uint32_t r)       { fan_rpm.store(r); }
inline void set_duty(uint16_t d)      { fan_duty.store(d); }
inline void set_ch1_ma(int32_t ma)    { ch1_ma.store(ma); }
inline void set_ch2_ma(int32_t ma)    { ch2_ma.store(ma); }
inline void set_ch3_ma(int32_t ma)    { ch3_ma.store(ma); }
inline void set_ch1_mv(uint16_t mv)   { ch1_mv.store(mv); }
inline void set_ch2_mv(uint16_t mv)   { ch2_mv.store(mv); }
inline void set_ch3_mv(uint16_t mv)   { ch3_mv.store(mv); }
inline void set_fault(bool f)         { fault_active.store(f); }
inline void set_psu_state(uint8_t s)  { psu_state_id.store(s); }
inline void set_ota_progress(int8_t p) { ota_progress.store(p); }

} // namespace app_state
