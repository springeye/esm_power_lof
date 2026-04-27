# ViewModel + 脏标记差分刷新 实现计划

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** 将硬编码 LVGL 控件索引的 `ui_bridge/` 重构为 ViewModel（纯 C++ 双模式） + ViewBinding（薄适配层），脏标记位掩码实现毫秒级差分刷新。

**Architecture:** ViewModel 聚合 `app_state` 原子变量到 `DashboardData`，通过值比较生成脏标记位掩码。ViewBinding 每 5ms LVGL tick 调用 `snapshot()`，仅对脏字段调用 `lv_label_set_text()`。内置 MockDataSource 可在 PC 端独立编译运行，供 UI 开发时预览。

**Tech Stack:** C++17, LVGL 9.2.2, FreeRTOS, PlatformIO (ESP32-S3 + native)

---

## 文件结构

```
新增:
  src/viewmodel/viewmodel.h       — 数据结构 + 脏标记 + Action + 接口声明
  src/viewmodel/viewmodel.cpp     — 双模式实现（正常/模拟）
  src/viewmodel/mock_source.h     — 模拟数据生成器声明
  src/viewmodel/mock_source.cpp   — 模拟数据生成器实现
  src/view_bridge/view_binding.h  — ViewBinding 接口声明
  src/view_bridge/view_binding.cpp — ViewBinding 实现

修改:
  include/app_config.h            — USE_DISPLAY_DEMO = false
  src/main.cpp                    — 移除旧 ui_bridge，接入新 ViewModel+ViewBinding
  src/app/tasks.cpp               — 按键事件改为 view_binding::handle_key()
  platformio.ini                  — build_src_filter 添加新文件

删除:
  src/ui_bridge/data_bridge.h
  src/ui_bridge/data_bridge.cpp
  src/ui_bridge/input_bridge.h
  src/ui_bridge/input_bridge.cpp

保留不动:
  src/ui_bridge/screen_manager.h/cpp  — splash→home 切换
  src/ui_bridge/lvgl_xml_stubs.c      — LVGL Editor XML stub
  src/sensors/*, src/fan/*, src/power/*, src/input/*, src/app/app_state.*
```

---

### Task 1: 创建目录结构

- [ ] **Step 1: 创建 viewmodel 和 view_bridge 目录**

```bash
mkdir -p src/viewmodel src/view_bridge
```

---

### Task 2: 编写 viewmodel.h（数据结构 + 接口声明）

**文件:** 创建 `src/viewmodel/viewmodel.h`

- [ ] **Step 1: 写入 viewmodel.h**

```cpp
#pragma once
#include <cstdint>

// ── DOMAIN TYPES ──────────────────────────────────────────────────────────
// These live in viewmodel.h so both ViewModel and ViewBinding can use them.
// Zero LVGL/Arduino/FreeRTOS dependency.

enum DirtyFlag : uint32_t {
    DIRTY_TEMP        = 1 << 0,
    DIRTY_TEMP_VALID  = 1 << 1,
    DIRTY_FAN_RPM     = 1 << 2,
    DIRTY_FAN_DUTY    = 1 << 3,
    DIRTY_FAN_PCT     = 1 << 4,
    DIRTY_CURRENT_1   = 1 << 5,
    DIRTY_CURRENT_2   = 1 << 6,
    DIRTY_CURRENT_3   = 1 << 7,
    DIRTY_CUR_VALID   = 1 << 8,   // any of the 3 channels' valid changed
    DIRTY_PSU_STATE   = 1 << 9,
    DIRTY_PWOK        = 1 << 10,
    DIRTY_FAULT       = 1 << 11,
    DIRTY_FAULT_CODE  = 1 << 12,
};

struct DashboardData {
    // temperature
    float    temp_c;
    bool     temp_valid;

    // fan
    uint32_t fan_rpm;
    uint16_t fan_duty;           // 0–1023
    uint8_t  fan_pct;            // derived: fan_duty * 100 / 1023

    // current (CH1/CH2/CH3)
    float    current_a[3];
    bool     current_valid[3];

    // power
    uint8_t  psu_state;          // 0=Off 1=Standby 2=Starting 3=On 4=Stopping 5=Fault
    bool     pwok;

    // fault
    bool     fault_active;
    uint8_t  fault_code;         // 0=none 1=overtemp 2=stall 3=overcurrent
};

enum class Action {
    POWER_TOGGLE,
    POWER_LONG_PRESS,
    FAN_MODE_AUTO,
    FAN_MODE_MANUAL,
    FAN_SET_DUTY,       // uses int param
    FAULT_CLEAR,
};

// ── VIEWMODEL PUBLIC API ──────────────────────────────────────────────────

namespace viewmodel {

void init();            // normal mode — reads app_state atomics
void init_mock();       // mock mode — simulated data

// Returns dirty-flag bitmask. Only bits for fields that changed since the
// last call are set. After this call the internal dirty state is cleared.
// Returns 0 if nothing changed — the caller may skip all UI updates.
uint32_t snapshot(DashboardData& out);

// Dispatch a user action from the View layer.
void dispatch(Action action);
void dispatch(Action action, int param);

// Mode switching (debug / preview)
bool is_mock();
void set_mock(bool on);

} // namespace viewmodel
```

---

### Task 3: 编写 mock_source.h 和 mock_source.cpp

**文件:** 创建 `src/viewmodel/mock_source.h` 和 `src/viewmodel/mock_source.cpp`

- [ ] **Step 1: 写入 mock_source.h**

```cpp
#pragma once
#include "viewmodel.h"
#include <cstdint>

namespace viewmodel {

// Call once before any snapshot() in mock mode.
void mock_init();

// Advance the internal time counter by one tick (~5ms) and compute
// the next DashboardData snapshot. Returns dirty-flag bitmask.
uint32_t mock_snapshot(DashboardData& out);

// React to a user action. Returns true if the action changed state.
bool mock_dispatch(Action action, int param);

} // namespace viewmodel
```

- [ ] **Step 2: 写入 mock_source.cpp**

```cpp
#include "mock_source.h"
#include <cmath>
#include <cstdint>

namespace {

DashboardData g_data;
uint32_t     g_step = 0;       // increments each snapshot() call (~5ms per step)
bool         g_fault_triggered = false;

// ── helpers ───────────────────────────────────────────────────────────────

float sim_temp_c(uint32_t step) {
    // 25→75→25 sine cycle, period = 60s / 0.005s = 12000 steps
    float phase = static_cast<float>(step % 12000) / 12000.0f * 6.283185f;
    return 25.0f + 25.0f * (1.0f - std::cos(phase));
}

uint16_t sim_duty(float temp) {
    if (temp < 30.0f) return 205;
    if (temp < 50.0f) return 205 + static_cast<uint16_t>((temp - 30.0f) / 20.0f * 409);
    if (temp < 70.0f) return 614 + static_cast<uint16_t>((temp - 50.0f) / 20.0f * 409);
    return 1023;
}

uint32_t sim_rpm(float temp) {
    if (temp < 25.0f) return 0;
    if (temp > 70.0f) return 3000;
    return static_cast<uint32_t>((temp - 25.0f) / 45.0f * 3000);
}

float sim_current(uint32_t step, int ch) {
    switch (ch) {
        case 0: return 1.2f + std::sin(static_cast<float>(step) * 0.001047f) * 0.4f;   // sin(t/3)
        case 1: return 0.8f + std::cos(static_cast<float>(step) * 0.000628f) * 0.25f;   // cos(t/5)
        case 2: return 2.5f + std::sin(static_cast<float>(step) * 0.000449f) * 0.6f;    // sin(t/7)
        default: return 0.0f;
    }
}

// Return bits that changed between old and new values for a float field.
uint32_t diff_f(float old_v, float new_v, DirtyFlag bit) {
    return (old_v != new_v) ? static_cast<uint32_t>(bit) : 0;
}

uint32_t diff_b(bool old_v, bool new_v, DirtyFlag bit) {
    return (old_v != new_v) ? static_cast<uint32_t>(bit) : 0;
}

uint32_t diff_u32(uint32_t old_v, uint32_t new_v, DirtyFlag bit) {
    return (old_v != new_v) ? static_cast<uint32_t>(bit) : 0;
}

uint32_t diff_u16(uint16_t old_v, uint16_t new_v, DirtyFlag bit) {
    return (old_v != new_v) ? static_cast<uint32_t>(bit) : 0;
}

uint32_t diff_u8(uint8_t old_v, uint8_t new_v, DirtyFlag bit) {
    return (old_v != new_v) ? static_cast<uint32_t>(bit) : 0;
}

} // anonymous namespace

namespace viewmodel {

void mock_init() {
    g_step = 0;
    g_fault_triggered = false;
    g_data = DashboardData{};
}

uint32_t mock_snapshot(DashboardData& out) {
    uint32_t dirty = 0;
    ++g_step;

    // ── temperature ──────────────────────────────────────────────────────
    float temp = sim_temp_c(g_step);
    dirty |= diff_f(g_data.temp_c, temp, DIRTY_TEMP);
    g_data.temp_c = temp;
    g_data.temp_valid = true;   // always valid in mock

    // ── fan ──────────────────────────────────────────────────────────────
    uint16_t duty = sim_duty(temp);
    dirty |= diff_u16(g_data.fan_duty, duty, DIRTY_FAN_DUTY);
    g_data.fan_duty = duty;

    uint8_t pct = static_cast<uint8_t>(static_cast<uint32_t>(duty) * 100 / 1023);
    dirty |= diff_u8(g_data.fan_pct, pct, DIRTY_FAN_PCT);
    g_data.fan_pct = pct;

    uint32_t rpm = sim_rpm(temp);
    dirty |= diff_u32(g_data.fan_rpm, rpm, DIRTY_FAN_RPM);
    g_data.fan_rpm = rpm;

    // ── current ──────────────────────────────────────────────────────────
    for (int i = 0; i < 3; ++i) {
        float cur = sim_current(g_step, i);
        dirty |= diff_f(g_data.current_a[i], cur,
                        (i == 0) ? DIRTY_CURRENT_1 : (i == 1) ? DIRTY_CURRENT_2 : DIRTY_CURRENT_3);
        g_data.current_a[i] = cur;
        g_data.current_valid[i] = true;
    }

    // ── PSU state ────────────────────────────────────────────────────────
    uint8_t psu_new;
    {
        uint32_t cycle = (g_step / 200) % 60;  // each state lasts ~1s (200 ticks)
        if (cycle < 1)       psu_new = 1;       // Standby
        else if (cycle < 2)  psu_new = 2;       // Starting
        else                 psu_new = 3;       // On
    }
    if (g_fault_triggered) psu_new = 5;          // Fault
    dirty |= diff_u8(g_data.psu_state, psu_new, DIRTY_PSU_STATE);
    g_data.psu_state = psu_new;

    bool pwok_new = (psu_new == 3);               // Only ON state has PWOK high
    dirty |= diff_b(g_data.pwok, pwok_new, DIRTY_PWOK);
    g_data.pwok = pwok_new;

    // ── fault ────────────────────────────────────────────────────────────
    bool fault_new = ((g_step % 9000) > 8900);     // ~45s cycle, 5s active
    if (fault_new && !g_fault_triggered) {
        g_fault_triggered = true;
    }
    if (!fault_new) g_fault_triggered = false;
    bool fault_active_new = fault_new || g_fault_triggered;
    dirty |= diff_b(g_data.fault_active, fault_active_new, DIRTY_FAULT);
    g_data.fault_active = fault_active_new;

    uint8_t fault_code_new = fault_active_new ? 1 : 0;
    dirty |= diff_u8(g_data.fault_code, fault_code_new, DIRTY_FAULT_CODE);
    g_data.fault_code = fault_code_new;

    // ── output ───────────────────────────────────────────────────────────
    out = g_data;
    return dirty;
}

bool mock_dispatch(Action action, int param) {
    (void)param;
    switch (action) {
        case Action::FAULT_CLEAR:
            g_fault_triggered = false;
            g_data.fault_active = false;
            g_data.fault_code = 0;
            g_data.psu_state = 3;   // back to On
            g_data.pwok = true;
            return true;
        case Action::POWER_TOGGLE:
            // toggle between On(3) and Off(0)
            g_data.psu_state = (g_data.psu_state == 3) ? 0 : 3;
            g_data.pwok = (g_data.psu_state == 3);
            return true;
        case Action::POWER_LONG_PRESS:
            g_data.psu_state = 4;   // Stopping
            g_data.pwok = false;
            return true;
        case Action::FAN_MODE_AUTO:
        case Action::FAN_MODE_MANUAL:
        case Action::FAN_SET_DUTY:
            return true;            // no-op in mock (fan follows temp curve)
    }
    return false;
}

} // namespace viewmodel
```

---

### Task 4: 编写 viewmodel.cpp（正常模式 + 模拟模式统一入口）

**文件:** 创建 `src/viewmodel/viewmodel.cpp`

- [ ] **Step 1: 写入 viewmodel.cpp — 头文件 + 内部状态**

```cpp
#include "viewmodel.h"
#include "mock_source.h"

#ifndef ESP32
// Native / PC — always mock mode
#else
#include "app/app_state.h"
#include "app/fault_guard.h"
#include "power/ps_on.h"
#endif

namespace {

bool     g_is_mock = false;
DashboardData g_cached;

// ── normal mode snapshot helpers ──────────────────────────────────────────

#ifdef ESP32

uint32_t normal_snapshot(DashboardData& out) {
    uint32_t dirty = 0;

    // temperature
    float t = app_state::get_temp_c();
    if (g_cached.temp_c != t) { g_cached.temp_c = t; dirty |= DIRTY_TEMP; }
    bool tv = (t > -40.0f && t < 150.0f);
    if (g_cached.temp_valid != tv) { g_cached.temp_valid = tv; dirty |= DIRTY_TEMP_VALID; }

    // fan rpm
    uint32_t rpm = app_state::get_rpm();
    if (g_cached.fan_rpm != rpm) { g_cached.fan_rpm = rpm; dirty |= DIRTY_FAN_RPM; }

    // fan duty + derived pct
    uint16_t duty = app_state::get_duty();
    if (g_cached.fan_duty != duty) {
        g_cached.fan_duty = duty;
        dirty |= DIRTY_FAN_DUTY;
        uint8_t pct = static_cast<uint8_t>(static_cast<uint32_t>(duty) * 100 / 1023);
        if (g_cached.fan_pct != pct) {
            g_cached.fan_pct = pct;
            dirty |= DIRTY_FAN_PCT;
        }
    }

    // current CH1/CH2/CH3
    float ch[3] = { app_state::get_ch1_a(), app_state::get_ch2_a(), app_state::get_ch3_a() };
    for (int i = 0; i < 3; ++i) {
        if (g_cached.current_a[i] != ch[i]) {
            g_cached.current_a[i] = ch[i];
            dirty |= (i == 0) ? DIRTY_CURRENT_1 : (i == 1) ? DIRTY_CURRENT_2 : DIRTY_CURRENT_3;
        }
    }
    // current_valid: all valid while no I2C error counter overflow
    // (placeholder — always true unless we add error tracking)
    bool cv = true;
    if (g_cached.current_valid[0] != cv) {
        g_cached.current_valid[0] = g_cached.current_valid[1] = g_cached.current_valid[2] = cv;
        dirty |= DIRTY_CUR_VALID;
    }

    // psu state
    uint8_t ps = app_state::psu_state_id.load();
    if (g_cached.psu_state != ps) { g_cached.psu_state = ps; dirty |= DIRTY_PSU_STATE; }

    // pwok — derived from psu_state for now (real PWOK read requires GPIO)
    bool pw = (ps == 3);
    if (g_cached.pwok != pw) { g_cached.pwok = pw; dirty |= DIRTY_PWOK; }

    // fault
    bool fa = app_state::is_fault();
    if (g_cached.fault_active != fa) { g_cached.fault_active = fa; dirty |= DIRTY_FAULT; }
    uint8_t fc = fa ? 1 : 0;   // simplified: fault_code derived from active flag
    if (g_cached.fault_code != fc) { g_cached.fault_code = fc; dirty |= DIRTY_FAULT_CODE; }

    out = g_cached;
    return dirty;
}

void normal_dispatch(Action action, int param) {
    switch (action) {
        case Action::POWER_TOGGLE:
            // Toggle: use cached psu_state to decide
            if (g_cached.psu_state == 3) {  // PSU_ON
                ps_on_deassert();
            } else {
                ps_on_assert();
            }
            break;
        case Action::POWER_LONG_PRESS:
            ps_on_deassert();
            break;
        case Action::FAULT_CLEAR:
            fault_guard::clear();
            app_state::set_fault(false);
            break;
        case Action::FAN_MODE_AUTO:
        case Action::FAN_MODE_MANUAL:
        case Action::FAN_SET_DUTY:
            (void)param;
            // Fan mode — placeholder, to be wired when fan_control supports manual mode.
            break;
    }
}

#else  // !ESP32 — native/PC always uses mock

uint32_t normal_snapshot(DashboardData& out) {
    return mock_snapshot(out);   // fallback
}

void normal_dispatch(Action action, int param) {
    mock_dispatch(action, param);
}

#endif // ESP32

} // anonymous namespace

// ── PUBLIC API ────────────────────────────────────────────────────────────

namespace viewmodel {

void init() {
    g_is_mock = false;
    g_cached = DashboardData{};
#ifdef ESP32
    // nothing extra needed — app_state is initialized by tasks
#endif
}

void init_mock() {
    g_is_mock = true;
    g_cached = DashboardData{};
    mock_init();
}

uint32_t snapshot(DashboardData& out) {
    if (g_is_mock) {
        return mock_snapshot(out);
    }
    return normal_snapshot(out);
}

void dispatch(Action action) {
    dispatch(action, 0);
}

void dispatch(Action action, int param) {
    if (g_is_mock) {
        mock_dispatch(action, param);
    } else {
        normal_dispatch(action, param);
    }
}

bool is_mock() { return g_is_mock; }

void set_mock(bool on) {
    if (on && !g_is_mock) init_mock();
    else if (!on && g_is_mock) init();
}

} // namespace viewmodel
```

---

### Task 5: 编写 view_binding.h

**文件:** 创建 `src/view_bridge/view_binding.h`

- [ ] **Step 1: 写入 view_binding.h**

```cpp
#pragma once
#include "viewmodel/viewmodel.h"
#include <cstdint>

struct _lv_obj_t;

namespace view_binding {

// Call once after home screen is created.
// `home` is the root lv_obj_t* from home_create().
void init(_lv_obj_t* home);

// Call from lvgl_task on Core 1 every tick (~5ms).
// Reads viewmodel::snapshot() and updates only dirty labels.
void refresh();

// Call from input_task on Core 0 when a key event is detected.
// key_id: 0=UP, 1=ENTER, 2=DOWN (matches KEY_K1/K2/K3 order).
void handle_key(uint8_t key_id);

// Register a label widget for automatic refresh.
// `name` must match lv_obj_set_name_static() in the View code.
void bind_label(const char* name, DirtyFlag flag);

} // namespace view_binding
```

---

### Task 6: 编写 view_binding.cpp

**文件:** 创建 `src/view_bridge/view_binding.cpp`

- [ ] **Step 1: 写入 view_binding.cpp**

```cpp
#include "view_binding.h"
#include "lvgl/lvgl.h"
#include <cstdio>
#include <cmath>

namespace {

// ── widget registry ───────────────────────────────────────────────────────

static constexpr int MAX_BINDINGS = 16;

struct Binding {
    const char* name;      // widget name (must match lv_obj_set_name_static)
    DirtyFlag   flag;      // dirty flag that triggers this widget's update
};

Binding g_bindings[MAX_BINDINGS];
lv_obj_t* g_widgets[MAX_BINDINGS] = {nullptr};
int       g_binding_count = 0;

lv_obj_t* g_home = nullptr;

// ── formatter ─────────────────────────────────────────────────────────────

void fmt_temp(lv_obj_t* label, const DashboardData& d) {
    char buf[16];
    if (!d.temp_valid || std::isnan(d.temp_c) || std::isinf(d.temp_c)) {
        std::snprintf(buf, sizeof(buf), "-- C");
    } else {
        std::snprintf(buf, sizeof(buf), "%.1f C", d.temp_c);
    }
    lv_label_set_text(label, buf);
}

void fmt_rpm(lv_obj_t* label, const DashboardData& d) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%lu RPM", static_cast<unsigned long>(d.fan_rpm));
    lv_label_set_text(label, buf);
}

void fmt_duty(lv_obj_t* label, const DashboardData& d) {
    char buf[16];
    std::snprintf(buf, sizeof(buf), "%u%%", d.fan_pct);
    lv_label_set_text(label, buf);
}

void fmt_current(lv_obj_t* label, const DashboardData& d, int ch) {
    char buf[16];
    if (!d.current_valid[ch] || std::isnan(d.current_a[ch]) || std::isinf(d.current_a[ch])) {
        std::snprintf(buf, sizeof(buf), "-- A");
    } else {
        std::snprintf(buf, sizeof(buf), "%.1f A", d.current_a[ch]);
    }
    lv_label_set_text(label, buf);
}

void fmt_psu_state(lv_obj_t* label, const DashboardData& d) {
    static const char* names[] = {"Off", "Standby", "Starting", "On", "Stopping", "Fault"};
    lv_label_set_text(label, names[d.psu_state % 6]);
}

void fmt_fault(lv_obj_t* label, const DashboardData& d) {
    lv_label_set_text(label, d.fault_active ? "FAULT" : "OK");
}

void update_widget(int idx, const DashboardData& data) {
    if (!g_widgets[idx]) return;
    DirtyFlag f = g_bindings[idx].flag;
    if (f == DIRTY_TEMP || f == DIRTY_TEMP_VALID) {
        fmt_temp(g_widgets[idx], data);
    } else if (f == DIRTY_FAN_RPM) {
        fmt_rpm(g_widgets[idx], data);
    } else if (f == DIRTY_FAN_DUTY || f == DIRTY_FAN_PCT) {
        fmt_duty(g_widgets[idx], data);
    } else if (f == DIRTY_CURRENT_1) {
        fmt_current(g_widgets[idx], data, 0);
    } else if (f == DIRTY_CURRENT_2) {
        fmt_current(g_widgets[idx], data, 1);
    } else if (f == DIRTY_CURRENT_3) {
        fmt_current(g_widgets[idx], data, 2);
    } else if (f == DIRTY_PSU_STATE) {
        fmt_psu_state(g_widgets[idx], data);
    } else if (f == DIRTY_FAULT || f == DIRTY_FAULT_CODE) {
        fmt_fault(g_widgets[idx], data);
    }
    // DIRTY_PWOK / DIRTY_CUR_VALID — no dedicated widget, ignored
}

// ── key handling ──────────────────────────────────────────────────────────

uint32_t g_focus_idx = 0;
uint32_t g_focusable_count = 0;

// Collect all focusable widgets (those with name starting with "focus_")
// LVGL doesn't have a built-in focus system in this project, so we do minimal
// cycling. Focus state is indicated by LV_STATE_FOCUSED on the widget.

lv_obj_t* get_focusable(uint32_t idx) {
    if (g_focusable_count == 0) return nullptr;
    uint32_t actual = idx % g_focusable_count;
    // Search bindings for focus_* named widgets
    uint32_t found = 0;
    for (int i = 0; i < g_binding_count; ++i) {
        if (g_bindings[i].name && g_bindings[i].name[0] == 'f') {
            // crude: widget IS focusable if it's in bindings with a valid pointer
            if (g_widgets[i] && found == actual) return g_widgets[i];
            ++found;
        }
    }
    g_focusable_count = found;
    return nullptr;
}

struct KeyEvent {
    uint8_t key_id;
};

void do_key_event(void* user_data) {
    auto* ev = static_cast<KeyEvent*>(user_data);
    if (!ev || !g_home) { delete ev; return; }

    // Re-count focusable
    g_focusable_count = 0;
    for (int i = 0; i < g_binding_count; ++i) {
        if (g_widgets[i]) ++g_focusable_count;
    }
    if (g_focusable_count == 0) { delete ev; return; }

    lv_obj_t* prev = get_focusable(g_focus_idx);

    switch (ev->key_id) {
        case 0: // UP
            g_focus_idx = (g_focus_idx > 0) ? g_focus_idx - 1 : g_focusable_count - 1;
            break;
        case 2: // DOWN
            g_focus_idx = (g_focus_idx + 1) % g_focusable_count;
            break;
        case 1: // ENTER
            {
                lv_obj_t* target = get_focusable(g_focus_idx);
                if (target) lv_obj_send_event(target, LV_EVENT_CLICKED, nullptr);
            }
            delete ev;
            return;
    }

    if (prev) lv_obj_clear_state(prev, LV_STATE_FOCUSED);
    lv_obj_t* next = get_focusable(g_focus_idx);
    if (next) lv_obj_add_state(next, LV_STATE_FOCUSED);

    delete ev;
}

} // anonymous namespace

// ── PUBLIC API ────────────────────────────────────────────────────────────

namespace view_binding {

void init(lv_obj_t* home) {
    g_home = home;
    g_binding_count = 0;
    for (int i = 0; i < MAX_BINDINGS; ++i) {
        g_widgets[i] = nullptr;
        g_bindings[i].name = nullptr;
    }
}

void bind_label(const char* name, DirtyFlag flag) {
    if (g_binding_count >= MAX_BINDINGS) return;
    if (!g_home) return;

    lv_obj_t* w = lv_obj_get_child_by_name(g_home, name);
    if (!w) return;   // widget not found in current UI — skip gracefully

    g_bindings[g_binding_count] = {name, flag};
    g_widgets[g_binding_count] = w;
    ++g_binding_count;
}

void refresh() {
    DashboardData data;
    uint32_t dirty = viewmodel::snapshot(data);
    if (dirty == 0) return;

    for (int i = 0; i < g_binding_count; ++i) {
        if (dirty & g_bindings[i].flag) {
            update_widget(i, data);
        }
    }
}

void handle_key(uint8_t key_id) {
    if (!g_home) return;
    // Must marshal to LVGL thread (Core 1) since we're called from input_task (Core 0).
    auto* ev = new KeyEvent{key_id};
    lv_async_call(do_key_event, ev);
}

} // namespace view_binding
```

---

### Task 7: 修改 include/app_config.h

**文件:** 修改 `include/app_config.h` 第 63 行

- [ ] **Step 1: 把 USE_DISPLAY_DEMO 改为 false**

将：
```cpp
static constexpr bool USE_DISPLAY_DEMO = true;
```
改为：
```cpp
static constexpr bool USE_DISPLAY_DEMO = false;
```

---

### Task 8: 修改 src/main.cpp

**文件:** 修改 `src/main.cpp`

- [ ] **Step 1: 替换 #include 和 init 调用**

将：
```cpp
#include "ui_bridge/screen_manager.h"
#include "ui_bridge/data_bridge.h"
#include "ui_bridge/input_bridge.h"
```
替换为：
```cpp
#include "ui_bridge/screen_manager.h"
#include "view_bridge/view_binding.h"
```

- [ ] **Step 2: 替换 setup() 中的桥接层 init**

将：
```cpp
    lof_power_system_init(NULL);
    ui_bridge::screen_manager_init(2000);
    ui_bridge::data_bridge_attach(ui_bridge::screen_manager_get_home());
    ui_bridge::data_bridge_init();
    ui_bridge::input_bridge_attach_home(ui_bridge::screen_manager_get_home());
```
替换为：
```cpp
    // ViewModel — normal mode (reads app_state atomics)
    // Use viewmodel::init_mock() for PC-side preview without hardware
    viewmodel::init();

    // LVGL Editor UI bootstrap
    lof_power_system_init(NULL);
    ui_bridge::screen_manager_init(2000);

    // ViewBinding — thin LVGL adapter
    view_binding::init(ui_bridge::screen_manager_get_home());
    // Widget bindings: register named labels from the View for dirty-flag refresh.
    // IMPORTANT: update widget names here when UI is redesigned in LVGL Editor.
    // view_binding::bind_label("temp_label", DIRTY_TEMP);
    // view_binding::bind_label("rpm_label",  DIRTY_FAN_RPM);
    // view_binding::bind_label("ch1_label",  DIRTY_CURRENT_1);
    // view_binding::bind_label("ch2_label",  DIRTY_CURRENT_2);
    // view_binding::bind_label("ch3_label",  DIRTY_CURRENT_3);
    // view_binding::bind_label("psu_label",  DIRTY_PSU_STATE);
    // view_binding::bind_label("fault_label", DIRTY_FAULT);
```

---

### Task 9: 修改 src/app/tasks.cpp

**文件:** 修改 `src/app/tasks.cpp`

- [ ] **Step 1: 替换 include**

将：
```cpp
#include "../ui_bridge/input_bridge.h"
```
替换为：
```cpp
#include "../view_bridge/view_binding.h"
```

- [ ] **Step 2: 在 lvgl_task 中添加 view_binding::refresh() 调用**

在 `lvgl_task` 函数中，`lvgl_port::task_handler();` 之后添加 `view_binding::refresh();`：

```cpp
void lvgl_task(void* /*param*/) {
    esp_task_wdt_add(nullptr);
    for (;;) {
        lvgl_port::tick_increment();
        lvgl_port::task_handler();
        view_binding::refresh();    // dirty-flag differential refresh

        esp_task_wdt_reset();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}
```

- [ ] **Step 3: 替换 input_task 中的按键处理函数调用**

将：
```cpp
                ui_bridge::input_handle_key(k, s_keys[k]);
```
替换为：
```cpp
                view_binding::handle_key(k);
```

---

### Task 10: 更新 platformio.ini

**文件:** 修改 `platformio.ini`

- [ ] **Step 1: 在 build_src_filter 中添加新文件，移除旧文件**

在 `build_src_filter` 中添加 viewmodel 和 view_bridge 目录的文件：

```ini
build_src_filter =
    +<*>
    +<../ui/lof_power_system.c>
    +<../ui/lof_power_system_gen.c>
    +<../ui/screens/home_gen.c>
    +<../ui/screens/splash_gen.c>
    +<../ui/fonts/hos_14_data.c>
    +<../ui/fonts/hos_bold_big_data.c>
    +<../ui/fonts/hos_regular_data.c>
    +<../src/compat/lvgl_v8_shim.cpp>
    +<../src/viewmodel/viewmodel.cpp>
    +<../src/viewmodel/mock_source.cpp>
    +<../src/view_bridge/view_binding.cpp>
    -<ui/_legacy/>
```

- [ ] **Step 2: 在 build_flags 中添加 include 路径**

```ini
build_flags =
    -DUSER_SETUP_LOADED=1
    ...(keep all existing flags)...
    -I src/viewmodel
    -I src/view_bridge
    -Iinclude
    -Isrc
    -I ui
    -I ui/screens
    -I ui/fonts
```

---

### Task 11: 删除旧 ui_bridge 文件

- [ ] **Step 1: 删除 4 个文件**

```bash
git rm src/ui_bridge/data_bridge.h
git rm src/ui_bridge/data_bridge.cpp
git rm src/ui_bridge/input_bridge.h
git rm src/ui_bridge/input_bridge.cpp
```

---

### Task 12: 首次编译 fix-loop

- [ ] **Step 1: 编译 ESP32-S3**

```bash
pio run -e esp32s3
```

期: exit 0, 无 error

- [ ] **Step 2: 如有编译错误，按类处理**

| 错误类型 | 修复动作 |
|----------|---------|
| `undefined reference to lv_xml_register_font` | 确认 lvgl_xml_stubs.c 在 build_src_filter 中 |
| `'app_state.h' file not found` for viewmodel.cpp | 检查 -Isrc 在 build_flags 中 |
| `'lvgl/lvgl.h' file not found` for view_binding.cpp | 检查 LVGL include 路径 |
| `multiple definition of ...` | 检查重复文件包含 |
| `'ps_on_assert' was not declared` | 检查 ps_on.h include 路径在 viewmodel.cpp 中是否正确 |

- [ ] **Step 3: 编译成功后检查固件大小**

```bash
pio run -e esp32s3 -t size
```

---

### Task 13: 编写并运行 native 单元测试

**文件:** 修改/新增 test 文件

- [ ] **Step 1: 确认 native 测试编译包含 viewmodel 文件**

修改 `platformio.ini` 的 `[env:native]` 段：

```ini
[env:native]
platform = native
test_framework = unity
extra_scripts = pre:scripts/use_msys2_mingw.py
build_flags = -std=c++17 -Iinclude -Isrc -Isrc/viewmodel
test_build_src = yes
build_src_filter =
    +<sensors/ntc/ntc.cpp>
    +<fan/fan_curve.cpp>
    +<power/psu_fsm.cpp>
    +<input/keys.cpp>
    +<native_main.cpp>
    +<viewmodel/viewmodel.cpp>
    +<viewmodel/mock_source.cpp>
lib_deps =
```

- [ ] **Step 2: 编写 native 测试**

创建 `test/native/test_viewmodel/test_vm.cpp`：

```cpp
#include <unity.h>
#include "viewmodel/viewmodel.h"
#include <cmath>

void test_mock_init_sets_mode() {
    viewmodel::init_mock();
    TEST_ASSERT_TRUE(viewmodel::is_mock());
}

void test_mock_snapshot_returns_nonzero_dirty_on_first_call() {
    viewmodel::init_mock();
    DashboardData data;
    uint32_t dirty = viewmodel::snapshot(data);
    TEST_ASSERT_TRUE(dirty != 0);   // first call all fields are "changed"
}

void test_mock_snapshot_returns_fewer_dirty_on_second_call() {
    viewmodel::init_mock();
    DashboardData data;
    uint32_t d1 = viewmodel::snapshot(data);  // first call: all fields "changed" from zero
    uint32_t d2 = viewmodel::snapshot(data);  // second call: only fast-changing fields dirty
    // After just 1 step (~5ms simulated), slow fields like temp should be unchanged.
    // Fast oscillating fields (current) will still register as dirty.
    // The important invariant: d2 is a subset of d1 for slow-changing fields.
    TEST_ASSERT_FALSE(d2 & DIRTY_TEMP);        // temp barely moves in one step
    TEST_ASSERT_FALSE(d2 & DIRTY_FAN_RPM);     // fan follows temp
    TEST_ASSERT_FALSE(d2 & DIRTY_PSU_STATE);   // PSU state changes every 1s
    TEST_ASSERT_FALSE(d2 & DIRTY_FAULT);       // fault not triggered
}

void test_mock_data_in_range() {
    viewmodel::init_mock();
    DashboardData data;
    viewmodel::snapshot(data);

    TEST_ASSERT_FLOAT_WITHIN(5.0f, 25.0f, data.temp_c);   // starts near 25
    TEST_ASSERT_TRUE(data.temp_valid);
    TEST_ASSERT_TRUE(data.fan_rpm >= 0);
    TEST_ASSERT_TRUE(data.fan_duty >= 205);
    TEST_ASSERT_TRUE(data.fan_duty <= 1023);
    TEST_ASSERT_TRUE(data.fan_pct >= 20);
    TEST_ASSERT_TRUE(data.fan_pct <= 100);
    TEST_ASSERT_TRUE(data.current_valid[0]);
    TEST_ASSERT_TRUE(data.current_valid[1]);
    TEST_ASSERT_TRUE(data.current_valid[2]);
    TEST_ASSERT_TRUE(data.current_a[0] > 0.0f);
    TEST_ASSERT_TRUE(data.current_a[1] > 0.0f);
    TEST_ASSERT_TRUE(data.current_a[2] > 0.0f);
    TEST_ASSERT_TRUE(data.psu_state <= 5);
    TEST_ASSERT_FALSE(data.fault_active);   // no fault at start
}

void test_mock_fault_clear_works() {
    viewmodel::init_mock();
    // We can't easily trigger fault in a test without advancing many steps,
    // but we can verify dispatch() doesn't crash.
    viewmodel::dispatch(Action::FAULT_CLEAR);
    // Should not crash and should clear any active fault
    DashboardData data;
    viewmodel::snapshot(data);
    TEST_ASSERT_FALSE(data.fault_active);
}

void test_mock_power_toggle() {
    viewmodel::init_mock();
    viewmodel::dispatch(Action::POWER_TOGGLE);
    DashboardData data;
    viewmodel::snapshot(data);
    TEST_ASSERT_EQUAL_UINT8(0, data.psu_state);   // toggled from On(3) to Off(0)
    TEST_ASSERT_FALSE(data.pwok);
}

void test_mode_switch() {
    viewmodel::init_mock();
    TEST_ASSERT_TRUE(viewmodel::is_mock());
    viewmodel::set_mock(false);
    TEST_ASSERT_FALSE(viewmodel::is_mock());
    viewmodel::set_mock(true);
    TEST_ASSERT_TRUE(viewmodel::is_mock());
}

void test_dirty_flag_temp_and_rpm_separate() {
    viewmodel::init_mock();
    DashboardData data;
    uint32_t d1 = viewmodel::snapshot(data);
    // d1 should have both DIRTY_TEMP and DIRTY_FAN_RPM set on first call
    TEST_ASSERT_TRUE(d1 & DIRTY_TEMP);
    TEST_ASSERT_TRUE(d1 & DIRTY_FAN_RPM);
}

int main() {
    UNITY_BEGIN();
    RUN_TEST(test_mock_init_sets_mode);
    RUN_TEST(test_mock_snapshot_returns_nonzero_dirty_on_first_call);
    RUN_TEST(test_mock_snapshot_returns_fewer_dirty_on_second_call);
    RUN_TEST(test_mock_data_in_range);
    RUN_TEST(test_mock_fault_clear_works);
    RUN_TEST(test_mock_power_toggle);
    RUN_TEST(test_mode_switch);
    RUN_TEST(test_dirty_flag_temp_and_rpm_separate);
    return UNITY_END();
}
```

- [ ] **Step 3: 运行 native 测试**

```bash
pio test -e native
```

期: 8 tests PASS, 0 FAIL

---

### Task 14: PC 端独立编译验证

- [ ] **Step 1: 创建最小 PC 编译测试**

```bash
cd src/viewmodel && g++ -std=c++17 -c -o /dev/null mock_source.cpp viewmodel.cpp 2>&1
```

期: 编译成功（可能需要设 ESP32 宏来走正常路径）。如 ESP32 宏未定义，mock_source 路径仍应成功编译。

---

### Task 15: 最终提交

- [ ] **Step 1: 确认所有改动**

```bash
git status
git diff --stat
```

- [ ] **Step 2: 提交**

```bash
git add -A
git commit -m "$(cat <<'EOF'
feat(viewmodel): add ViewModel + dirty-flag differential refresh architecture

- Add src/viewmodel/: pure C++ ViewModel with dual mode (normal/mock)
- Add src/view_bridge/: thin LVGL adapter for dirty-flag partial refresh
- Remove old ui_bridge (data_bridge, input_bridge) — replaced by ViewBinding
- Modify main.cpp, tasks.cpp to wire new architecture
- Set USE_DISPLAY_DEMO=false to enable real firmware path
- Add native unit tests for ViewModel mock mode
- Keep screen_manager and lvgl_xml_stubs.c in ui_bridge/

ViewModel snapshot() returns dirty-flag bitmask — only changed fields
trigger lv_label_set_text(), achieving millisecond-level differential
refresh. MockDataSource enables PC-side UI development without hardware.

Co-Authored-By: Claude Opus 4.7 <noreply@anthropic.com>
EOF
)"
```

---

## 验证清单

- [ ] `pio run -e esp32s3` exit 0
- [ ] `pio test -e native` 8 tests PASS
- [ ] `src/viewmodel/mock_source.cpp` 可在 PC 端 `g++ -std=c++17` 编译
- [ ] 旧 `ui_bridge/data_bridge.* input_bridge.*` 已删除
- [ ] `screen_manager.*` 和 `lvgl_xml_stubs.c` 保留且可编译
- [ ] `src/sensors/` `src/fan/` `src/power/` 文件未修改
- [ ] `ui/` 下 `*_gen.c` 文件未修改
