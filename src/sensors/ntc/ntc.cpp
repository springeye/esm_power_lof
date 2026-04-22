#include "sensors/ntc/ntc.h"
#include "app_config.h"
#include <cmath>

float ntc_adc_to_temp(uint16_t adc_raw) {
    // Boundary: short circuit (ADC=0 → V_ntc=0 → R_ntc=0)
    if (adc_raw == 0) {
        return NTC_TEMP_SHORT_C;
    }
    // Boundary: open circuit (ADC=MAX → V_ntc=Vref → R_ntc=∞)
    if (adc_raw >= NTC_ADC_MAX) {
        return NTC_TEMP_OPEN_C;
    }

    // V_ntc = Vref * adc_raw / ADC_MAX
    // R_ntc = R_pullup * V_ntc / (Vref - V_ntc)
    //       = R_pullup * adc_raw / (ADC_MAX - adc_raw)
    float r_ntc = NTC_PULLUP_OHM * static_cast<float>(adc_raw)
                  / static_cast<float>(NTC_ADC_MAX - adc_raw);

    // Beta formula: 1/T = 1/T0 + (1/B) * ln(R/R25)
    float inv_t = (1.0f / NTC_T0_K)
                  + (1.0f / NTC_B_VALUE) * logf(r_ntc / NTC_R25_OHM);

    float temp_c = (1.0f / inv_t) - 273.15f;

    // Clamp to fault thresholds
    if (temp_c >= NTC_TEMP_SHORT_C) return NTC_TEMP_SHORT_C;
    if (temp_c <= NTC_TEMP_OPEN_C)  return NTC_TEMP_OPEN_C;

    return temp_c;
}
