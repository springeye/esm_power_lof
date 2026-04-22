/**
 * @file app_state.cpp
 * @brief 全局应用状态 atomic 变量定义
 */

#include "app_state.h"

namespace app_state {

std::atomic<int32_t>  temp_cdeg    {2500};  // 25.00°C 初始值
std::atomic<uint32_t> fan_rpm      {0};
std::atomic<uint16_t> fan_duty     {0};
std::atomic<int32_t>  ch1_ma       {0};
std::atomic<int32_t>  ch2_ma       {0};
std::atomic<int32_t>  ch3_ma       {0};
std::atomic<uint8_t>  psu_state_id {0};     // PSU_OFF = 0
std::atomic<bool>      fault_active {false};

} // namespace app_state
