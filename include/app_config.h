#pragma once
#include <cstdint>

// ── NTC Temperature Sensor (specs/temperature-sensing, design.md D4) ──
static constexpr float    NTC_R25_OHM       = 10000.0f;  // R25 = 10kΩ
static constexpr float    NTC_B_VALUE       = 3950.0f;   // β coefficient
static constexpr float    NTC_PULLUP_OHM    = 10000.0f;  // pull-up resistor
static constexpr float    NTC_T0_K          = 298.15f;   // 25°C in Kelvin
static constexpr float    NTC_VREF          = 3.3f;      // ADC reference voltage
static constexpr uint16_t NTC_ADC_MAX       = 4095;      // 12-bit ADC
static constexpr uint8_t  NTC_SAMPLES       = 16;        // median filter samples
static constexpr float    NTC_TEMP_SHORT_C  = 150.0f;   // short-circuit threshold
static constexpr float    NTC_TEMP_OPEN_C   = -40.0f;   // open-circuit threshold

// ── Fan Control (specs/fan-control, design.md D4-D5) ──
static constexpr uint32_t FAN_PWM_FREQ_HZ      = 25000;  // 25kHz PWM
static constexpr uint8_t  FAN_PWM_RES_BITS     = 10;     // 10-bit resolution
static constexpr uint8_t  FAN_LEDC_CH          = 0;      // LEDC channel 0
static constexpr uint16_t FAN_PWM_MIN          = 205;    // 20% of 1023
static constexpr uint16_t FAN_PWM_MAX          = 1023;   // 100%
static constexpr float    FAN_TEMP_LOW         = 30.0f;  // °C: start ramp
static constexpr float    FAN_TEMP_MID         = 50.0f;  // °C: mid ramp
static constexpr float    FAN_TEMP_HIGH        = 70.0f;  // °C: full speed
static constexpr float    FAN_TEMP_FORCE       = 75.0f;  // °C: force 100%
static constexpr float    FAN_HYSTERESIS       = 2.0f;   // °C hysteresis
static constexpr uint16_t FAN_STALL_DUTY_THRESH = 307;  // 30% of 1023
static constexpr uint32_t FAN_STALL_TIMEOUT_MS  = 3000; // 3s stall detection
static constexpr uint32_t FAN_FAULT_SHUTDOWN_MS = 5000; // 5s fan fault → PSU off

// ── INA226 Power Rail Monitoring (specs/power-rail-monitoring, design.md D6) ──
static constexpr uint32_t I2C_FREQ_HZ          = 400000; // 400kHz Fast Mode
// 7-bit: {0x40, 0x41, 0x44}  8-bit write: {0x80, 0x82, 0x88}  8-bit read: {0x81, 0x83, 0x89}
static constexpr uint8_t  INA226_ADDR_CH[3]  = {0x40, 0x41, 0x44};
static constexpr float    INA226_SHUNT_OHMS     = 0.002f; // 2mΩ shunt
static constexpr float    INA226_MAX_CURRENT_A  = 40.0f;  // 40A max
static constexpr uint32_t INA226_POLL_PERIOD_MS = 750;    // 3 rails × 250ms

// ── Keys (specs/user-input, design.md D8) ──
static constexpr uint32_t KEYS_DEBOUNCE_MS  = 5;    // 5ms sampling interval
static constexpr uint32_t KEYS_LONGPRESS_MS = 800;  // ≥800ms = long press
static constexpr uint32_t KEYS_POLL_MS      = 5;    // inputTask period
static constexpr uint32_t PSU_LONGPRESS_MS  = 2000; // K1 ≥2s = shutdown

// ── PSU Timing (specs/power-switch-control, design.md D7) ──
static constexpr uint32_t PSU_START_TIMEOUT_MS = 1000; // 1s startup timeout
static constexpr uint32_t PSU_PWOK_LOST_MS     = 100;  // 100ms PWOK instability

// ── FreeRTOS Task Stacks (design.md D9) ──
static constexpr uint32_t TASK_STACK_LVGL   = 8192;
static constexpr uint32_t TASK_STACK_SENSOR = 4096;
static constexpr uint32_t TASK_STACK_CTRL   = 4096;
static constexpr uint32_t TASK_STACK_INPUT  = 3072;
static constexpr uint32_t TASK_STACK_POWER  = 3072;

// ── Watchdog (design.md D9) ──
static constexpr uint32_t TASK_WDT_TIMEOUT_S = 5;
