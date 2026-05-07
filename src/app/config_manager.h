#pragma once

#include <cstdint>

namespace config_manager {

struct FanConfig {
    float temp_low;
    float temp_mid;
    float temp_high;
    float temp_force;
    uint8_t pwm_min_percent;
    uint8_t pwm_mid_percent;
    float hysteresis;
};

struct TempProtectionConfig {
    float warning_threshold;
    float shutdown_threshold;
    uint16_t stall_rpm_thresh;      // 堵转判定转速阈值（RPM）
    uint16_t stall_duty_thresh;     // 堵转判定占空比阈值
    uint32_t stall_timeout_ms;      // 堵转持续判定时间（ms）
};

struct DisplayConfig {
    uint8_t brightness_percent;
    uint8_t theme_mode;
    uint8_t chart_yaxis_mode;
    uint8_t default_view;
};

struct PowerConfig {
    uint16_t design_power_w;
};

struct SensorConfig {
    float ntc_temp_offset;
};

struct AppConfig {
    FanConfig fan;
    TempProtectionConfig temp_protection;
    DisplayConfig display;
    PowerConfig power;
    SensorConfig sensor;
};

void init();
void reset_to_defaults();
void load_from_nvs();
void save_to_nvs();

float get_fan_temp_low();
void set_fan_temp_low(float v);
float get_fan_temp_mid();
void set_fan_temp_mid(float v);
float get_fan_temp_high();
void set_fan_temp_high(float v);
float get_fan_temp_force();
void set_fan_temp_force(float v);
uint8_t get_fan_pwm_min_percent();
void set_fan_pwm_min_percent(uint8_t v);
uint8_t get_fan_pwm_mid_percent();
void set_fan_pwm_mid_percent(uint8_t v);
float get_fan_hysteresis();
void set_fan_hysteresis(float v);

float get_temp_warning_threshold();
void set_temp_warning_threshold(float v);
float get_temp_shutdown_threshold();
void set_temp_shutdown_threshold(float v);
uint16_t get_fan_stall_rpm_thresh();
void set_fan_stall_rpm_thresh(uint16_t v);
uint16_t get_fan_stall_duty_thresh();
void set_fan_stall_duty_thresh(uint16_t v);
uint32_t get_fan_stall_timeout_ms();
void set_fan_stall_timeout_ms(uint32_t v);

uint8_t get_brightness_percent();
void set_brightness_percent(uint8_t v);

uint8_t get_theme_mode();
void set_theme_mode(uint8_t v);

uint8_t get_chart_yaxis_mode();
void set_chart_yaxis_mode(uint8_t v);

uint8_t get_default_view();
void set_default_view(uint8_t v);

uint16_t get_design_power_w();
void set_design_power_w(uint16_t v);

float get_ntc_temp_offset();
void set_ntc_temp_offset(float v);

} // namespace config_manager
