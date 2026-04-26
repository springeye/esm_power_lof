# UI 集成工作计划：LVGL Editor `ui/` 接入主固件

## TL;DR

> **Quick Summary**: 将 `ui/` 中 LVGL Editor 9.x 导出的 splash + home 屏幕集成到主 ESP32 固件，接通真实传感器（NTC 温度、PCNT 转速、INA226 ×3 电流）显示，并接入 KEY_UP/DOWN/ENTER 物理按键导航。
>
> **Deliverables**:
> - 归档 `src/ui/` 旧 UI 至 `src/ui/_legacy/`，新 UI 完全由 `ui/` 子工程驱动
> - `platformio.ini` 接入 `ui/` 源 + include 路径，排除 `preview-build/`、`preview-bin/`
> - `include/lv_conf.h` 显式 `LV_USE_XML=0`，提供 `lv_xml_register_font` 弱符号 stub
> - 新建 `src/ui_bridge/` 桥接层：`screen_manager`（splash→home 切换）+ `data_bridge`（传感器→控件）+ `input_bridge`（按键→home 状态）
> - `src/main.cpp` 改用 `lof_power_system_init(NULL)` 替代旧 `ui_main/ui_menu/ui_events::init()`
> - `pio run -e esp32dev` 编译通过；`pio run -t size` 显示 Flash/RAM 占用在预算内
>
> **Estimated Effort**: Medium
> **Parallel Execution**: YES - 4 waves
> **Critical Path**: T1 → T2 → T3 → T6 → T9 → T11 → T13 → F1-F4 → user okay

---

## Context

### Original Request
> "@ui/ 帮我集成这个目录中的UI代码"

### Interview Summary

**Key Discussions**:
- **Q1 (旧 UI 处理)**: 完全替换 → 移到 `src/ui/_legacy/` + `#if 0` 禁用编译（Q8 复确认）
- **Q2 (分辨率适配)**: 用户已在 LVGL Editor 中重新编辑 XML 并导出代码（240×280 已生效）
- **Q3 (集成深度)**: 里程碑 3 — 完整数据接入（温度/转速/3 路电流实时）+ 按键导航
- **Q4 (构建集成方式)**: PlatformIO `build_src_filter` + `extra_includes`，单段构建，**不引入** ui/CMakeLists.txt 二段构建
- **Q5 (验证策略)**: 仅 `pio run -e esp32dev` + `-t size` 验证；无单元测试；无硬件烧录 QA
- **Q6 (字体处理)**: 用户已在 Editor 中缩减子集（hos_14 + hos_bold_big + hos_regular = 216KB 总计，巨型 18MB 字体已剔除）
- **Q7 (LVGL XML 运行时)**: 禁用 `LV_USE_XML`，走纯 C `*_create()` 路径
- **Q9 (splash 切换)**: 固定时长 2 秒后自动切换到 home

**Research Findings**:
- ✅ `ui/project.xml`: `width=240 height=280` 已与硬件匹配
- ✅ `ui/file_list_gen.cmake` 仅列 7 个 .c：3 字体 + lof_power_system{,.gen}.c + screens/{home,splash}_gen.c
- ✅ 入口 `void lof_power_system_init(const char * asset_path)` (ui/lof_power_system.h:37)
- ✅ 屏幕函数 `lv_obj_t * home_create(void)` (home_gen.h:38)、`lv_obj_t * splash_create(void)` (splash_gen.h:38)
- ⚠️ **关键风险**: `ui/lof_power_system_gen.c:108-110` 调用 `lv_xml_register_font(NULL, "hos_bold_big", hos_bold_big)` 等 3 处 XML API
  - 主固件 `lv_conf.h` 当前未启用 LV_USE_XML（默认 0）→ 链接器会报 undefined reference
  - 解决方案：在桥接层提供弱符号 stub（noop 实现），保持生成文件不变
- ✅ home_gen.c 使用 `lv_obj_set_name_static(obj, "home_#")` 命名机制 → 桥接层可用 `lv_obj_get_child_by_name()` 寻址控件（LVGL 9.x API）
- ✅ `src/main.cpp` 现有初始化序列：`i2c_bus_init → spi_bus_init → tft_driver::init → lvgl_port::init → ui_main::init → ui_menu::init → ui_events::init → 传感器/风扇/电源 init → watchdog::init → tasks::start_all`
- ✅ `platformio.ini` 当前 `build_src_filter` 为空，`build_flags = -std=c++17 -Iinclude -Isrc`
- ✅ `lv_conf.h`: LV_COLOR_DEPTH=16, LV_MEM_SIZE=32KB, 启用了 Montserrat 14/16/20/28

### Pre-Plan Gap Analysis (Self-Reviewed, Metis Skipped Due to Timeout)

**Identified Gaps & Resolutions**:
- **lv_xml_register_font 链接错误**: 提供弱符号 stub `__attribute__((weak)) void lv_xml_register_font(void*, const char*, const lv_font_t*) {}` 在桥接层
- **头文件路径冲突**: ui/ 用 `#include "lvgl/lvgl.h"`，主固件 `-Ilib/lvgl/src` 或类似 → 在 `build_flags` 添加 `-Ilib` 让 `lvgl/lvgl.h` 可解析（待验证）
- **widget 寻址**: home.xml 中具体 widget id 未知 → 任务 T10 独立扫描 home_gen.c 中所有 `lv_obj_set_name_static` 调用，列出可用 id
- **传感器值获取 API**: 不修改 sensors API → 桥接层调用现有 getter（`ntc_read_celsius()`、`ina226_read_*()`、`fan_tach_get_rpm()`，需在 T8 中确认实际函数名）
- **数据更新频率**: 默认 200ms（5 Hz），LVGL task 内执行
- **NaN/超限值显示**: 显示 "--" 字符（应用层 sanitize）

---

## Work Objectives

### Core Objective
将 `ui/` LVGL Editor 导出工程接入主固件构建，实现 splash→home 启动序列、5Hz 传感器数据刷新、3 键导航 home，达到可编译且 Flash/RAM 在预算内的固件状态。

### Concrete Deliverables
1. `platformio.ini` — 修改 `build_src_filter` 与 `build_flags`，接入 `ui/` 源与 include
2. `include/lv_conf.h` — 显式 `#define LV_USE_XML 0` 与字体启用调整
3. `src/ui/_legacy/` — 旧 ui_main/ui_menu/ui_events 移入此目录并 `#if 0` 禁用
4. `src/ui_bridge/lvgl_xml_stubs.c` — XML API 弱符号 stub
5. `src/ui_bridge/screen_manager.{h,cpp}` — splash→home 计时切换
6. `src/ui_bridge/data_bridge.{h,cpp}` — 5Hz 传感器→home 控件刷新
7. `src/ui_bridge/input_bridge.{h,cpp}` — keys→home 焦点/激活事件
8. `src/main.cpp` — 改用 `lof_power_system_init(NULL)` + 桥接层 init
9. `.sisyphus/evidence/` — 编译日志、size 报告、桥接层单元行为证据

### Definition of Done
- [ ] `pio run -e esp32dev` exit code 0，无 error/warning（unused warning 可接受）
- [ ] `pio run -e esp32dev -t size` Flash 使用 < 6MB（8MB 总量留 2MB 余量），RAM < 200KB
- [ ] 编译产物 `.pio/build/esp32dev/firmware.elf` 存在
- [ ] `nm firmware.elf | grep lof_power_system_init` 显示符号存在
- [ ] `nm firmware.elf | grep -E 'splash_create|home_create'` 显示两个符号存在
- [ ] `src/ui/_legacy/` 中所有 .cpp 文件首行均为 `#if 0`，末行为 `#endif`
- [ ] `pio run` 输出中无 `undefined reference` 错误
- [ ] 桥接层 3 个文件均存在且包含 `init()` 入口

### Must Have
- 完全替换 `src/ui/` 旧三个模块（移至 `_legacy/` 并禁用编译）
- `lof_power_system_init()` 在 lvgl_port 初始化后被调用一次
- splash 显示 ~2 秒后通过 `lv_scr_load(home)` 切换
- home 上的温度/转速/3 路电流值每 200ms 刷新
- KEY_UP/DOWN/ENTER 在 home 屏幕产生有效响应（焦点切换或值变化）
- `ui/AGENTS.md` 约定：不修改任何 `*_gen.c/h` 文件
- `preview-build/`, `preview-bin/` 不参与固件编译

### Must NOT Have (Guardrails)
- ❌ 不修改 `ui/lof_power_system_gen.c`、`ui/screens/*_gen.c/h`（生成文件）
- ❌ 不修改 `src/sensors/`、`src/fan/`、`src/power/` 对外 API
- ❌ 不引入 LVGL XML 运行时（不启用 LV_USE_XML）
- ❌ 不引入 WiFi/BLE/OTA/NVS
- ❌ 不引入 LittleFS/SPIFFS（asset_path 传 NULL）
- ❌ 不引入 FreeType/tiny_ttf（字体已为 C 数组）
- ❌ 不"顺手"优化 `lvgl_port.cpp`、`tft_driver.cpp`
- ❌ 不"顺手"重构 `_legacy/` 内代码
- ❌ 不引入新依赖库（platformio.ini `lib_deps` 不增项）
- ❌ 不创建 README/MD 之外的文档
- ❌ 不写 mock/fake 传感器数据用于 UI 测试（用真实 getter；启动时若返回 NaN 则显示 "--"）

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** - 所有验证由 agent 直接执行命令完成。

### Test Decision
- **Infrastructure exists**: YES（test/native + Unity，但本次不使用）
- **Automated tests**: NO（用户 Q5 明确选择仅构建验证）
- **Framework**: bun (PlatformIO core toolchain)
- **Verification primary tool**: `pio run -e esp32dev` + `pio run -e esp32dev -t size` + `nm`/`objdump` 符号检查

### QA Policy
- 每个任务的 QA Scenarios 通过 PowerShell + PlatformIO CLI + nm/objdump/Select-String 完成
- 证据文件保存到 `.sisyphus/evidence/task-{N}-{slug}.{txt|log}`
- **没有硬件烧录、没有视觉测试、没有 Playwright**
- 行为验证依赖编译时检查 + 符号表 + 静态分析

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Start Immediately - 准备 + 隔离):
├── T1: 创建 src/ui/_legacy/ 目录并移动旧文件 [quick]
├── T2: 在 _legacy/ 文件首尾包裹 #if 0 / #endif [quick]
└── T3: 创建 src/ui_bridge/ 目录骨架 (3 头文件 + 1 stub.c) [quick]

Wave 2 (After Wave 1 - 配置 + 桥接层实现) [Round 2 修订: T8 已删除]:
├── T4: 修改 platformio.ini (build_src_filter + build_flags) [quick]
├── T5: 修改 include/lv_conf.h (LV_USE_XML=0 + 相关项) [quick]
├── T6: 实现 src/ui_bridge/lvgl_xml_stubs.c (3 个 weak stub) [quick]
├── T7: 实现 src/ui_bridge/screen_manager.{h,cpp} (eager home + splash 2s timer) [unspecified-low]
├── T10: 调研 home_gen.c widget 层级 → 输出索引图 (lv_obj_get_child) [explore]
├── T9: 实现 src/ui_bridge/data_bridge.{h,cpp} (5Hz 刷新, app_state 数据源, 依赖 T10) [unspecified-low]
└── T11: 实现 src/ui_bridge/input_bridge.{h,cpp} (input_handle_key 替代, 依赖 T10) [unspecified-low]

Wave 3 (After Wave 2 - 主入口接线):
├── T12: 修改 src/main.cpp (替换 ui_main/menu/events init 为 lof_power_system + 桥接层 init) [quick]
└── T13: 首次编译 pio run -e esp32dev 抢救式 fix-loop (依赖 T4-T12) [deep]

Wave FINAL (4 并行评审):
├── F1: Plan compliance audit (oracle)
├── F2: Code quality review (unspecified-high)
├── F3: Build artifact QA (unspecified-high)
└── F4: Scope fidelity check (deep)
→ Present results → Get explicit user okay

Critical Path: T1 → T3 → T4 → T6 → T10 → T9 → T11 → T12 → T13 → F1-F4 → user okay
Parallel Speedup: ~50% faster than sequential
Max Concurrent: 7 (Wave 2, T8 已删除)
```

### Dependency Matrix

| Task | Depends On | Blocks |
|---|---|---|
| T1 | — | T2, T12 |
| T2 | T1 | T13 |
| T3 | — | T6, T7, T9, T11 |
| T4 | — (T1 推荐先做) | T13 |
| T5 | — | T13 |
| T6 | T3 | T13 |
| T7 | T3 | T9, T11, T12, T13 |
| T8 | — DELETED — | — |
| T9 | T3, T10 | T12, T13 |
| T10 | — | T9, T11 |
| T11 | T3, T10 | T12, T13 |
| T12 | T1, T7, T9, T11 | T13 |
| T13 | T2, T4, T5, T6, T12 | F1-F4 |
| F1-F4 | T13 | user okay |

### Agent Dispatch Summary

| Wave | Tasks | Categories |
|---|---|---|
| 1 | 3 | T1-T3 → `quick` |
| 2 | 7 | T4-T6 → `quick`, T7/T9/T11 → `unspecified-low`, T10 → `explore` (subagent_type) [T8 已删除] |
| 3 | 2 | T12 → `quick`, T13 → `deep` |
| FINAL | 4 | F1 → `oracle`, F2 → `unspecified-high`, F3 → `unspecified-high`, F4 → `deep` |

---

## Plan Revision Note (Momus Round 1 - 2026-04-26)

Momus REJECT 提出 3 个真实阻塞，全部通过证据确认：

1. **`home_gen.c` 仅命名根对象 `"home_#"`，无子控件命名** → 放弃 `lv_obj_get_child_by_name()` 路径，T9 改用 **app_state 数据源 + 索引访问**；T10 改为输出"控件层级图与目标 label 索引路径"
2. **当前固件无 LVGL keypad indev**，按键由 `tasks.cpp::input_task` 通过 GPIO 直读后调 `ui_events::handle_key(key, KeyState)` → T11 改为提供 `ui_bridge::input_handle_key(key, KeyState)` 替代函数，**绕开 LVGL group/indev**；T12 让 `tasks.cpp::input_task` 调用新函数（**新增 T12 子步骤**）
3. **sensor API 与 plan 假设不符**：实际为 `ntc_adc_to_temp(uint16_t)` 与 `ina226_read(rail, *out)`，**且** `src/app/app_state.h:46-52` 已暴露 UI 安全的 atomic getter（`get_temp_c/get_rpm/get_ch1_a/ch2_a/ch3_a`） → **删除 T8**（无需调研），T9 改用 `app_state::get_*()`

修订使桥接层以 **app_state 为权威 UI 数据源**、**复用现有按键事件路径**，避免引入新的硬件 I/O 与新的 indev 注册。

---

## TODOs

- [x] 1. 创建 `src/ui/_legacy/` 目录并移动旧 UI 文件

  **What to do**:
  - `mkdir src/ui/_legacy`
  - `git mv src/ui/ui_main.h src/ui/ui_main.cpp src/ui/ui_menu.h src/ui/ui_menu.cpp src/ui/ui_events.h src/ui/ui_events.cpp src/ui/_legacy/`
  - 验证 `src/ui/` 仅剩 `_legacy/` 子目录

  **Must NOT do**:
  - 不删除文件（仅移动）；不修改文件内容（T2 处理 `#if 0`）

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单纯文件移动，6 个文件，无逻辑变更
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO（首位前置，T2/T12 依赖此完成）
  - **Wave**: 1
  - **Blocks**: T2, T12
  - **Blocked By**: None

  **References**:
  - `src/ui/ui_main.{h,cpp}` - 待移动文件 1
  - `src/ui/ui_menu.{h,cpp}` - 待移动文件 2
  - `src/ui/ui_events.{h,cpp}` - 待移动文件 3
  - 用户决策 Q8: "移到 src/ui/_legacy/ + #if 0 禁用编译" - 此任务执行"移到"部分

  **Acceptance Criteria**:
  - [ ] `Test-Path src/ui/_legacy/ui_main.cpp` → True
  - [ ] `Test-Path src/ui/_legacy/ui_menu.cpp` → True
  - [ ] `Test-Path src/ui/_legacy/ui_events.cpp` → True
  - [ ] `Test-Path src/ui/ui_main.cpp` → False（已移走）
  - [ ] `git status` 显示 6 个 renamed 项

  **QA Scenarios**:

  ```
  Scenario: 文件成功移动
    Tool: Bash (PowerShell)
    Preconditions: src/ui/ 包含 ui_main/menu/events 共 6 个文件
    Steps:
      1. 执行: Get-ChildItem src/ui/_legacy/ -File | Measure-Object | Select-Object -ExpandProperty Count
      2. 执行: Get-ChildItem src/ui/ -File | Measure-Object | Select-Object -ExpandProperty Count
    Expected Result: 第 1 步输出 6，第 2 步输出 0
    Failure Indicators: _legacy/ 不存在；文件数 != 6；src/ui/ 仍有 .cpp/.h 文件
    Evidence: .sisyphus/evidence/task-1-file-move.txt

  Scenario: git 识别为重命名（保留历史）
    Tool: Bash
    Preconditions: 文件已移动且未提交
    Steps:
      1. 执行: git status --short | Select-String 'R\s+src/ui/' | Measure-Object -Line
    Expected Result: 6 行 R (renamed)
    Failure Indicators: 显示 D (delete) + ?? (untracked) 而非 R
    Evidence: .sisyphus/evidence/task-1-git-rename.txt
  ```

  **Commit**: NO（与 T2-T13 合并为单提交）

- [x] 2. 在 `_legacy/` 文件首尾包裹 `#if 0` / `#endif`

  **What to do**:
  - 对 `src/ui/_legacy/` 中 3 个 .cpp 与 3 个 .h 文件，每个文件：
    - 首行插入 `#if 0  // legacy UI - replaced by ui/ subproject`
    - 末行追加 `#endif // legacy UI`

  **Must NOT do**:
  - 不修改原有代码内容；不删除任何行；不调整缩进

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 6 个文件简单包裹，无逻辑分析
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 1
  - **Blocks**: T13
  - **Blocked By**: T1

  **References**:
  - 用户决策 Q8: "移到 src/ui/_legacy/ + #if 0 禁用编译" - 此任务执行"#if 0 禁用编译"部分
  - C 预处理器 `#if 0 ... #endif` 标准用法：将整文件作为编译时不可见

  **Acceptance Criteria**:
  - [ ] 6 个文件全部首行 == `#if 0  // legacy UI - replaced by ui/ subproject`
  - [ ] 6 个文件全部末行 == `#endif // legacy UI`

  **QA Scenarios**:

  ```
  Scenario: 全部文件正确包裹
    Tool: Bash (PowerShell)
    Preconditions: T1 完成，6 个文件在 _legacy/
    Steps:
      1. 执行: Get-ChildItem src/ui/_legacy/ -File | ForEach-Object { $first = (Get-Content $_.FullName -TotalCount 1); $last = (Get-Content $_.FullName | Select-Object -Last 1); "$($_.Name): first='$first' last='$last'" }
    Expected Result: 6 行输出，每行 first 含 "#if 0"、last 含 "#endif"
    Failure Indicators: 任一文件 first 不含 "#if 0" 或 last 不含 "#endif"
    Evidence: .sisyphus/evidence/task-2-wrap.txt
  ```

  **Commit**: NO

- [x] 3. 创建 `src/ui_bridge/` 目录骨架

  **What to do**:
  - `mkdir src/ui_bridge`
  - 创建占位文件（仅骨架，T6/T7/T9/T11 填充实现）：
    - `src/ui_bridge/lvgl_xml_stubs.c` （空文件或 `// placeholder`）
    - `src/ui_bridge/screen_manager.h` (`#pragma once` + 函数声明占位)
    - `src/ui_bridge/screen_manager.cpp` (`#include "screen_manager.h"`)
    - `src/ui_bridge/data_bridge.h`
    - `src/ui_bridge/data_bridge.cpp`
    - `src/ui_bridge/input_bridge.h`
    - `src/ui_bridge/input_bridge.cpp`

  **Must NOT do**:
  - 不实现任何函数体（属于 T6/T7/T9/T11）
  - 不引入 LVGL 头（推迟到对应实现任务，避免 LSP 提前报错）

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 创建空骨架文件
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 1
  - **Blocks**: T6, T7, T9, T11
  - **Blocked By**: None

  **References**:
  - 计划 "Concrete Deliverables" 4-7 项 - 桥接层 4 个 .c/.cpp 模块

  **Acceptance Criteria**:
  - [ ] `src/ui_bridge/` 存在
  - [ ] 7 个文件全部存在（1 个 .c + 3 个 .h + 3 个 .cpp）
  - [ ] 每个 .h 含 `#pragma once`

  **QA Scenarios**:

  ```
  Scenario: 骨架文件齐全
    Tool: Bash (PowerShell)
    Preconditions: 无
    Steps:
      1. 执行: (Get-ChildItem src/ui_bridge/ -File).Count
      2. 执行: (Get-ChildItem src/ui_bridge/*.h -File | ForEach-Object { Select-String -Path $_.FullName -Pattern '#pragma once' }).Count
    Expected Result: 第 1 步 == 7；第 2 步 == 3
    Failure Indicators: 文件数 != 7；任一 .h 缺少 `#pragma once`
    Evidence: .sisyphus/evidence/task-3-skeleton.txt
  ```

  **Commit**: NO

- [x] 4. 修改 `platformio.ini`（接入 ui/ 源 + include 路径）

  **What to do**:
  - 修改 `[env:esp32dev]` 中的两个键：
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
      -<ui/_legacy/>
    build_flags =
      -std=c++17
      -Iinclude
      -Isrc
      -I ui
      -I ui/screens
      -I ui/fonts
    ```
  - 不引入 `+<../ui/preview-build/>` 与 `+<../ui/preview-bin/>`
  - 不修改 `lib_deps`、`platform`、`board`、`framework`

  **Must NOT do**:
  - 不用 wildcard `+<../ui/**>`（会拉入 preview-build/preview-bin）
  - 不修改 LVGL 库版本

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单文件配置编辑
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 2
  - **Blocks**: T13
  - **Blocked By**: None（可与 T1 并行，但 T1 先做更安全）

  **References**:
  - `platformio.ini` 当前内容 - 现有 `build_flags = -std=c++17 -Iinclude -Isrc`
  - `ui/file_list_gen.cmake` - 7 个 .c 文件清单（与 build_src_filter 中 7 项一一对应）
  - PlatformIO 文档: `build_src_filter` 语法 `+<>` 添加，`-<>` 排除，相对路径基于 `src_dir`
  - 用户决策 Q4: "用 build_src_filter + extra_includes" - 此任务实现该决策

  **Acceptance Criteria**:
  - [ ] `platformio.ini` 含 `+<../ui/lof_power_system_gen.c>` 字串
  - [ ] `platformio.ini` 含 `-I ui` 字串
  - [ ] 不含 `preview-build` 或 `preview-bin` 字串
  - [ ] `lib_deps` 段内容未变（git diff 该段为空）

  **QA Scenarios**:

  ```
  Scenario: build_src_filter 包含全部 7 个 ui/ 源
    Tool: Bash (PowerShell)
    Steps:
      1. 执行: Select-String -Path platformio.ini -Pattern '\+<\.\./ui/' | Measure-Object -Line | Select-Object -ExpandProperty Lines
    Expected Result: == 7
    Failure Indicators: != 7 表示遗漏或多余
    Evidence: .sisyphus/evidence/task-4-filter.txt

  Scenario: include 路径全部就绪
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path platformio.ini -Pattern '-I ui($|/screens|/fonts)' | Measure-Object -Line
    Expected Result: 3 行匹配
    Evidence: .sisyphus/evidence/task-4-includes.txt

  Scenario: preview 目录未被纳入
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path platformio.ini -Pattern 'preview-(build|bin)' -Quiet
    Expected Result: $false
    Failure Indicators: $true 表示禁忌目录被纳入
    Evidence: .sisyphus/evidence/task-4-no-preview.txt

  Scenario: lib_deps 未变（边界守恒）
    Tool: Bash
    Steps:
      1. 执行: git diff platformio.ini | Select-String 'lib_deps' | Measure-Object -Line
    Expected Result: == 0（lib_deps 段没有任何 +/- 行）
    Evidence: .sisyphus/evidence/task-4-libdeps-untouched.txt
  ```

  **Commit**: NO

- [x] 5. 修改 `include/lv_conf.h`（显式 LV_USE_XML=0 + 字体启用）

  **What to do**:
  - 在合适位置（XML 章节附近）添加：
    ```c
    /* LVGL XML runtime - explicitly disabled, ui/ uses pure C create() path */
    #ifdef LV_USE_XML
    #undef LV_USE_XML
    #endif
    #define LV_USE_XML 0
    ```
  - 检查现有 Montserrat 字体启用项保持不变（14/16/20/28）
  - **不**新增 hos_* 字体启用宏（hos 字体由 ui/fonts/*_data.c 直接编译为 lv_font_t 全局变量，无需 LV_FONT_* 宏）

  **Must NOT do**:
  - 不修改 `LV_COLOR_DEPTH`（保持 16）
  - 不修改 `LV_MEM_SIZE`（保持 32KB）
  - 不启用任何新的 LV_USE_* 模块（LV_USE_FREETYPE、LV_USE_TINY_TTF 等）

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单文件局部编辑
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 2
  - **Blocks**: T13
  - **Blocked By**: None

  **References**:
  - `include/lv_conf.h` - 现有配置: LV_COLOR_DEPTH=16, LV_MEM_SIZE=32KB, Montserrat 14/16/20/28
  - LVGL 9.x 官方文档 LV_USE_XML 默认值（默认 0，但显式 define 防止上游变化）
  - 用户决策 Q7: "禁用 LV_USE_XML，走纯 C create() 路径"

  **Acceptance Criteria**:
  - [ ] `include/lv_conf.h` 含 `#define LV_USE_XML 0`
  - [ ] LV_COLOR_DEPTH 仍为 16
  - [ ] LV_MEM_SIZE 仍为 (32 * 1024U) 或等价

  **QA Scenarios**:

  ```
  Scenario: LV_USE_XML 显式 0
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path include/lv_conf.h -Pattern '^\s*#define\s+LV_USE_XML\s+0'
    Expected Result: 至少 1 行匹配
    Failure Indicators: 0 行 或 匹配为 1
    Evidence: .sisyphus/evidence/task-5-xml-off.txt

  Scenario: 关键配置未被改动
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path include/lv_conf.h -Pattern 'LV_COLOR_DEPTH\s+16'
      2. 执行: Select-String -Path include/lv_conf.h -Pattern 'LV_MEM_SIZE'
    Expected Result: 两步均匹配 ≥1 行
    Evidence: .sisyphus/evidence/task-5-untouched.txt
  ```

  **Commit**: NO

- [x] 6. 实现 `src/ui_bridge/lvgl_xml_stubs.c`（XML API 弱符号 stub）

  **What to do**:
  - 文件完整内容：
    ```c
    /**
     * @file lvgl_xml_stubs.c
     * @brief Weak symbol stubs for LVGL XML runtime APIs.
     *
     * The ui/ subproject is exported by LVGL Editor and contains calls to
     * lv_xml_register_font() in lof_power_system_gen.c. We disable LV_USE_XML
     * (see include/lv_conf.h) but still need these symbols at link time.
     *
     * These stubs are __attribute__((weak)) so that if LV_USE_XML is later
     * enabled, the real implementations from LVGL automatically override them.
     */
    #include "lvgl/lvgl.h"

    typedef struct _lv_xml_component_scope_t lv_xml_component_scope_t;

    __attribute__((weak))
    int32_t lv_xml_register_font(lv_xml_component_scope_t * scope,
                                 const char * name,
                                 const lv_font_t * font)
    {
        (void)scope; (void)name; (void)font;
        return 0;  /* LV_RESULT_OK equivalent; ui/ ignores return value */
    }
    ```
  - **不**实现 `lv_xml_register_image`、`lv_xml_register_widget` 等其他 XML API（只 stub 实际被调用的）

  **Must NOT do**:
  - 不修改 `ui/lof_power_system_gen.c` 中的 3 处调用
  - 不引入 LV_USE_XML 启用逻辑
  - 不实现 stub 之外的任何函数

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单函数 stub 实现
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 2
  - **Blocks**: T13
  - **Blocked By**: T3

  **References**:
  - `ui/lof_power_system_gen.c:108-110` - 3 处 `lv_xml_register_font(NULL, "hos_bold_big"|"hos_regular"|"hos_14", &font_var)` 调用
  - LVGL 9.x `lv_xml_register_font` 签名（来自 LVGL 源码 src/others/xml/lv_xml.h）
  - GCC `__attribute__((weak))` 弱符号机制 - 允许真实实现在启用 XML 时覆盖
  - 计划 Pre-Plan Gap Analysis "lv_xml_register_font 链接错误" 段

  **Acceptance Criteria**:
  - [ ] 文件存在且包含 `__attribute__((weak))`
  - [ ] 函数名 `lv_xml_register_font` 出现 1 次
  - [ ] 包含 `#include "lvgl/lvgl.h"`

  **QA Scenarios**:

  ```
  Scenario: stub 函数与 weak 属性正确
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path src/ui_bridge/lvgl_xml_stubs.c -Pattern '__attribute__\(\(weak\)\)' -Quiet
      2. 执行: Select-String -Path src/ui_bridge/lvgl_xml_stubs.c -Pattern 'lv_xml_register_font' | Measure-Object -Line | Select-Object -ExpandProperty Lines
    Expected Result: 第 1 步 == $true；第 2 步 == 1（只 1 处函数定义）
    Failure Indicators: weak 属性缺失；函数被定义 0 次或 >1 次
    Evidence: .sisyphus/evidence/task-6-stub.txt

  Scenario: 编译期 stub 解析（间接验证经 T13 确认无 undefined reference）
    Tool: 由 T13 验证
  ```

  **Commit**: NO

- [x] 7. 实现 `src/ui_bridge/screen_manager.{h,cpp}`（splash → home 2s 切换）

  **What to do**:
  - `screen_manager.h`：
    ```cpp
    #pragma once
    namespace ui_bridge {
    /** Show splash, then auto-load home after splash_duration_ms. */
    void screen_manager_init(uint32_t splash_duration_ms = 2000);
    /** Returns the current home screen object (NULL if not yet loaded). */
    struct _lv_obj_t* screen_manager_get_home();
    }
    ```
  - `screen_manager.cpp`：
    ```cpp
    #include "screen_manager.h"
    #include "lvgl/lvgl.h"

    extern "C" lv_obj_t* splash_create(void);
    extern "C" lv_obj_t* home_create(void);

    namespace {
        lv_obj_t* g_home = nullptr;
        void splash_timer_cb(lv_timer_t* t) {
            // home 已在 screen_manager_init 中 eager 创建（Round 2 修订）
            lv_screen_load(g_home);
            lv_timer_delete(t);
        }
    }
    namespace ui_bridge {
        void screen_manager_init(uint32_t splash_duration_ms) {
            // Round 2 修订：eager 创建 home，确保 data_bridge_attach / input_bridge_attach_home
            // 在 main.cpp 中能立即获得有效指针，避免 splash 期间 attach 到 null
            g_home = home_create();
            lv_obj_t* splash = splash_create();
            lv_screen_load(splash);
            lv_timer_t* t = lv_timer_create(splash_timer_cb, splash_duration_ms, nullptr);
            lv_timer_set_repeat_count(t, 1);
        }
        lv_obj_t* screen_manager_get_home() { return g_home; }
    }
    ```

  **Must NOT do**:
  - 不实现按键拦截（属 T11）
  - 不调用 `splash_create()` 多次
  - 不在 splash 期间销毁 home 引用

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
    - Reason: 桥接层实现，少量 LVGL API 使用
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 2
  - **Blocks**: T12, T13
  - **Blocked By**: T3

  **References**:
  - `ui/screens/splash_gen.h:38` - `lv_obj_t * splash_create(void)`
  - `ui/screens/home_gen.h:38` - `lv_obj_t * home_create(void)`
  - LVGL 9.x API: `lv_screen_load`, `lv_timer_create`, `lv_timer_set_repeat_count`, `lv_timer_delete`
  - 用户决策 Q9: "固定时长（2 秒）后自动切换"

  **Acceptance Criteria**:
  - [ ] `screen_manager.h` 含 `screen_manager_init` 与 `screen_manager_get_home` 声明
  - [ ] `screen_manager.cpp` 调用 `splash_create()` 与 `home_create()` 各 1 次
  - [ ] 含 `lv_timer_create` 与 2000ms 默认值

  **QA Scenarios**:

  ```
  Scenario: API 完整且仅调用 1 次
    Tool: Bash
    Steps:
      1. 执行: (Select-String -Path src/ui_bridge/screen_manager.cpp -Pattern 'splash_create\(\)' | Measure-Object -Line).Lines
      2. 执行: (Select-String -Path src/ui_bridge/screen_manager.cpp -Pattern 'home_create\(\)' | Measure-Object -Line).Lines
      3. 执行: Select-String -Path src/ui_bridge/screen_manager.h -Pattern 'screen_manager_init' -Quiet
    Expected Result: 第 1/2 步均 == 1；第 3 步 == $true
    Evidence: .sisyphus/evidence/task-7-screen-mgr.txt

  Scenario: 默认 2000ms
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path src/ui_bridge/screen_manager.h -Pattern '2000' -Quiet
    Expected Result: $true
    Evidence: .sisyphus/evidence/task-7-2sec.txt
  ```

  **Commit**: NO

- [ ] 8. [DELETED — Momus 修订] 不再需要 sensor API 调研

  **Status**: 已删除。`src/app/app_state.h:46-52` 已暴露 atomic UI getter（`get_temp_c()` / `get_rpm()` / `get_ch1_a()` / `get_ch2_a()` / `get_ch3_a()` / `get_duty()` / `is_fault()`），全部线程安全、无需查阅 sensor 内部 API。

  **Action**: 跳过此任务（保留编号以维持后续引用稳定）。

  **Recommended Agent Profile**: 无需调度。
  **Parallelization**: 无（已删除）。
  **Acceptance Criteria**: N/A（已删除）。
  **Commit**: NO

- [x] 9. 实现 `src/ui_bridge/data_bridge.{h,cpp}`（5Hz app_state → home labels）

  **What to do**:
  - `data_bridge.h`：
    ```cpp
    #pragma once
    struct _lv_obj_t;
    namespace ui_bridge {
    void data_bridge_init();                              // 创建 200ms LVGL timer
    void data_bridge_attach(struct _lv_obj_t* home);      // 缓存 5 个 label 句柄
    }
    ```
  - `data_bridge.cpp` 关键逻辑：
    ```cpp
    #include "data_bridge.h"
    #include "app/app_state.h"
    #include "lvgl/lvgl.h"
    #include <cmath>
    #include <cstdio>
    namespace {
        lv_obj_t* g_temp = nullptr;
        lv_obj_t* g_rpm  = nullptr;
        lv_obj_t* g_i[3] = {nullptr, nullptr, nullptr};

        // 索引由 T10 输出的 widget 层级图确定（占位常量，T10 完成后回填）
        constexpr int IDX_TEMP = -1, IDX_RPM = -1;
        constexpr int IDX_I[3] = {-1, -1, -1};

        void fmt(char* b, size_t n, float v, const char* unit) {
            if (std::isnan(v) || std::isinf(v)) std::snprintf(b, n, "--%s", unit);
            else std::snprintf(b, n, "%.1f%s", v, unit);
        }
        void refresh_cb(lv_timer_t*) {
            char buf[16];
            if (g_temp) { fmt(buf, sizeof(buf), app_state::get_temp_c(), " C"); lv_label_set_text(g_temp, buf); }
            if (g_rpm)  { fmt(buf, sizeof(buf), (float)app_state::get_rpm(), " RPM"); lv_label_set_text(g_rpm, buf); }
            float ch[3] = { app_state::get_ch1_a(), app_state::get_ch2_a(), app_state::get_ch3_a() };
            for (int i = 0; i < 3; ++i) if (g_i[i]) { fmt(buf, sizeof(buf), ch[i], " A"); lv_label_set_text(g_i[i], buf); }
        }
    }
    namespace ui_bridge {
        void data_bridge_attach(lv_obj_t* home) {
            // T10 输出层级图后，将索引回填上方常量并启用：
            // g_temp = lv_obj_get_child(home, IDX_TEMP);
            // g_rpm  = lv_obj_get_child(home, IDX_RPM);
            // for (int i = 0; i < 3; ++i) g_i[i] = lv_obj_get_child(home, IDX_I[i]);
            // 在索引未确定前 attach 不绑定，refresh_cb 是 no-op，安全。
        }
        void data_bridge_init() { lv_timer_create(refresh_cb, 200, nullptr); }
    }
    ```
  - **关键**：数据源固定为 `app_state::get_*()`（atomic、UI 安全）；widget 句柄通过 **`lv_obj_get_child(home, idx)` 索引访问**，索引值由 T10 提供。**禁用** `lv_obj_get_child_by_name`（`home_gen.c` 仅命名根对象，子控件无 name）。

  **Must NOT do**:
  - 不引入 fake/mock/random 数据
  - 不修改 sensors / fan / power / app_state 任何文件
  - 不调用 `lv_obj_get_child_by_name`（子控件无 name）
  - 不在 LVGL timer 回调内做阻塞 I/O
  - NaN/inf → "--"（不显示 "nan"/"inf"）

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 2
  - **Blocks**: T12, T13
  - **Blocked By**: T3, T10（仅索引常量回填）

  **References**:
  - `src/app/app_state.h:46-52` — **权威数据源**（atomic getter）
  - `.sisyphus/evidence/task-10-widget-tree.md` — widget 索引地图（T10 输出）
  - LVGL 9.x：`lv_obj_get_child`、`lv_label_set_text`、`lv_timer_create`

  **Acceptance Criteria**:
  - [ ] `data_bridge.h` 暴露 `data_bridge_init` + `data_bridge_attach`
  - [ ] `data_bridge.cpp` 含 `lv_timer_create(refresh_cb, 200, nullptr)`
  - [ ] NaN/inf 防护（`std::isnan` + `std::isinf`）
  - [ ] 数据来源仅用 `app_state::get_*()`
  - [ ] 不出现 `lv_obj_get_child_by_name`
  - [ ] 不出现 `rand`/`random`/`fake`/`mock`

  **QA Scenarios**:

  ```
  Scenario: 5Hz timer + NaN 防护 + app_state 数据源
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path src/ui_bridge/data_bridge.cpp -Pattern 'lv_timer_create.*200' -Quiet
      2. 执行: Select-String -Path src/ui_bridge/data_bridge.cpp -Pattern 'isnan' -Quiet
      3. 执行: (Select-String -Path src/ui_bridge/data_bridge.cpp -Pattern 'app_state::get_' | Measure-Object -Line).Lines
      4. 执行: Select-String -Path src/ui_bridge/data_bridge.cpp -Pattern 'lv_obj_get_child_by_name' -Quiet
    Expected Result: 第 1/2 行 == $true；第 3 行 ≥ 5；第 4 行 == $false
    Evidence: .sisyphus/evidence/task-9-data-bridge.txt

  Scenario: 无 fake/mock 数据
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path src/ui_bridge/data_bridge.cpp -Pattern 'rand\(|random\(|fake|mock' -Quiet
    Expected Result: $false
    Evidence: .sisyphus/evidence/task-9-no-fake.txt
  ```

  **Commit**: NO

- [x] 10. 输出 `ui/screens/home_gen.c` 控件层级图与 5 个目标 label 索引

  **What to do**:
  - explore agent 静态分析 `home_gen.c`：
    1. 解析所有 `lv_*_create(parent, ...)` 调用（共 44 个 widget），构造**父子层级树**
    2. 标注每个 widget：类型、行号、父对象变量名、在父对象 children 列表中的 **0-based 索引**
    3. 通过位置/字体/语义（如 `lv_label_create` + `hos_bold_big`）识别承载 **温度 / 转速 / I1 / I2 / I3** 的 5 个 label
    4. 输出从 `home` 根到目标 label 的**完整索引路径**（例如 `home->child(2)->child(0)`）
  - 写入 `.sisyphus/evidence/task-10-widget-tree.md`：
    - 第 1 节：完整层级树（缩进表示父子）
    - 第 2 节：5 个目标 label 表 `| 用途 | LVGL 类型 | 行号 | 索引路径 |`
    - 第 3 节：建议代码片段（`lv_obj_get_child(home, ...)` 序列）

  **Must NOT do**:
  - 不修改 `home_gen.c`
  - 不假设任何 widget 命名（仅根对象有 `"home_#"`）
  - 不猜测：每个目标 label 必须有行号 + 索引证据

  **Recommended Agent Profile**:
  - **Subagent Type**: `explore`
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 2
  - **Blocks**: T9（仅最终索引常量回填，可与 T9 并行启动）
  - **Blocked By**: None

  **References**:
  - `ui/screens/home_gen.c`（431 行，44 个 `lv_*_create`，仅根对象 `lv_obj_set_name_static(obj, "home_#")`）
  - `ui/fonts/hos_bold_big_data.c` — 大字体（数值显示候选标记）
  - LVGL 9.x：`lv_obj_get_child(parent, index)`

  **Acceptance Criteria**:
  - [ ] `.sisyphus/evidence/task-10-widget-tree.md` 存在
  - [ ] 第 2 节恰好 5 行（温度 / 转速 / I1 / I2 / I3）
  - [ ] 每行含行号 + 索引路径
  - [ ] 第 3 节给出 5 行 `lv_obj_get_child(...)` 代码

  **QA Scenarios**:

  ```
  Scenario: 层级图 + 5 label 索引输出
    Tool: Bash
    Steps:
      1. 执行: Test-Path .sisyphus/evidence/task-10-widget-tree.md
      2. 执行: (Select-String -Path .sisyphus/evidence/task-10-widget-tree.md -Pattern 'lv_obj_get_child' | Measure-Object -Line).Lines
    Expected Result: 第 1 行 == $true；第 2 行 ≥ 5
    Evidence: 文件本身
  ```

  **Commit**: NO

- [x] 11. 实现 `src/ui_bridge/input_bridge.{h,cpp}`（KEY → home，绕开 LVGL indev）

  **What to do**:
  - **架构决策（来自 Momus 修订）**：当前固件**无 keypad indev**（`src/display/lvgl_port.cpp` 只注册 display driver），且 `src/app/tasks.cpp::input_task` 已通过 GPIO 直读后调 `ui_events::handle_key(key, KeyState)`。**不引入 LVGL group/indev**，改为提供同型号替代函数 `ui_bridge::input_handle_key(key_id, KeyState)`，由 T12 中替换 `tasks.cpp` 内的调用。
  - `input_bridge.h`：
    ```cpp
    #pragma once
    #include <cstdint>
    struct KeyState;     // 由 src/input/keys.h 定义
    struct _lv_obj_t;
    namespace ui_bridge {
    void input_bridge_attach_home(struct _lv_obj_t* home);  // 缓存 home 根 + 可聚焦控件索引（来自 T10）
    void input_handle_key(uint8_t key_id, const KeyState& state);  // 替代 ui_events::handle_key
    }
    ```
  - `input_bridge.cpp` 关键逻辑：
    - 内部维护 `g_focus_idx`（0/1/2 等）+ 可聚焦控件索引数组（占位，T10 输出后回填）
    - `input_handle_key`：根据 `state.event` 判定（短按 K1/K2/K3 → KEY_UP/DOWN/ENTER）
      - KEY_UP：`g_focus_idx = (g_focus_idx - 1 + N) % N`，调 `lv_obj_add_state(target, LV_STATE_FOCUSED)` + 上一个移除
      - KEY_DOWN：`g_focus_idx = (g_focus_idx + 1) % N`，同上
      - KEY_ENTER：调 `lv_obj_send_event(target, LV_EVENT_CLICKED, NULL)`（LVGL 9.x）
    - **不**调 `lv_group_create` / `lv_indev_*` / `lv_indev_set_group`
    - **不**直读 GPIO（`digitalRead` / `gpio_get_level`）

  **Must NOT do**:
  - 不创建 LVGL group 或 indev
  - 不修改 `src/input/`、`src/display/lvgl_port.cpp`
  - 不直读 GPIO
  - 不影响 splash 屏幕（仅作用于 home）
  - 不修改 `src/ui/ui_events.{h,cpp}`（已在 T1 移到 _legacy/）

  **Recommended Agent Profile**:
  - **Category**: `unspecified-low`
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 2
  - **Blocks**: T12, T13
  - **Blocked By**: T3, T10（仅可聚焦索引回填）

  **References**:
  - `src/app/tasks.cpp:111-130` — `input_task` 当前调用 `ui_events::handle_key(k, s_keys[k])`
  - `src/ui/ui_events.h:31` — 现有签名 `void handle_key(uint8_t key, KeyState state)`，新函数沿用
  - `.sisyphus/evidence/task-10-widget-tree.md` — 可聚焦控件索引来源
  - LVGL 9.x：`lv_obj_add_state` / `lv_obj_clear_state` / `lv_obj_send_event` / `LV_STATE_FOCUSED` / `LV_EVENT_CLICKED`
  - 用户 Q3：3 键导航

  **Acceptance Criteria**:
  - [ ] `input_bridge.h` 暴露 `input_bridge_attach_home` + `input_handle_key`
  - [ ] `input_bridge.cpp` 不出现 `lv_group_create`、`lv_indev_drv_register`、`lv_indev_set_group`
  - [ ] `input_bridge.cpp` 不出现 `digitalRead`、`gpio_get_level`
  - [ ] `input_bridge.cpp` 出现 `lv_obj_send_event` 或 `lv_obj_add_state(.*FOCUSED)`

  **QA Scenarios**:

  ```
  Scenario: 不引入 indev/group，不直读 GPIO，使用 LVGL 状态/事件 API
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path src/ui_bridge/input_bridge.cpp -Pattern 'lv_group_create|lv_indev_drv_register|lv_indev_set_group' -Quiet
      2. 执行: Select-String -Path src/ui_bridge/input_bridge.cpp -Pattern 'digitalRead|gpio_get_level' -Quiet
      3. 执行: Select-String -Path src/ui_bridge/input_bridge.cpp -Pattern 'lv_obj_send_event|LV_STATE_FOCUSED' -Quiet
      4. 执行: Select-String -Path src/ui_bridge/input_bridge.h -Pattern 'input_handle_key' -Quiet
    Expected Result: 第 1 行 == $false；第 2 行 == $false；第 3/4 行 == $true
    Evidence: .sisyphus/evidence/task-11-input-bridge.txt
  ```

  **Commit**: NO
- [x] 12. 修改 `src/main.cpp`（替换旧 UI init 为 lof_power_system + 桥接层）

  **What to do**:
  - 删除 `#include "ui/ui_main.h"`、`"ui/ui_menu.h"`、`"ui/ui_events.h"`
  - 删除 `ui_main::init();` `ui_menu::init();` `ui_events::init();` 三行
  - 添加 includes：
    ```cpp
    extern "C" { #include "lof_power_system.h" }
    #include "ui_bridge/screen_manager.h"
    #include "ui_bridge/data_bridge.h"
    #include "ui_bridge/input_bridge.h"
    ```
  - 在 `lvgl_port::init()` 之后、`watchdog::init()` 之前插入：
    ```cpp
    lof_power_system_init(NULL);              // 注册字体（stub no-op）+ 全局 init
    ui_bridge::screen_manager_init(2000);     // splash → home
    ui_bridge::data_bridge_attach(ui_bridge::screen_manager_get_home());
    ui_bridge::data_bridge_init();            // 5Hz 刷新
    ui_bridge::input_bridge_attach_home(ui_bridge::screen_manager_get_home());
    // 注：input_bridge 不创建 LVGL indev/group（Momus 修订），仅暴露 input_handle_key
    ```
  - **额外步骤（Momus 修订 — 接入物理按键）**：编辑 `src/app/tasks.cpp`：
    1. 添加 `#include "ui_bridge/input_bridge.h"`
    2. 删除/移除 `#include "ui/ui_events.h"`（已 _legacy/）
    3. 在 `input_task` 函数体内将 `ui_events::handle_key(k, s_keys[k]);` 替换为 `ui_bridge::input_handle_key(k, s_keys[k]);`
    4. 其他 `input_task` 逻辑（GPIO 直读、debounce、watchdog reset、5ms 周期）**保持不变**
  - **注意 home 时序**：`screen_manager_init` 调用 `splash_create` 但 `home` 尚未创建。data_bridge_attach 必须在 splash 切到 home 后才能拿到 home 句柄。**简化方案**：在 `screen_manager_init` 中先 eager 调用 `home_create()` 缓存，但延迟 `lv_screen_load(home)`。修改 T7 的实现：splash 之前先 `g_home = home_create()` 但 `lv_screen_load(splash)`，timer 触发时仅 `lv_screen_load(g_home)`。这样 attach 在 init 时即可获得有效 home 指针。

  **Must NOT do**:
  - 不删除 `lvgl_port::init()`、`tft_driver::init()`、传感器/风扇/电源 init 调用
  - 不修改任务调度 `tasks::start_all()`
  - 不引入新的全局 setup/loop 行为

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单文件局部编辑（删 3 行 + 加 ~10 行）
  - **Skills**: []

  **Parallelization**:
  - **Wave**: 3
  - **Blocks**: T13
  - **Blocked By**: T1, T7, T9, T11

  **References**:
  - `src/main.cpp` - 现有 init 序列（i2c → spi → tft → lvgl_port → ui_main/menu/events → sensors → fan → power → watchdog → tasks）
  - `ui/lof_power_system.h:37` - `void lof_power_system_init(const char * asset_path)`
  - T7/T9/T11 实现的桥接层 API

  **Acceptance Criteria**:
  - [ ] 不含 `ui_main::init`、`ui_menu::init`、`ui_events::init`
  - [ ] 含 `lof_power_system_init(NULL)`
  - [ ] 含 `ui_bridge::screen_manager_init`、`data_bridge_init`、`data_bridge_attach`、`input_bridge_attach_home`（**不含** `input_bridge_init`，已删除）
  - [ ] `lvgl_port::init` 与 `tft_driver::init` 调用仍存在
  - [ ] **`src/app/tasks.cpp` 中 `input_task` 调用 `ui_bridge::input_handle_key`，且不再调用 `ui_events::handle_key`**
  - [ ] **`src/app/tasks.cpp` 含 `#include "ui_bridge/input_bridge.h"`**

  **QA Scenarios**:

  ```
  Scenario: 旧 init 移除、新 init 接入、桥接 init 闭合
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path src/main.cpp -Pattern 'ui_main::init|ui_menu::init|ui_events::init' -Quiet
      2. 执行: Select-String -Path src/main.cpp -Pattern 'lof_power_system_init\(NULL\)' -Quiet
      3. 执行: (Select-String -Path src/main.cpp -Pattern 'ui_bridge::(screen_manager_init|data_bridge_init|data_bridge_attach|input_bridge_attach_home)' | Measure-Object -Line).Lines
      4. 执行: Select-String -Path src/main.cpp -Pattern 'lvgl_port::init|tft_driver::init' | Measure-Object -Line | Select-Object -ExpandProperty Lines
    Expected Result: 第 1 行 == $false；第 2 行 == $true；第 3 行 == 4；第 4 行 ≥ 2
    Evidence: .sisyphus/evidence/task-12-main.txt

  Scenario: tasks.cpp::input_task 接入新桥接
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path src/app/tasks.cpp -Pattern 'ui_bridge::input_handle_key' -Quiet
      2. 执行: Select-String -Path src/app/tasks.cpp -Pattern 'ui_events::handle_key' -Quiet
      3. 执行: Select-String -Path src/app/tasks.cpp -Pattern '#include "ui_bridge/input_bridge.h"' -Quiet
    Expected Result: 第 1 行 == $true；第 2 行 == $false；第 3 行 == $true
    Evidence: .sisyphus/evidence/task-12-tasks-cpp.txt
  ```

  **Commit**: NO

- [x] 13. 首次编译 + 抢救式 fix-loop

  **What to do**:
  - 执行 `pio run -e esp32dev 2>&1 | Tee-Object -FilePath .sisyphus/evidence/task-13-build.log`
  - 若失败，按错误类别处理：
    - `undefined reference to lv_xml_register_*`：检查 T6 stub 是否包含被引用的具体函数；如发现新 XML API 调用，扩展 stub
    - `lvgl/lvgl.h: No such file`：检查 T4 build_flags `-I` 是否覆盖 LVGL 源根目录；如 LVGL 在 `lib/lvgl/src/lvgl.h` 则需添加 `-I lib/lvgl`
    - `redefinition of LV_USE_XML`：检查 T5 是否正确 #undef 后重定义
    - `multiple definition of font_xxx`：检查 build_src_filter 是否重复包含 ui/fonts
    - `lv_obj_get_child_by_name not declared`：确认 LVGL 9.x 版本支持此 API；如不支持，回退到 `lv_obj_get_child(parent, idx)` 索引方式（需更新 T9/T11）
  - 每轮 fix 后重跑 `pio run -e esp32dev`
  - 成功后执行 `pio run -e esp32dev -t size 2>&1 | Tee-Object -FilePath .sisyphus/evidence/task-13-size.log`
  - 验证 ELF 符号：
    ```powershell
    $nm = "$env:USERPROFILE\.platformio\packages\toolchain-xtensa-esp32\bin\xtensa-esp32-elf-nm.exe"
    & $nm .pio/build/esp32dev/firmware.elf | Select-String 'lof_power_system_init|home_create|splash_create' | Tee-Object -FilePath .sisyphus/evidence/task-13-symbols.txt
    ```

  **Must NOT do**:
  - 不通过修改 `ui/*_gen.c` 来"绕过"链接错误（违反 ui/AGENTS.md）
  - 不通过启用 LV_USE_XML 来"避免" stub
  - 不通过删除 build_src_filter 项来"通过"编译
  - 不修改 sensors/fan/power API 签名以匹配 data_bridge 调用（如签名不匹配则修改 data_bridge 而非传感器）
  - 不超过 5 轮 fix 循环；如仍未通过则 SESSION HANDOFF 给 oracle

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 端到端编译 + 错误分类 + 系统性修复
  - **Skills**: [`systematic-debugging`]
    - `systematic-debugging`: 链接/编译错误排查需结构化方法

  **Parallelization**:
  - **Wave**: 3
  - **Blocks**: F1-F4
  - **Blocked By**: T2, T4, T5, T6, T12（即 Wave 1+2 全部完成）

  **References**:
  - `.sisyphus/evidence/task-*-*.txt` - 前 12 任务全部证据
  - PlatformIO docs: `pio run`, `pio run -t size`
  - GCC linker error 文档

  **Acceptance Criteria**:
  - [ ] `.sisyphus/evidence/task-13-build.log` 末尾含 `[SUCCESS]` 或 `Building .pio\build\esp32dev\firmware.bin` 行
  - [ ] `.sisyphus/evidence/task-13-build.log` 不含 `undefined reference`
  - [ ] `.sisyphus/evidence/task-13-build.log` 不含 `error:`（warnings 可接受）
  - [ ] `.sisyphus/evidence/task-13-size.log` Flash < 6MB、RAM < 200KB
  - [ ] `.sisyphus/evidence/task-13-symbols.txt` 含 3 行符号匹配
  - [ ] `.pio/build/esp32dev/firmware.elf` 存在

  **QA Scenarios**:

  ```
  Scenario: 编译成功
    Tool: Bash
    Steps:
      1. 执行: pio run -e esp32dev; $LASTEXITCODE
    Expected Result: 0
    Failure Indicators: 非 0 退出码
    Evidence: .sisyphus/evidence/task-13-build.log

  Scenario: 无未定义引用
    Tool: Bash
    Steps:
      1. 执行: Select-String -Path .sisyphus/evidence/task-13-build.log -Pattern 'undefined reference' -Quiet
    Expected Result: $false
    Evidence: .sisyphus/evidence/task-13-build.log

  Scenario: 大小预算
    Tool: Bash
    Steps:
      1. 执行 pio run -e esp32dev -t size
      2. 解析输出 Flash / RAM 使用量
    Expected Result: Flash < 6291456 (6MB) 且 RAM < 204800 (200KB)
    Failure Indicators: 超出预算
    Evidence: .sisyphus/evidence/task-13-size.log

  Scenario: 关键符号存在
    Tool: Bash
    Steps:
      1. 执行 nm firmware.elf | grep 关键符号
    Expected Result: 3 行匹配（lof_power_system_init, home_create, splash_create）
    Evidence: .sisyphus/evidence/task-13-symbols.txt
  ```

  **Commit**: YES（最终单提交，message 见 Commit Strategy 段）
  - Files: `platformio.ini`, `include/lv_conf.h`, `src/main.cpp`, `src/ui/_legacy/*`, `src/ui_bridge/*`
  - Pre-commit: `pio run -e esp32dev` 必须 0 退出

---

## Final Verification Wave (MANDATORY — after ALL implementation tasks)

> 4 review agents run in PARALLEL. ALL must APPROVE. Present consolidated results to user and get explicit "okay" before completing.
>
> **Do NOT auto-proceed after verification. Wait for user's explicit approval before marking work complete.**

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Read this plan end-to-end. For each "Must Have": verify implementation exists (read file, check symbol via `nm`). For each "Must NOT Have": grep codebase for forbidden patterns (LV_USE_XML=1, WiFi.h include, lib_deps changes, *_gen.c modifications via `git diff ui/`). Confirm `src/ui/_legacy/` 全部文件已 `#if 0` 包裹。检查 `.sisyphus/evidence/` 中所有 task evidence 文件存在。
  Output: `Must Have [N/N] | Must NOT Have [N/N] | Tasks [N/N] | VERDICT: APPROVE/REJECT`

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Review all NEW files in `src/ui_bridge/` 与改动后的 `src/main.cpp`、`platformio.ini`、`include/lv_conf.h`：检查 `as any` 等价模式、空 catch（C++ 中 try/catch 滥用）、`Serial.println` 调试代码遗留、commented-out 块、未使用 include、AI slop（无意义注释、过度抽象、generic 命名）。运行 `pio check -e esp32dev --skip-packages` 静态检查。
  Output: `Lint [PASS/FAIL] | Files [N clean/N issues] | AI slop [N findings] | VERDICT`

- [ ] F3. **Build Artifact QA** — `unspecified-high`
  执行：`pio run -e esp32dev` (期 0 退出码) + `pio run -e esp32dev -t size` (期 Flash<6MB, RAM<200KB) + `nm .pio/build/esp32dev/firmware.elf | Select-String 'lof_power_system_init|home_create|splash_create'` (期 3 个符号都在) + `Select-String 'undefined reference' .sisyphus/evidence/task-13-*.log` (期无匹配)。所有结果保存 `.sisyphus/evidence/final-qa/`。
  Output: `Build [PASS/FAIL] | Size [Flash X / RAM Y] | Symbols [3/3] | VERDICT`

- [ ] F4. **Scope Fidelity Check** — `deep`
  对每个任务: 读"What to do"，读 git diff 该任务涉及文件。验证 1:1 — 计划的全部建了，计划外的没建。检查"Must NOT do"合规：`git diff ui/` 应为空（生成文件未改）；`git diff src/sensors/ src/fan/ src/power/` 应为空；`git status` 不应有 README*.md 新增；`platformio.ini` 的 `lib_deps` 应未变。
  Output: `Tasks [N/N compliant] | Generated files [CLEAN/dirty] | API boundaries [CLEAN/violated] | Unaccounted [CLEAN/N files] | VERDICT`

---

## Commit Strategy

**单提交策略**：所有改动作为 1 个原子提交，message 遵循 chinese-commit-conventions：

```
feat(ui): 集成 LVGL Editor 导出的 ui/ 子工程 (splash + home)

- 归档旧 src/ui/ 至 _legacy/ 并禁用编译
- platformio.ini 接入 ui/ 源与 include 路径
- 新增 src/ui_bridge/ 桥接层 (screen_manager + data_bridge + input_bridge)
- 提供 lv_xml_register_font weak stub 以避免链接错误
- main.cpp 改用 lof_power_system_init() 替代旧 ui::init()

里程碑3：实时温度/转速/3路电流显示 + KEY_UP/DOWN/ENTER 导航
仅编译验证，未做硬件烧录测试
```

**Pre-commit verification**: `pio run -e esp32dev && pio run -e esp32dev -t size`

---

## Success Criteria

### Verification Commands

```powershell
# 编译验证
pio run -e esp32dev
# 期: exit 0, 无 error/undefined reference

# 大小验证
pio run -e esp32dev -t size
# 期: Flash < 6MB (实际预算 8MB 留 2MB), RAM < 200KB

# 符号验证
& "$env:USERPROFILE\.platformio\packages\toolchain-xtensa-esp32\bin\xtensa-esp32-elf-nm.exe" .pio/build/esp32dev/firmware.elf | Select-String 'lof_power_system_init|home_create|splash_create'
# 期: 3 行匹配

# 边界验证（所有应为空）
git diff -- ui/
git diff -- src/sensors/ src/fan/ src/power/
git diff platformio.ini | Select-String 'lib_deps'

# Legacy 隔离验证
Get-Content src/ui/_legacy/ui_main.cpp | Select-Object -First 1
Get-Content src/ui/_legacy/ui_main.cpp | Select-Object -Last 1
# 期: 第一行 "#if 0"，最后一行 "#endif"
```

### Final Checklist
- [ ] 所有"Must Have"项实现并通过验证命令
- [ ] 所有"Must NOT Have"项检查为空（git diff 各域）
- [ ] `pio run -e esp32dev` 成功
- [ ] `pio run -t size` 在预算内
- [ ] 3 个关键符号在 firmware.elf 中存在
- [ ] F1-F4 全部 APPROVE
- [ ] 用户明确 okay
