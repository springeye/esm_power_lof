#include "config_manager.h"

#include "app_config.h"

#include <cstring>
#include <mutex>

#if !defined(BUILD_NATIVE) && defined(ARDUINO_ARCH_ESP32) && defined(__has_include)
#if __has_include(<Preferences.h>)
#include <Preferences.h>
#define CONFIG_MANAGER_HAS_PREFERENCES 1
#endif
#endif

#ifndef CONFIG_MANAGER_HAS_PREFERENCES
#define CONFIG_MANAGER_HAS_PREFERENCES 0
#endif

namespace {

constexpr char kNamespace[] = "esm_power_lof";
constexpr char kKeyFanLow[] = "cfg_fan_low";
constexpr char kKeyFanMid[] = "cfg_fan_mid";
constexpr char kKeyFanHigh[] = "cfg_fan_high";
constexpr char kKeyFanForce[] = "cfg_fan_force";
constexpr char kKeyFanPwmMin[] = "cfg_fan_pwm_min";
constexpr char kKeyFanPwmMid[] = "cfg_fan_pwm_mid";
constexpr char kKeyFanHyst[] = "cfg_fan_hyst";
constexpr char kKeyTempWarn[] = "cfg_temp_warn";
constexpr char kKeyTempShutdown[] = "cfg_temp_shutdown";
constexpr char kKeyStallRpm[] = "cfg_stall_rpm";
constexpr char kKeyStallDuty[] = "cfg_stall_duty";
constexpr char kKeyStallTimeout[] = "cfg_stall_tm";
constexpr char kKeyBrightness[] = "cfg_brightness";
constexpr char kKeyThemeMode[] = "cfg_theme_mode";
constexpr char kKeyChartYaxis[] = "cfg_chart_yaxis";
constexpr char kKeyDefaultView[] = "cfg_default_view";
constexpr char kKeyScreenRotation[] = "cfg_scr_rot";
constexpr char kKeyDesignPower[] = "cfg_design_power";
constexpr char kKeyNtcOffset[]   = "cfg_ntc_offset";
constexpr char kKeyInaGain0[]    = "cfg_ina_g0";
constexpr char kKeyInaGain1[]    = "cfg_ina_g1";
constexpr char kKeyInaGain2[]    = "cfg_ina_g2";
constexpr char kKeyInaOff0[]     = "cfg_ina_o0";
constexpr char kKeyInaOff1[]     = "cfg_ina_o1";
constexpr char kKeyInaOff2[]     = "cfg_ina_o2";
constexpr char kKeyWifiSsid[]    = "cfg_wifi_ssid";
constexpr char kKeyWifiPass[]    = "cfg_wifi_pass";
constexpr char kKeyWebMgmt[]     = "cfg_web_mgmt";

constexpr uint16_t kFanStallRpmMin = 0u;
constexpr uint16_t kFanStallRpmMax = 5000u;
constexpr uint16_t kFanStallDutyMin = 0u;
constexpr uint16_t kFanStallDutyMax = FAN_PWM_MAX;
constexpr uint32_t kFanStallTimeoutMinMs = 100u;
constexpr uint32_t kFanStallTimeoutMaxMs = 60000u;

std::mutex s_config_mutex;
config_manager::AppConfig s_config {};
bool s_initialized = false;

float clamp_float(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

uint8_t clamp_u8(uint8_t value, uint8_t min_value, uint8_t max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

uint16_t clamp_u16(uint16_t value, uint16_t min_value, uint16_t max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

uint32_t clamp_u32(uint32_t value, uint32_t min_value, uint32_t max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

bool is_valid_design_power(uint16_t value) {
    return value == 350u || value == 450u || value == 550u || value == 750u;
}

void set_defaults_locked() {
    s_config.fan.temp_low = 35.0f;
    s_config.fan.temp_mid = 45.0f;
    s_config.fan.temp_high = 55.0f;
    s_config.fan.temp_force = 60.0f;
    s_config.fan.pwm_min_percent = 20u;
    s_config.fan.pwm_mid_percent = 60u;
    s_config.fan.hysteresis = 2.0f;

    s_config.temp_protection.warning_threshold = 65.0f;
    s_config.temp_protection.shutdown_threshold = 75.0f;
    s_config.temp_protection.stall_rpm_thresh = 100u;
    s_config.temp_protection.stall_duty_thresh = FAN_STALL_DUTY_THRESH;
    s_config.temp_protection.stall_timeout_ms = FAN_STALL_TIMEOUT_MS;

    s_config.display.brightness_percent = 80u;
    s_config.display.theme_mode = 1u;
    s_config.display.chart_yaxis_mode = 0u;
    //TODO "启动视图"（0=默认,1=CH1图,2=CH2图,3=CH3图）
    s_config.display.default_view = 0u;
    s_config.display.screen_rotation = 0u;
    s_config.power.design_power_w = 750u;
    s_config.sensor.ntc_temp_offset = 0.0f;
    s_config.sensor.ina_gain_ch[0] = INA226_CAL_GAIN_DEFAULT;
    s_config.sensor.ina_gain_ch[1] = INA226_CAL_GAIN_DEFAULT;
    s_config.sensor.ina_gain_ch[2] = INA226_CAL_GAIN_DEFAULT;
    s_config.sensor.ina_offset_ch[0] = INA226_CAL_OFFSET_DEFAULT;
    s_config.sensor.ina_offset_ch[1] = INA226_CAL_OFFSET_DEFAULT;
    s_config.sensor.ina_offset_ch[2] = INA226_CAL_OFFSET_DEFAULT;
}

void ensure_initialized_locked() {
    if (!s_initialized) {
        set_defaults_locked();
        s_initialized = true;
    }
}

#if CONFIG_MANAGER_HAS_PREFERENCES
void save_to_nvs_locked() {
    Preferences prefs;
    if (!prefs.begin(kNamespace, false)) {
        return;
    }

    prefs.putFloat(kKeyFanLow, s_config.fan.temp_low);
    prefs.putFloat(kKeyFanMid, s_config.fan.temp_mid);
    prefs.putFloat(kKeyFanHigh, s_config.fan.temp_high);
    prefs.putFloat(kKeyFanForce, s_config.fan.temp_force);
    prefs.putUChar(kKeyFanPwmMin, s_config.fan.pwm_min_percent);
    prefs.putUChar(kKeyFanPwmMid, s_config.fan.pwm_mid_percent);
    prefs.putFloat(kKeyFanHyst, s_config.fan.hysteresis);
    prefs.putFloat(kKeyTempWarn, s_config.temp_protection.warning_threshold);
    prefs.putFloat(kKeyTempShutdown, s_config.temp_protection.shutdown_threshold);
    prefs.putUShort(kKeyStallRpm, s_config.temp_protection.stall_rpm_thresh);
    prefs.putUShort(kKeyStallDuty, s_config.temp_protection.stall_duty_thresh);
    prefs.putUInt(kKeyStallTimeout, s_config.temp_protection.stall_timeout_ms);
    prefs.putUChar(kKeyBrightness, s_config.display.brightness_percent);
    prefs.putUChar(kKeyThemeMode, s_config.display.theme_mode);
    prefs.putUChar(kKeyChartYaxis, s_config.display.chart_yaxis_mode);
    prefs.putUChar(kKeyDefaultView, s_config.display.default_view);
    prefs.putUChar(kKeyScreenRotation, s_config.display.screen_rotation);
    prefs.putUShort(kKeyDesignPower, s_config.power.design_power_w);
    prefs.putFloat(kKeyNtcOffset, s_config.sensor.ntc_temp_offset);
    prefs.putFloat(kKeyInaGain0, s_config.sensor.ina_gain_ch[0]);
    prefs.putFloat(kKeyInaGain1, s_config.sensor.ina_gain_ch[1]);
    prefs.putFloat(kKeyInaGain2, s_config.sensor.ina_gain_ch[2]);
    prefs.putFloat(kKeyInaOff0, s_config.sensor.ina_offset_ch[0]);
    prefs.putFloat(kKeyInaOff1, s_config.sensor.ina_offset_ch[1]);
    prefs.putFloat(kKeyInaOff2, s_config.sensor.ina_offset_ch[2]);
    prefs.putString(kKeyWifiSsid, s_config.wifi.ssid);
    prefs.putString(kKeyWifiPass, s_config.wifi.password);
    prefs.putBool(kKeyWebMgmt, s_config.wifi.web_mgmt_enabled);
    prefs.end();
}

void load_from_nvs_locked() {
    Preferences prefs;
    if (!prefs.begin(kNamespace, true)) {
        return;
    }

    s_config.fan.temp_low = clamp_float(
        prefs.getFloat(kKeyFanLow, s_config.fan.temp_low), 20.0f, 50.0f);
    s_config.fan.temp_mid = clamp_float(
        prefs.getFloat(kKeyFanMid, s_config.fan.temp_mid), 30.0f, 60.0f);
    s_config.fan.temp_high = clamp_float(
        prefs.getFloat(kKeyFanHigh, s_config.fan.temp_high), 40.0f, 70.0f);
    s_config.fan.temp_force = clamp_float(
        prefs.getFloat(kKeyFanForce, s_config.fan.temp_force), 50.0f, 80.0f);
    s_config.fan.pwm_min_percent = clamp_u8(
        prefs.getUChar(kKeyFanPwmMin, s_config.fan.pwm_min_percent), 10u, 50u);
    s_config.fan.pwm_mid_percent = clamp_u8(
        prefs.getUChar(kKeyFanPwmMid, s_config.fan.pwm_mid_percent), 40u, 90u);
    s_config.fan.hysteresis = clamp_float(
        prefs.getFloat(kKeyFanHyst, s_config.fan.hysteresis), 0.5f, 5.0f);
    s_config.temp_protection.warning_threshold = clamp_float(
        prefs.getFloat(kKeyTempWarn, s_config.temp_protection.warning_threshold), 50.0f, 80.0f);
    s_config.temp_protection.shutdown_threshold = clamp_float(
        prefs.getFloat(kKeyTempShutdown, s_config.temp_protection.shutdown_threshold), 60.0f, 90.0f);
    s_config.temp_protection.stall_rpm_thresh = clamp_u16(
        prefs.getUShort(kKeyStallRpm, s_config.temp_protection.stall_rpm_thresh),
        kFanStallRpmMin,
        kFanStallRpmMax);
    s_config.temp_protection.stall_duty_thresh = clamp_u16(
        prefs.getUShort(kKeyStallDuty, s_config.temp_protection.stall_duty_thresh),
        kFanStallDutyMin,
        kFanStallDutyMax);
    s_config.temp_protection.stall_timeout_ms = clamp_u32(
        prefs.getUInt(kKeyStallTimeout, s_config.temp_protection.stall_timeout_ms),
        kFanStallTimeoutMinMs,
        kFanStallTimeoutMaxMs);
    s_config.display.brightness_percent = clamp_u8(
        prefs.getUChar(kKeyBrightness, s_config.display.brightness_percent), 10u, 100u);
    s_config.display.theme_mode = clamp_u8(
        prefs.getUChar(kKeyThemeMode, s_config.display.theme_mode), 0u, 1u);
    s_config.display.chart_yaxis_mode = clamp_u8(
        prefs.getUChar(kKeyChartYaxis, s_config.display.chart_yaxis_mode), 0u, 1u);
    s_config.display.default_view = clamp_u8(
        prefs.getUChar(kKeyDefaultView, s_config.display.default_view), 0u, 3u);
    s_config.display.screen_rotation = clamp_u8(
        prefs.getUChar(kKeyScreenRotation, s_config.display.screen_rotation), 0u, 3u);

    const uint16_t design_power = prefs.getUShort(kKeyDesignPower, s_config.power.design_power_w);
    if (is_valid_design_power(design_power)) {
        s_config.power.design_power_w = design_power;
    }

    s_config.sensor.ntc_temp_offset = clamp_float(
        prefs.getFloat(kKeyNtcOffset, s_config.sensor.ntc_temp_offset), -10.0f, 10.0f);
    s_config.sensor.ina_gain_ch[0] = clamp_float(
        prefs.getFloat(kKeyInaGain0, s_config.sensor.ina_gain_ch[0]),
        INA226_CAL_GAIN_MIN, INA226_CAL_GAIN_MAX);
    s_config.sensor.ina_gain_ch[1] = clamp_float(
        prefs.getFloat(kKeyInaGain1, s_config.sensor.ina_gain_ch[1]),
        INA226_CAL_GAIN_MIN, INA226_CAL_GAIN_MAX);
    s_config.sensor.ina_gain_ch[2] = clamp_float(
        prefs.getFloat(kKeyInaGain2, s_config.sensor.ina_gain_ch[2]),
        INA226_CAL_GAIN_MIN, INA226_CAL_GAIN_MAX);
    s_config.sensor.ina_offset_ch[0] = clamp_float(
        prefs.getFloat(kKeyInaOff0, s_config.sensor.ina_offset_ch[0]),
        -INA226_CAL_OFFSET_MAX, INA226_CAL_OFFSET_MAX);
    s_config.sensor.ina_offset_ch[1] = clamp_float(
        prefs.getFloat(kKeyInaOff1, s_config.sensor.ina_offset_ch[1]),
        -INA226_CAL_OFFSET_MAX, INA226_CAL_OFFSET_MAX);
    s_config.sensor.ina_offset_ch[2] = clamp_float(
        prefs.getFloat(kKeyInaOff2, s_config.sensor.ina_offset_ch[2]),
        -INA226_CAL_OFFSET_MAX, INA226_CAL_OFFSET_MAX);
    prefs.getString(kKeyWifiSsid, s_config.wifi.ssid, sizeof(s_config.wifi.ssid));
    prefs.getString(kKeyWifiPass, s_config.wifi.password, sizeof(s_config.wifi.password));
    s_config.wifi.web_mgmt_enabled = prefs.getBool(kKeyWebMgmt, false);
    prefs.end();
}

void clear_nvs_locked() {
    Preferences prefs;
    if (!prefs.begin(kNamespace, false)) {
        return;
    }

    prefs.clear();
    prefs.end();
}
#else
void save_to_nvs_locked() {}
void load_from_nvs_locked() {}
void clear_nvs_locked() {}
#endif

} // namespace

namespace config_manager {

void init() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    set_defaults_locked();
    load_from_nvs_locked();
    s_initialized = true;
}

void reset_to_defaults() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    set_defaults_locked();
    clear_nvs_locked();
}

void load_from_nvs() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    load_from_nvs_locked();
}

void save_to_nvs() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    save_to_nvs_locked();
}

float get_fan_temp_low() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.fan.temp_low;
}

void set_fan_temp_low(float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.fan.temp_low = clamp_float(v, 20.0f, 50.0f);
    save_to_nvs_locked();
}

float get_fan_temp_mid() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.fan.temp_mid;
}

void set_fan_temp_mid(float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.fan.temp_mid = clamp_float(v, 30.0f, 60.0f);
    save_to_nvs_locked();
}

float get_fan_temp_high() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.fan.temp_high;
}

void set_fan_temp_high(float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.fan.temp_high = clamp_float(v, 40.0f, 70.0f);
    save_to_nvs_locked();
}

float get_fan_temp_force() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.fan.temp_force;
}

void set_fan_temp_force(float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.fan.temp_force = clamp_float(v, 50.0f, 80.0f);
    save_to_nvs_locked();
}

uint8_t get_fan_pwm_min_percent() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.fan.pwm_min_percent;
}

void set_fan_pwm_min_percent(uint8_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.fan.pwm_min_percent = clamp_u8(v, 10u, 50u);
    save_to_nvs_locked();
}

uint8_t get_fan_pwm_mid_percent() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.fan.pwm_mid_percent;
}

void set_fan_pwm_mid_percent(uint8_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.fan.pwm_mid_percent = clamp_u8(v, 40u, 90u);
    save_to_nvs_locked();
}

float get_fan_hysteresis() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.fan.hysteresis;
}

void set_fan_hysteresis(float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.fan.hysteresis = clamp_float(v, 0.5f, 5.0f);
    save_to_nvs_locked();
}

float get_temp_warning_threshold() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.temp_protection.warning_threshold;
}

void set_temp_warning_threshold(float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.temp_protection.warning_threshold = clamp_float(v, 50.0f, 80.0f);
    save_to_nvs_locked();
}

float get_temp_shutdown_threshold() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.temp_protection.shutdown_threshold;
}

void set_temp_shutdown_threshold(float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.temp_protection.shutdown_threshold = clamp_float(v, 60.0f, 90.0f);
    save_to_nvs_locked();
}

uint16_t get_fan_stall_rpm_thresh() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.temp_protection.stall_rpm_thresh;
}

void set_fan_stall_rpm_thresh(uint16_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.temp_protection.stall_rpm_thresh = clamp_u16(v, kFanStallRpmMin, kFanStallRpmMax);
    save_to_nvs_locked();
}

uint16_t get_fan_stall_duty_thresh() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.temp_protection.stall_duty_thresh;
}

void set_fan_stall_duty_thresh(uint16_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.temp_protection.stall_duty_thresh = clamp_u16(v, kFanStallDutyMin, kFanStallDutyMax);
    save_to_nvs_locked();
}

uint32_t get_fan_stall_timeout_ms() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.temp_protection.stall_timeout_ms;
}

void set_fan_stall_timeout_ms(uint32_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.temp_protection.stall_timeout_ms = clamp_u32(v, kFanStallTimeoutMinMs, kFanStallTimeoutMaxMs);
    save_to_nvs_locked();
}

uint8_t get_brightness_percent() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.display.brightness_percent;
}

void set_brightness_percent(uint8_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.display.brightness_percent = clamp_u8(v, 10u, 100u);
    save_to_nvs_locked();
}

uint8_t get_theme_mode() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.display.theme_mode;
}

void set_theme_mode(uint8_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.display.theme_mode = clamp_u8(v, 0u, 1u);
    save_to_nvs_locked();
}

uint8_t get_chart_yaxis_mode() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.display.chart_yaxis_mode;
}

void set_chart_yaxis_mode(uint8_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.display.chart_yaxis_mode = clamp_u8(v, 0u, 1u);
    save_to_nvs_locked();
}

uint8_t get_default_view() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.display.default_view;
}

void set_default_view(uint8_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.display.default_view = clamp_u8(v, 0u, 3u);
    save_to_nvs_locked();
}

uint8_t get_screen_rotation() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.display.screen_rotation;
}

void set_screen_rotation(uint8_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.display.screen_rotation = clamp_u8(v, 0u, 3u);
    save_to_nvs_locked();
}

uint16_t get_design_power_w() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.power.design_power_w;
}

void set_design_power_w(uint16_t v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (is_valid_design_power(v)) {
        s_config.power.design_power_w = v;
        save_to_nvs_locked();
    }
}

float get_ntc_temp_offset() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.sensor.ntc_temp_offset;
}

void set_ntc_temp_offset(float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.sensor.ntc_temp_offset = clamp_float(v, -10.0f, 10.0f);
    save_to_nvs_locked();
}

float get_ina_gain(uint8_t ch) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (ch >= 3u) {
        return INA226_CAL_GAIN_DEFAULT;
    }
    return s_config.sensor.ina_gain_ch[ch];
}

void set_ina_gain(uint8_t ch, float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (ch >= 3u) {
        return;
    }
    s_config.sensor.ina_gain_ch[ch] =
        clamp_float(v, INA226_CAL_GAIN_MIN, INA226_CAL_GAIN_MAX);
    save_to_nvs_locked();
}

float get_ina_offset(uint8_t ch) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (ch >= 3u) {
        return INA226_CAL_OFFSET_DEFAULT;
    }
    return s_config.sensor.ina_offset_ch[ch];
}

void set_ina_offset(uint8_t ch, float v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (ch >= 3u) {
        return;
    }
    s_config.sensor.ina_offset_ch[ch] =
        clamp_float(v, -INA226_CAL_OFFSET_MAX, INA226_CAL_OFFSET_MAX);
    save_to_nvs_locked();
}

bool get_web_mgmt_enabled() {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    return s_config.wifi.web_mgmt_enabled;
}

void set_web_mgmt_enabled(bool v) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    s_config.wifi.web_mgmt_enabled = v;
    save_to_nvs_locked();
}

void get_wifi_ssid(char* buf, size_t n) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (buf && n > 0) {
        strncpy(buf, s_config.wifi.ssid, n - 1);
        buf[n - 1] = '\0';
    }
}

void set_wifi_ssid(const char* s) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (s) {
        strncpy(s_config.wifi.ssid, s, sizeof(s_config.wifi.ssid) - 1);
        s_config.wifi.ssid[sizeof(s_config.wifi.ssid) - 1] = '\0';
        save_to_nvs_locked();
    }
}

void get_wifi_password(char* buf, size_t n) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (buf && n > 0) {
        strncpy(buf, s_config.wifi.password, n - 1);
        buf[n - 1] = '\0';
    }
}

void set_wifi_password(const char* p) {
    std::lock_guard<std::mutex> lock(s_config_mutex);
    ensure_initialized_locked();
    if (p) {
        strncpy(s_config.wifi.password, p, sizeof(s_config.wifi.password) - 1);
        s_config.wifi.password[sizeof(s_config.wifi.password) - 1] = '\0';
        save_to_nvs_locked();
    }
}

} // namespace config_manager
