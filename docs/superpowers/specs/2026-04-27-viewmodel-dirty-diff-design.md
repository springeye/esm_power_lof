# ViewModel + 脏标记差分刷新 架构设计

## TL;DR

将现有硬编码 LVGL 控件索引的 `ui_bridge/` 重构为两层：**ViewModel**（纯 C++，零 LVGL 依赖，含模拟数据源） + **ViewBinding**（薄适配层，唯一依赖 LVGL）。数据用脏标记位掩码实现毫秒级差分推送：哪个字段变了刷哪个控件，不变不刷。

## 架构分层

```
┌────────────────────────────────────────────────┐
│ View (ui/)                                      │
│ LVGL Editor 导出，控件通过 lv_obj_set_name_*()  │
│ 命名。仅负责布局/样式/动画。                     │
├────────────────────────────────────────────────┤
│ ViewBinding (src/view_bridge/view_binding.cpp)  │
│ 薄适配层 ~150 行。通过名字查找 LVGL 控件，      │
│ 每 LVGL tick(5ms) 调用 snapshot()，按 dirty     │
│ mask 局部刷新控件。控件事件转译 Action 分发。    │
│ 唯一依赖 LVGL 头文件的层。                       │
├────────────────────────────────────────────────┤
│ ViewModel (src/viewmodel/)                      │
│ 纯 C++，无 LVGL/Arduino/FreeRTOS 依赖。         │
│ 聚合 app_state → DashboardData。                │
│ 脏标记追踪数据变更。                             │
│ 接收 Action → 转调 Model 层。                    │
│ 内置 MockDataSource（离机模式循环假数据）。       │
├────────────────────────────────────────────────┤
│ Model (src/*) — 现有代码基本不动                 │
│ app_state 原子变量 / sensors / fan / power /    │
│ keys / FreeRTOS tasks                          │
└────────────────────────────────────────────────┘
```

## ViewModel 层

### 数据方向（Model → ViewModel → ViewBinding → View）

### 命令方向（View → ViewBinding → ViewModel → Model）

### DashboardData（聚合快照）

```cpp
struct DashboardData {
    // 温度
    float    temp_c;
    bool     temp_valid;

    // 风扇
    uint32_t fan_rpm;
    uint16_t fan_duty;       // 0-1023
    uint8_t  fan_pct;        // 0-100，由 fan_duty*100/1023 计算得出

    // 电流（三路输出接口）
    float    current_a[3];   // CH1/CH2/CH3
    bool     current_valid[3];

    // 电源
    uint8_t  psu_state;      // 0=Off 1=Standby 2=Starting 3=On 4=Stopping 5=Fault
    bool     pwok;

    // 故障
    bool     fault_active;
    uint8_t  fault_code;     // 0=无 1=过温 2=堵转 3=过流
};
```

### DirtyFlag（变更位掩码）

```cpp
enum DirtyFlag : uint32_t {
    DIRTY_TEMP        = 1 << 0,
    DIRTY_TEMP_VALID  = 1 << 1,
    DIRTY_FAN_RPM     = 1 << 2,
    DIRTY_FAN_DUTY    = 1 << 3,
    DIRTY_FAN_PCT     = 1 << 4,
    DIRTY_CURRENT_1   = 1 << 5,
    DIRTY_CURRENT_2   = 1 << 6,
    DIRTY_CURRENT_3   = 1 << 7,
    DIRTY_CUR_VALID   = 1 << 8,  // 三路任一 valid 变化时置位
    DIRTY_PSU_STATE   = 1 << 9,
    DIRTY_PWOK        = 1 << 10,
    DIRTY_FAULT       = 1 << 11,
    DIRTY_FAULT_CODE  = 1 << 12,
};
```

### Action（View → ViewModel）

```cpp
enum class Action {
    POWER_TOGGLE,
    POWER_LONG_PRESS,
    FAN_MODE_AUTO,
    FAN_MODE_MANUAL,
    FAN_SET_DUTY,       // 带参数
    FAULT_CLEAR,
};
```

### ViewModel 对外接口

```cpp
namespace viewmodel {

// 初始化
void init();            // 正常模式，从 app_state 读
void init_mock();       // 模拟模式，循环假数据

/**
 * 获取当前数据快照，返回脏标记位掩码。
 * 仅置位自上次 snapshot() 以来变更的字段。
 * 调用后内部脏标记清零。
 * 若返回 0 表示无变化，ViewBinding 可跳过所有控件更新。
 */
uint32_t snapshot(DashboardData& out);

// 操作指令
void dispatch(Action action);
void dispatch(Action action, int param);

// 模式切换
bool is_mock();
void set_mock(bool on);

}
```

### 脏标记更新机制

**正常模式**：`snapshot()` 内部逐个读取 `app_state` 原子变量，与内部缓存的上次值比较，值不同的字段置入 `g_dirty` 位掩码，同时刷新缓存。

```cpp
// viewmodel.cpp snapshot() 内部（正常模式）
uint32_t snapshot(DashboardData& out) {
    uint32_t result = 0;

    float t = app_state::get_temp_c();
    if (g_cached.temp_c != t) {
        g_cached.temp_c = t;
        result |= DIRTY_TEMP;
    }
    // ... 同理对比 rpm、duty、电流、psu_state、fault 等

    out = g_cached;        // 把最新缓存 copy 给调用方
    return result;
}
```

**模拟模式**：mock_source 推进 → 写内部缓存 → 值比较 → 置脏标记，逻辑同上，只是数据源换成模拟生成器。

### 模拟数据源（MockDataSource）

时间驱动：每次 `snapshot()` 调用时内部推进一次步进，不依赖 FreeRTOS 延时。

| 字段 | 模拟规律 |
|------|---------|
| temp_c | 25°C → 75°C → 25°C 正弦周期 60s，步进量由调用频率自动计算 |
| fan_rpm | 跟随温度曲线：20°C=0 rpm, 75°C=3000 rpm |
| fan_duty | 跟随温度：20°C=205, 75°C=1023 |
| current_a[0] | 1.2A + sin(t/3)*0.4A |
| current_a[1] | 0.8A + cos(t/5)*0.25A |
| current_a[2] | 2.5A + sin(t/7)*0.6A |
| psu_state | 循环：Standby→Starting→On→On...(持续)，10s 后重新循环 |
| pwok | 跟随 psu_state：On=high，其他=low |
| fault_active | 每 45s 触发一次过温故障，5s 后自动清除 |

模拟模式下 `dispatch()` 正常工作时数据会相应变化（如 `FAULT_CLEAR` 清除故障）。

### 编译约束

- **正常模式**（`viewmodel::init()`）：运行时读取 `app_state` 原子变量，适用于 ESP32 实际运行
- **模拟模式**（`viewmodel::init_mock()`）：适用于任何平台（包括 PC），零硬件依赖，用 `<cmath>` 生成假数据
- 两模式可在同一份代码中共存，通过编译宏或运行时标志切换；ViewModel 头文件不依赖平台头文件

## ViewBinding 层

### 职责边界

- 唯一使用 LVGL API 的适配层
- 通过控件名字（`lv_obj_get_child_by_name()` 或用户自定义命名）查找 View 控件
- 每 5ms（LVGL tick 频率）调用 `viewmodel::snapshot()`，按脏标记局部更新控件
- 绑定控件事件（如按键、点击）→ 转译 `viewmodel::dispatch()`
- **不做**：数据聚合、业务逻辑、传感器读写

### 刷新流程

ViewBinding 的 `refresh()` 由 LVGL 任务（每 5ms tick）调用，不是独立 timer。

```
lvgl_task 循环 (Core 1, 每 5ms)
  │
  ├── lv_tick_inc(5)
  ├── lv_timer_handler()          // LVGL 内部渲染
  │
  └── view_binding::refresh()     // 数据刷新
        │
        ├── dirty = viewmodel::snapshot(data)
        ├── if (dirty == 0) → return（无变化，跳过所有控件操作）
        │
        ├── if (dirty & DIRTY_TEMP) → lv_label_set_text(g_temp_label, ...)
        ├── if (dirty & DIRTY_FAN_RPM) → lv_label_set_text(g_rpm_label, ...)
        └── ...仅更新有标记的字段...
```

`lv_refr_now()` 不在此处调用——LVGL 在下一轮 `lv_timer_handler()` 自动渲染变更。

### 控件查找

ViewBinding 内部维护一个 `std::map<std::string, lv_obj_t*>` 或固定数组，View 初始化时按控件名字填充。

```cpp
// ViewBinding 内部注册表，UI 重做时只改这里的名字字符串
struct BindingEntry {
    const char* widget_name;  // View 侧控件通过 lv_obj_set_name_static() 设置的名字
    DirtyFlag   flag;         // 哪个脏标记触发更新
    void      (*formatter)(lv_obj_t*, const DashboardData&);
};
```

## Model 层需要修改的项

### app_config.h
- `USE_DISPLAY_DEMO` 改为 `false`

### main.cpp
- 移除对 `data_bridge`、`input_bridge`（旧 ui_bridge）的引用
- 接入 ViewModel + ViewBinding 初始化
- 保留 `screen_manager` 的 splash → home 切换逻辑

### 现有 ui_bridge/ 目录
- `data_bridge.cpp` — 删除，由 ViewModel + ViewBinding 替代
- `input_bridge.cpp` — 删除，按键事件由 ViewBinding 统一处理
- `screen_manager.cpp` — 保留，职责不变（splash → home 切换）
- `lvgl_xml_stubs.c` — 保留，LVGL Editor 导出需要

### tasks.cpp
- `input_task` 中按键事件不再调 `ui_bridge::input_handle_key`，改为调 `viewmodel::dispatch(Action)`

## 文件变更清单

| 操作 | 文件 | 说明 |
|------|------|------|
| 新增 | `src/viewmodel/viewmodel.h` | ViewModel 接口 + 数据结构 + 脏标记定义 |
| 新增 | `src/viewmodel/viewmodel.cpp` | ViewModel 实现（正常/模拟双模式） |
| 新增 | `src/viewmodel/mock_source.h` | 模拟数据生成器 |
| 新增 | `src/viewmodel/mock_source.cpp` | 模拟数据实现 |
| 新增 | `src/view_bridge/view_binding.h` | ViewBinding 接口 |
| 新增 | `src/view_bridge/view_binding.cpp` | ViewBinding 实现 |
| 修改 | `src/main.cpp` | 移除旧 ui_bridge，接入新 ViewModel+ViewBinding |
| 修改 | `src/app/tasks.cpp` | 按键事件改为 viewmodel::dispatch() |
| 修改 | `include/app_config.h` | USE_DISPLAY_DEMO=false |
| 修改 | `platformio.ini` | build_src_filter 添加新文件路径 |
| 删除 | `src/ui_bridge/data_bridge.h` | 被 ViewModel 替代 |
| 删除 | `src/ui_bridge/data_bridge.cpp` | 被 ViewModel 替代 |
| 删除 | `src/ui_bridge/input_bridge.h` | 被 ViewModel 替代 |
| 删除 | `src/ui_bridge/input_bridge.cpp` | 被 ViewModel 替代 |
| 保留 | `src/ui_bridge/screen_manager.h/cpp` | splash → home 切换 |
| 保留 | `src/ui_bridge/lvgl_xml_stubs.c` | LVGL Editor XML stub |

## 验证方式

- `pio run -e esp32s3` 编译通过
- `pio test -e native` 对 ViewModel 单元测试（snapshot 脏标记正确性、模拟数据合理性）
- PC 端独立编译 ViewModel（用模拟模式），验证可脱离 ESP32 环境运行

## 不做什么

- 不修改 `src/sensors/`、`src/fan/`、`src/power/` 对外 API
- 不修改 `ui/` 下任何 `*_gen.c` 文件
- 不引入 WiFi/BLE/OTA/NVS
- 不引入新依赖库
- 不修改 LVGL 配置
- 不做 View 层开发（由用户后续在 LVGL Editor 完成）
