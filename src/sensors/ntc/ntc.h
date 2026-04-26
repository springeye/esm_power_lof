#pragma once
#include <cstdint>

/**
 * @brief 使用 NTC β 公式将 ADC 读数转换为温度。
 *
 * 电路：Vref -- R_pullup -- NTC -- GND
 * V_ntc = Vref * R_ntc / (R_pullup + R_ntc)
 * ADC = V_ntc / Vref * ADC_MAX
 *
 * β 公式：1/T = 1/T0 + (1/B) * ln(R/R25)
 *
 * 边界条件（来自 app_config.h）：
 *   ADC=0   → 短路 → 返回 NTC_TEMP_SHORT_C（150.0f）
 *   ADC=MAX → 开路 → 返回 NTC_TEMP_OPEN_C（-40.0f）
 *
 * @param adc_raw  12 位 ADC 采样值（0–4095）
 * @return 温度值，单位 °C
 */
float ntc_adc_to_temp(uint16_t adc_raw);
