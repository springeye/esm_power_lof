#include "settings_ui.h"

#include "../app/app_state.h"
#include "../app/config_manager.h"
#include "../wifi/wifi_manager.h"
#include "theme_manager.h"
#include "screen_manager.h"

#include <atomic>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <cstdio>

#include "lvgl/lvgl.h"

extern "C" {
#include "../../ui/lof_power_system_gen.h"
#include "../../ui/screens/settings_gen.h"
}

namespace settings_ui {
namespace {

constexpr uint8_t KEY_K1_ID = 0;
constexpr uint8_t KEY_K2_ID = 1;
constexpr uint8_t KEY_K3_ID = 2;
constexpr uint32_t BLINK_PERIOD_MS = 350;
constexpr size_t MAX_PAGE_ITEMS = 8;

enum SettingsPage : uint8_t {
    PAGE_FAN = 0,
    PAGE_TEMP,
    PAGE_DISPLAY,
    PAGE_POWER,
    PAGE_SENSOR,
    PAGE_SYSTEM,
    PAGE_COUNT
};

enum class SettingsItemType : uint8_t {
    FLOAT,
    UINT8,
    UINT16,
    BOOL,
};

using FloatGetter = float (*)();
using FloatSetter = void (*)(float);
using UInt8Getter = uint8_t (*)();
using UInt8Setter = void (*)(uint8_t);
using UInt16Getter = uint16_t (*)();
using UInt16Setter = void (*)(uint16_t);
using BoolGetter = bool (*)();
using BoolSetter = void (*)(bool);

struct SettingsItem {
    const char* label;
    const char* unit;
    SettingsItemType type;
    float min_val;
    float max_val;
    float step;
    FloatGetter get_float;
    FloatSetter set_float;
    UInt8Getter get_u8;
    UInt8Setter set_u8;
    UInt16Getter get_u16;
    UInt16Setter set_u16;
    bool (*get_bool)();
    void (*set_bool)(bool);
    const uint16_t* preset_values;
    size_t preset_count;
};

struct SettingsPageDef {
    const char* title;
    const SettingsItem* items;
    size_t item_count;
};

struct RowRefs {
    lv_obj_t* row;
    lv_obj_t* value_label;
};

struct KeyDispatch {
    uint8_t key_id;
    KeyEvent event;
};

constexpr uint16_t POWER_PRESETS[] = {350u, 450u, 550u, 750u};

const SettingsItem FAN_ITEMS[] = {
    {"低温阈值", "°C", SettingsItemType::FLOAT, 20.0f, 50.0f, 1.0f,
     config_manager::get_fan_temp_low, config_manager::set_fan_temp_low,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"中温阈值", "°C", SettingsItemType::FLOAT, 30.0f, 60.0f, 1.0f,
     config_manager::get_fan_temp_mid, config_manager::set_fan_temp_mid,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"高温阈值", "°C", SettingsItemType::FLOAT, 40.0f, 70.0f, 1.0f,
     config_manager::get_fan_temp_high, config_manager::set_fan_temp_high,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"强制阈值", "°C", SettingsItemType::FLOAT, 50.0f, 80.0f, 1.0f,
     config_manager::get_fan_temp_force, config_manager::set_fan_temp_force,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"最低转速%", "%", SettingsItemType::UINT8, 0.0f, 100.0f, 5.0f,
     nullptr, nullptr,
     config_manager::get_fan_pwm_min_percent, config_manager::set_fan_pwm_min_percent,
     nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"中间转速%", "%", SettingsItemType::UINT8, 0.0f, 100.0f, 5.0f,
     nullptr, nullptr,
     config_manager::get_fan_pwm_mid_percent, config_manager::set_fan_pwm_mid_percent,
     nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"滞回温度", "°C", SettingsItemType::FLOAT, 0.5f, 5.0f, 0.5f,
     config_manager::get_fan_hysteresis, config_manager::set_fan_hysteresis,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0},
};

const SettingsItem TEMP_ITEMS[] = {
    {"警告阈值", "°C", SettingsItemType::FLOAT, 50.0f, 80.0f, 1.0f,
     config_manager::get_temp_warning_threshold, config_manager::set_temp_warning_threshold,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"关机阈值", "°C", SettingsItemType::FLOAT, 60.0f, 90.0f, 1.0f,
     config_manager::get_temp_shutdown_threshold, config_manager::set_temp_shutdown_threshold,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0},
};

const SettingsItem DISPLAY_ITEMS[] = {
    {"亮度%", "%", SettingsItemType::UINT8, 10.0f, 100.0f, 5.0f,
     nullptr, nullptr,
     config_manager::get_brightness_percent, config_manager::set_brightness_percent,
     nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"主题模式", "", SettingsItemType::UINT8, 0.0f, 1.0f, 1.0f,
     nullptr, nullptr,
     config_manager::get_theme_mode, config_manager::set_theme_mode,
     nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"Y轴模式", "", SettingsItemType::UINT8, 0.0f, 1.0f, 1.0f,
     nullptr, nullptr,
     config_manager::get_chart_yaxis_mode, config_manager::set_chart_yaxis_mode,
     nullptr, nullptr, nullptr, nullptr, nullptr, 0},
    {"启动视图", "", SettingsItemType::UINT8, 0.0f, 3.0f, 1.0f,
     nullptr, nullptr,
     config_manager::get_default_view, config_manager::set_default_view,
     nullptr, nullptr, nullptr, nullptr, nullptr, 0},
};

const SettingsItem POWER_ITEMS[] = {
    {"设计功率W", "W", SettingsItemType::UINT16, 350.0f, 750.0f, 100.0f,
     nullptr, nullptr, nullptr, nullptr,
     config_manager::get_design_power_w, config_manager::set_design_power_w,
     nullptr, nullptr,
     POWER_PRESETS, sizeof(POWER_PRESETS) / sizeof(POWER_PRESETS[0])},
};

const SettingsItem SENSOR_ITEMS[] = {
    {"温度偏移°C", "°C", SettingsItemType::FLOAT, -5.0f, 5.0f, 0.1f,
     config_manager::get_ntc_temp_offset, config_manager::set_ntc_temp_offset,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, 0},
};

const SettingsItem SYSTEM_ITEMS[] = {
    {"Web 管理", "", SettingsItemType::BOOL, 0.0f, 1.0f, 1.0f,
     nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
     config_manager::get_web_mgmt_enabled, config_manager::set_web_mgmt_enabled,
     nullptr, 0},
};

const SettingsPageDef PAGES[] = {
    {"风扇设置", FAN_ITEMS, sizeof(FAN_ITEMS) / sizeof(FAN_ITEMS[0])},
    {"温度保护", TEMP_ITEMS, sizeof(TEMP_ITEMS) / sizeof(TEMP_ITEMS[0])},
    {"显示设置", DISPLAY_ITEMS, sizeof(DISPLAY_ITEMS) / sizeof(DISPLAY_ITEMS[0])},
    {"功率配置", POWER_ITEMS, sizeof(POWER_ITEMS) / sizeof(POWER_ITEMS[0])},
    {"传感器校准", SENSOR_ITEMS, sizeof(SENSOR_ITEMS) / sizeof(SENSOR_ITEMS[0])},
    {"系统管理", SYSTEM_ITEMS, sizeof(SYSTEM_ITEMS) / sizeof(SYSTEM_ITEMS[0])},
};

lv_obj_t* g_screen = nullptr;
lv_obj_t* g_title_label = nullptr;
lv_obj_t* g_page_label = nullptr;
lv_obj_t* g_content_area = nullptr;
RowRefs g_rows[MAX_PAGE_ITEMS] = {};
size_t g_row_count = 0;
uint8_t g_current_page = PAGE_FAN;
uint8_t g_focus_index = 0;
bool g_editing = false;
float g_edit_value = 0.0f;
lv_timer_t* g_blink_timer = nullptr;
std::atomic_bool g_active {false};
bool g_initialized = false;
lv_obj_t* g_system_pwd_label = nullptr;

void rebuild_page();

void init_impl() {
    if (g_initialized) {
        return;
    }

    g_screen = settings_create();
    if (g_screen == nullptr) {
        return;
    }

    // ── 给关键子对象设置 name（按当前 settings.xml 结构）────
    if (g_screen == nullptr) {
        return;
    }

    // ── 给关键子对象设置 name（按当前 settings.xml 结构）────
    {
        lv_obj_t* header  = lv_obj_get_child(g_screen, 0);
        lv_obj_t* content = lv_obj_get_child(g_screen, 1);
        if (header) {
            lv_obj_set_name(header, "settings_header");
            lv_obj_t* title = lv_obj_get_child(header, 1);
            lv_obj_t* page  = lv_obj_get_child(header, 2);
            if (title) lv_obj_set_name(title, "settings_title");
            if (page)  lv_obj_set_name(page,  "settings_page");
        }
        if (content) {
            lv_obj_set_name(content, "settings_content");
        }
    }

    // ── 改用 name 查找，不再依赖 index ──
    lv_obj_t* header  = lv_obj_get_child_by_name(g_screen, "settings_header");
    g_content_area     = lv_obj_get_child_by_name(g_screen, "settings_content");

    if (header != nullptr) {
        g_title_label = lv_obj_get_child_by_name(header, "settings_title");
        g_page_label  = lv_obj_get_child_by_name(header, "settings_page");
    }

    g_initialized = (g_content_area != nullptr && g_title_label != nullptr && g_page_label != nullptr);
    if (!g_initialized) {
        g_screen = nullptr;
        g_title_label = nullptr;
        g_page_label = nullptr;
        g_content_area = nullptr;
    }
}

const SettingsPageDef& current_page() {
    return PAGES[g_current_page];
}

const SettingsItem& current_item() {
    return current_page().items[g_focus_index];
}

float clamp_value(float value, float min_value, float max_value) {
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

float read_item_value(const SettingsItem& item) {
    switch (item.type) {
        case SettingsItemType::FLOAT:
            return item.get_float ? item.get_float() : 0.0f;
        case SettingsItemType::UINT8:
            return item.get_u8 ? static_cast<float>(item.get_u8()) : 0.0f;
        case SettingsItemType::UINT16:
            return item.get_u16 ? static_cast<float>(item.get_u16()) : 0.0f;
        case SettingsItemType::BOOL:
            return item.get_bool ? (item.get_bool() ? 1.0f : 0.0f) : 0.0f;
        default:
            return 0.0f;
    }
}

void write_item_value(const SettingsItem& item, float value) {
    const float clamped = clamp_value(value, item.min_val, item.max_val);
    switch (item.type) {
        case SettingsItemType::FLOAT:
            if (item.set_float) {
                item.set_float(clamped);
            }
            break;
        case SettingsItemType::UINT8:
            if (item.set_u8) {
                item.set_u8(static_cast<uint8_t>(std::lround(clamped)));
            }
            break;
        case SettingsItemType::UINT16:
            if (item.set_u16) {
                item.set_u16(static_cast<uint16_t>(std::lround(clamped)));
            }
            break;
        case SettingsItemType::BOOL: {
            const bool new_val = (std::lround(clamped) != 0);
            if (item.set_bool) {
                item.set_bool(new_val);
            }
            // 联动 WiFi AP 开关
            if (item.get_bool == config_manager::get_web_mgmt_enabled) {
                if (new_val) {
                    wifi_mgr::start_ap();
                } else {
                    wifi_mgr::stop();
                }
            }
            break;
        }
        default:
            break;
    }
}

void format_item_value(const SettingsItem& item, float value, char* buffer, size_t size) {
    if (item.get_u8 == config_manager::get_theme_mode) {
        const int v = static_cast<int>(std::lround(value));
        std::snprintf(buffer, size, "%s", v == 0 ? "日间" : "夜间");
        return;
    }

    if (item.get_u8 == config_manager::get_chart_yaxis_mode) {
        const int v = static_cast<int>(std::lround(value));
        std::snprintf(buffer, size, "%s", v == 0 ? "自动" : "固定");
        return;
    }

    if (item.get_u8 == config_manager::get_default_view) {
        const int v = static_cast<int>(std::lround(value));
        const char* names[] = {"默认", "CH1图", "CH2图", "CH3图"};
        std::snprintf(buffer, size, "%s", (v >= 0 && v <= 3) ? names[v] : "默认");
        return;
    }

    switch (item.type) {
        case SettingsItemType::FLOAT:
            std::snprintf(buffer, size, "%.1f%s", value, item.unit ? item.unit : "");
            break;
        case SettingsItemType::UINT8:
        case SettingsItemType::UINT16:
            std::snprintf(buffer, size, "%d%s", static_cast<int>(std::lround(value)), item.unit ? item.unit : "");
            break;
        case SettingsItemType::BOOL:
            std::snprintf(buffer, size, "%s", (std::lround(value) != 0) ? "开" : "关");
            break;
        default:
            std::snprintf(buffer, size, "--");
            break;
    }
}

void refresh_row_visual(size_t row_index) {
    if (row_index >= g_row_count || g_rows[row_index].row == nullptr) {
        return;
    }

    lv_obj_t* row = g_rows[row_index].row;
    lv_obj_t* value_label = g_rows[row_index].value_label;
    const bool focused = row_index == g_focus_index;
    const bool editing = focused && g_editing;

    const auto& colors = theme_manager::theme_current_colors();
    lv_color_t bg = colors.row_default_bg;
    lv_color_t border = colors.row_default_border;
    lv_color_t value_color = colors.text_primary;

    if (editing) {
        bg = colors.row_edit_bg;
        border = colors.row_edit_border;
        value_color = lv_color_hex(0xffd166);
    } else if (focused) {
        bg = colors.row_focus_bg;
        border = colors.row_focus_border;
    }

    lv_obj_set_style_bg_color(row, bg, 0);
    lv_obj_set_style_bg_opa(row, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(row, border, 0);
    lv_obj_set_style_border_width(row, 1, 0);
    lv_obj_set_style_text_color(value_label, value_color, 0);
}

void stop_blink() {
    if (g_blink_timer != nullptr) {
        lv_timer_delete(g_blink_timer);
        g_blink_timer = nullptr;
    }

    if (g_focus_index < g_row_count && g_rows[g_focus_index].value_label != nullptr) {
        lv_obj_clear_flag(g_rows[g_focus_index].value_label, LV_OBJ_FLAG_HIDDEN);
    }
}

void blink_timer_cb(lv_timer_t* timer) {
    LV_UNUSED(timer);

    if (!g_editing || g_focus_index >= g_row_count || g_rows[g_focus_index].value_label == nullptr) {
        stop_blink();
        return;
    }

    lv_obj_t* label = g_rows[g_focus_index].value_label;
    if (lv_obj_has_flag(label, LV_OBJ_FLAG_HIDDEN)) {
        lv_obj_clear_flag(label, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    }
}

void update_header() {
    if (g_title_label == nullptr || g_page_label == nullptr) {
        return;
    }

    char page_text[8] = {};
    lv_label_set_text(g_title_label, current_page().title);
    std::snprintf(page_text, sizeof(page_text), "%u/%u",
                  static_cast<unsigned>(g_current_page + 1),
                  static_cast<unsigned>(PAGE_COUNT));
    lv_label_set_text(g_page_label, page_text);
}

void update_row_value(size_t row_index) {
    if (row_index >= g_row_count || g_rows[row_index].value_label == nullptr) {
        return;
    }

    const SettingsItem& item = current_page().items[row_index];
    const float value = (g_editing && row_index == g_focus_index) ? g_edit_value : read_item_value(item);
    char buffer[32] = {};
    format_item_value(item, value, buffer, sizeof(buffer));
    lv_label_set_text(g_rows[row_index].value_label, buffer);
    if (!(g_editing && row_index == g_focus_index)) {
        lv_obj_clear_flag(g_rows[row_index].value_label, LV_OBJ_FLAG_HIDDEN);
    }
}

void refresh_all_values() {
    for (size_t i = 0; i < g_row_count; ++i) {
        update_row_value(i);
        refresh_row_visual(i);
    }
}

void rebuild_page() {
    if (g_content_area == nullptr) {
        return;
    }

    stop_blink();
    g_editing = false;
    lv_obj_clean(g_content_area);
    g_row_count = current_page().item_count;

    for (size_t i = 0; i < g_row_count; ++i) {
        const SettingsItem& item = current_page().items[i];
        lv_obj_t* row = lv_obj_create(g_content_area);
        lv_obj_set_width(row, lv_pct(100));
        lv_obj_set_height(row, 30);
        lv_obj_set_style_layout(row, LV_LAYOUT_FLEX, 0);
        lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
        lv_obj_set_style_flex_main_place(row, LV_FLEX_ALIGN_SPACE_BETWEEN, 0);
        lv_obj_set_style_flex_cross_place(row, LV_FLEX_ALIGN_CENTER, 0);
        lv_obj_set_style_pad_left(row, 8, 0);
        lv_obj_set_style_pad_right(row, 8, 0);
        lv_obj_set_style_pad_top(row, 4, 0);
        lv_obj_set_style_pad_bottom(row, 4, 0);
        lv_obj_set_style_radius(row, 3, 0);

        lv_obj_t* label = lv_label_create(row);
        lv_label_set_text(label, item.label);
        lv_obj_set_style_text_font(label, hos_14, 0);
        lv_obj_set_style_text_color(label, theme_manager::theme_current_colors().text_primary, 0);

        lv_obj_t* value_label = lv_label_create(row);
        lv_obj_set_width(value_label, 72);
        lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_RIGHT, 0);
        lv_obj_set_style_text_font(value_label, hos_14, 0);

        g_rows[i] = {row, value_label};
        update_row_value(i);
        refresh_row_visual(i);
    }

    for (size_t i = g_row_count; i < MAX_PAGE_ITEMS; ++i) {
        g_rows[i] = {};
    }

    if (g_focus_index >= g_row_count) {
        g_focus_index = 0;
    }

    update_header();
    refresh_all_values();

    // SYSTEM 页：底部显示 AP 密码
    if (g_current_page == PAGE_SYSTEM) {
        g_system_pwd_label = lv_label_create(g_content_area);
        lv_obj_set_style_text_font(g_system_pwd_label, hos_14, 0);
        lv_obj_align(g_system_pwd_label, LV_ALIGN_BOTTOM_MID, 0, -10);
        const char* pwd = app_state::wifi_ap_password;
        if (pwd[0] != '\0') {
            char buf[32];
            snprintf(buf, sizeof(buf), "密码: %s", pwd);
            lv_label_set_text(g_system_pwd_label, buf);
            lv_obj_clear_flag(g_system_pwd_label, LV_OBJ_FLAG_HIDDEN);
        } else {
            lv_obj_add_flag(g_system_pwd_label, LV_OBJ_FLAG_HIDDEN);
        }
    } else {
        g_system_pwd_label = nullptr;
    }
}

void set_focus(uint8_t new_index) {
    if (g_row_count == 0) {
        g_focus_index = 0;
        return;
    }

    if (g_focus_index < g_row_count) {
        refresh_row_visual(g_focus_index);
    }
    g_focus_index = static_cast<uint8_t>(new_index % g_row_count);
    refresh_row_visual(g_focus_index);
}

void move_focus(int delta) {
    if (g_row_count == 0 || g_editing) {
        return;
    }

    const int count = static_cast<int>(g_row_count);
    const int next = (static_cast<int>(g_focus_index) + delta + count) % count;
    set_focus(static_cast<uint8_t>(next));
}

float adjust_with_presets(const SettingsItem& item, float current_value, int delta) {
    if (item.preset_values == nullptr || item.preset_count == 0) {
        return current_value;
    }

    size_t index = 0;
    int best_distance = 0x7fffffff;
    const int rounded_current = static_cast<int>(std::lround(current_value));
    for (size_t i = 0; i < item.preset_count; ++i) {
        const int distance = std::abs(static_cast<int>(item.preset_values[i]) - rounded_current);
        if (distance < best_distance) {
            best_distance = distance;
            index = i;
        }
    }

    if (delta < 0) {
        index = (index == 0) ? 0 : (index - 1);
    } else if (delta > 0) {
        index = (index + 1 >= item.preset_count) ? (item.preset_count - 1) : (index + 1);
    }

    return static_cast<float>(item.preset_values[index]);
}

void adjust_edit_value(int delta) {
    if (!g_editing || g_focus_index >= g_row_count) {
        return;
    }

    const SettingsItem& item = current_item();
    if (item.preset_count > 0) {
        g_edit_value = adjust_with_presets(item, g_edit_value, delta);
    } else {
        const float next = g_edit_value + (static_cast<float>(delta) * item.step);
        const float clamped = clamp_value(next, item.min_val, item.max_val);
        const float snapped = std::round(clamped / item.step) * item.step;
        g_edit_value = clamp_value(snapped, item.min_val, item.max_val);
    }

    update_row_value(g_focus_index);
    lv_obj_clear_flag(g_rows[g_focus_index].value_label, LV_OBJ_FLAG_HIDDEN);
}

void enter_edit_mode() {
    if (g_editing || g_focus_index >= g_row_count) {
        return;
    }

    const SettingsItem& item = current_item();
    if (item.type == SettingsItemType::BOOL) {
        const float current = read_item_value(item);
        write_item_value(item, current > 0.5f ? 0.0f : 1.0f);
        config_manager::save_to_nvs();
        update_row_value(g_focus_index);
        refresh_row_visual(g_focus_index);

        if (g_system_pwd_label != nullptr) {
            const char* pwd = app_state::wifi_ap_password;
            if (pwd[0] != '\0') {
                char buf[32];
                snprintf(buf, sizeof(buf), "密码: %s", pwd);
                lv_label_set_text(g_system_pwd_label, buf);
                lv_obj_clear_flag(g_system_pwd_label, LV_OBJ_FLAG_HIDDEN);
            } else {
                lv_obj_add_flag(g_system_pwd_label, LV_OBJ_FLAG_HIDDEN);
            }
        }
        return;
    }

    g_editing = true;
    g_edit_value = read_item_value(item);
    update_row_value(g_focus_index);
    refresh_row_visual(g_focus_index);
    stop_blink();
    g_blink_timer = lv_timer_create(blink_timer_cb, BLINK_PERIOD_MS, nullptr);
}

void commit_edit_mode() {
    if (!g_editing || g_focus_index >= g_row_count) {
        return;
    }

    const SettingsItem& item = current_item();
    write_item_value(item, g_edit_value);

    // 如果修改的是主题模式，立即应用主题
    if (item.get_u8 == config_manager::get_theme_mode) {
        theme_manager::theme_apply_to_active_screen();
    }

    config_manager::save_to_nvs();

    g_editing = false;
    stop_blink();
    update_row_value(g_focus_index);
    refresh_row_visual(g_focus_index);
}

void cancel_edit_mode() {
    if (!g_editing) {
        return;
    }

    g_editing = false;
    stop_blink();
    update_row_value(g_focus_index);
    refresh_row_visual(g_focus_index);
}

void switch_page(int delta) {
    if (g_editing) {
        return;
    }

    const int next = (static_cast<int>(g_current_page) + delta + static_cast<int>(PAGE_COUNT)) % static_cast<int>(PAGE_COUNT);
    g_current_page = static_cast<uint8_t>(next);
    g_focus_index = 0;
    rebuild_page();
}

void show_impl() {
    init_impl();
    if (!g_initialized || g_screen == nullptr) {
        return;
    }

    g_active.store(true);
    rebuild_page();
    lv_screen_load(g_screen);
}

void hide_impl() {
    cancel_edit_mode();
    g_active.store(false);

    lv_obj_t* home = ui_bridge::screen_manager_get_home();
    if (home != nullptr) {
        lv_screen_load(home);
    }
}

void async_show(void*) {
    show_impl();
}

void async_init(void*) {
    init_impl();
}

void async_hide(void*) {
    hide_impl();
}

void dispatch_key(void* user_data) {
    KeyDispatch* dispatch = static_cast<KeyDispatch*>(user_data);
    if (dispatch == nullptr) {
        return;
    }

    if (!g_initialized || !g_active.load()) {
        delete dispatch;
        return;
    }

    if (dispatch->event == KEY_SHORT) {
        if (dispatch->key_id == KEY_K2_ID) {
            if (g_editing) {
                commit_edit_mode();
            } else {
                enter_edit_mode();
            }
        } else if (dispatch->key_id == KEY_K1_ID) {
            if (g_editing) {
                adjust_edit_value(-1);
            } else {
                move_focus(-1);
            }
        } else if (dispatch->key_id == KEY_K3_ID) {
            if (g_editing) {
                adjust_edit_value(1);
            } else {
                move_focus(1);
            }
        }
    } else if (dispatch->event == KEY_LONG) {
        if (dispatch->key_id == KEY_K2_ID) {
            hide_impl();
        } else if (dispatch->key_id == KEY_K1_ID) {
            switch_page(-1);
        } else if (dispatch->key_id == KEY_K3_ID) {
            switch_page(1);
        }
    }

    delete dispatch;
}

} // namespace

void init() {
    if (g_initialized) {
        return;
    }
    init_impl();
}

void show() {
    lv_async_call(async_show, nullptr);
}

void hide() {
    if (!g_initialized) {
        return;
    }
    lv_async_call(async_hide, nullptr);
}

void handle_key(uint8_t key_id, const KeyState& state) {
    if (!g_initialized || !g_active.load()) {
        return;
    }
    if (state.event != KEY_SHORT && state.event != KEY_LONG) {
        return;
    }

    KeyDispatch* dispatch = new KeyDispatch {key_id, state.event};
    lv_async_call(dispatch_key, dispatch);
}

bool is_active() {
    return g_active.load();
}

} // namespace settings_ui
