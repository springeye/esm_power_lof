# Redesign Home Screen Layout

## TL;DR

> **Quick Summary**: 依照 `power_monitor_mockup.html` 效果图重构 `home.xml` 屏幕布局，修复 CH3 绑定 bug，扩展字体支持中文标签，添加风扇转速显示，增加停止状态运行时支持，合理使用 lv_obj 小图标。
> 
> **Deliverables**:
> - 扩展后的 `ui/globals.xml`（新增字体 symbols + fan_rpm_txt 变量）
> - 修改后的 `src/ui_bridge/data_bridge.cpp`（风扇 RPM 刷新 + 三状态映射）
> - 重构后的 `ui/screens/home.xml`
> - 重新生成的 `home_gen.c` / `home_gen.h`
> 
> **Estimated Effort**: Medium（5 个实现任务 + 2 个审核任务）
> **Parallel Execution**: YES - Wave 1 有 3 个并行任务
> **Critical Path**: Task 1/2/3 → Task 4 → Task 5 → F1/F2

---

## Context

### Original Request
用户要求参考 `ui/power_monitor_mockup.html` 效果图修改 `ui/screens/home.xml`，使 Home 屏幕布局与效果图一致。后追加需求：增加风扇 RPM 转速显示，UI 中合理使用小图标。

### Interview Summary
**Key Discussions**:
- 效果图历经多轮迭代：上下→三列横排→填满底部→两状态（运行中/停止）
- 用户明确两个状态映射：运行中(PSU On, 绿色指示) 和 停止(PSU Off, 红色指示)
- 通道区域从上下三行改为左中右三列卡片
- 风扇显示：百分比 + RPM
- 小图标：lv_obj 原生样式 + 扩展字体 symbols 两种方式并用
- 运行状态：添加"停止"状态，放开 data_bridge 修改限制

**Research Findings**:
- 现有 `home.xml` 有 CH3 绑定错误（`ch2_*` 替代 `ch3_*`）
- `globals.xml` 字体 symbols 仅包含：`电源功耗运行面板系统温度电压电流功率就绪运行中停止通道℃：`
- 需要新增字符：`当前占比时间总能效输出风扇转故障` — 共 15 个字
- `data_bridge.cpp` 仅输出 "运行中" 或 "故障"，需添加 "停止" 状态
- `app_state.h` 有 `fan_rpm`（atomic<uint32_t>）、`fan_duty`、`psu_state_id` 可用
- `◉` (U+25C9)、`⚡` (U+26A1) 不在字体范围，用 lv_obj 原生样式替代

### Metis Review
**Identified Gaps** (addressed):
- 字体 symbols 缺少中文标签字符 → 新增 Task 1 扩展 globals.xml
- 运行状态动态映射未实现 → 新增 Task 2 修改 data_bridge.cpp
- 效率百分比绑定变量暂用 `device_power_percent_txt`，后续需 data_bridge 扩展
- 状态色彩联动（绿/红）由 data_bridge 通过 `psu_state_id` 控制
- `style_flex_grow` 在 LVGL 嵌套 flex 中可能不生效 → 备选：显式百分比高度

### Momus Review (Round 2)
**Identified Gaps** (addressed):
- 字体字形缺失导致标签无法渲染 → Task 1 扩展 symbols
- data_bridge 禁改导致"停止"状态无法出现 → Task 2 添加状态逻辑

---

## Work Objectives

### Core Objective
依照效果图重构 `home.xml`，扩展字体和绑定变量支持，使 Home 屏幕在 240×280 分辨率下呈现：状态栏→功率主区→进度条+风扇→统计行→三列通道卡片，并支持运行中/停止两种状态和风扇转速显示。

### Concrete Deliverables
- `ui/globals.xml` — 扩展字体 symbols，新增 `fan_rpm_txt` 绑定变量
- `src/ui_bridge/data_bridge.cpp` — 添加风扇 RPM 刷新逻辑 + 三状态映射
- `ui/screens/home.xml` — 重构后布局文件
- `ui/screens/home_gen.c` / `ui/screens/home_gen.h` — 重新生成

### Definition of Done
- [ ] `home.xml` 布局与效果图视觉一致
- [ ] CH3 绑定使用 `ch3_voltage` / `ch3_current` / `ch3_pwer`
- [ ] LVGL Editor 可正常重新生成 `home_gen.c/h`
- [ ] `screen_manager.cpp` 中 `home_create()` 调用兼容
- [ ] 字体包含所有中文标签字符
- [ ] 风扇转速 (RPM + 百分比) 在 UI 上显示
- [ ] 运行状态可显示 运行中/停止/故障

### Must Have
- 状态栏（lv_obj 状态圆点 + system_state 文本 | 运行时间 + 温度）
- 功率大字区（`device_current_power`）
- 进度条区域（功率占比标签 + 百分比 + `lv_bar`）
- 风扇转速行（`fan_percent` + `fan_rpm_txt`）
- 四列统计行（运行时间/总电能/效率/风扇转速，或三列+独立风扇行）
- 三列通道卡片（CH1/CH2/CH3，含电压电流行+功率大字）
- CH3 绑定修复
- 字体 symbols 扩展（当前占比时间总能效输出风扇转故障）
- data_bridge 三状态映射（On→运行中/Off→停止/Fault→故障）

### Must NOT Have (Guardrails)
- ❌ 不引入自定义字体源文件（可扩展现有 `globals.xml` 的 symbols 列表并重新生成字体数据）
- ❌ 不使用字体不支持的特殊 Unicode 字符：`◉` (U+25C9)、`⚡` (U+26A1)、`●` (U+25CF) 等。用 LVGL 原生 `lv_obj` 带圆角和背景色实现视觉指示器
- ❌ 不手改 `*_gen.c` / `*_gen.h`（必须通过 LVGL Editor 重新生成）
- ❌ 不修改 `screen_manager.cpp` 中 `home_create()` 的调用方式

### LVGL Editor Workflow (重要)

> **当 agent 修改了 LVGL Editor 的 XML 文件（`globals.xml`、`home.xml` 等）之后，必须暂停并提醒用户在 LVGL Editor 中打开项目、重新生成对应的 C 代码文件。** 用户完成生成后，agent 再继续后续的开发工作（如编译验证、数据桥接等）。
>
> 这是因为：
> - `*_gen.c` / `*_gen.h` 必须由 LVGL Editor 工具生成，不能手改
> - `ui/fonts/*_data.c` 字体数据也必须由 LVGL Editor 重新生成
> - `lof_power_system_gen.c/h` 中包含全局 subject 声明（如 `fan_rpm_txt`），修改 `globals.xml` 后也需重新生成
> - 编译验证必须在生成文件完成之后才能执行
>
> 需要用户在 LVGL Editor 中重新生成的时机：
> 1. Task 1 完成后（globals.xml 修改）→ 用户重新生成字体数据和 global subjects
> 2. Task 4 完成后（home.xml 修改）→ 用户重新生成 home_gen.c/h

---

## Verification Strategy

> **ZERO HUMAN INTERVENTION** — ALL verification is agent-executed.

### Test Decision
- **Infrastructure exists**: NO（UI 屏幕无自动化测试）
- **Automated tests**: None
- **Agent-Executed QA**: Visual + structural + compilation verification

### QA Policy
- **XML Structural**: Agent reads `home.xml` and verifies each section exists with correct bindings
- **Font Coverage**: Agent verifies all Chinese characters in `home.xml` are in `globals.xml` font symbols
- **Data Bridge Logic**: Agent verifies `data_bridge.cpp` three-state mapping and fan RPM refresh
- **Compilation**: Agent runs `pio run -e esp32s3` and `pio run -e native-smoke` to check builds

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Start Immediately — foundation, MAX PARALLEL):
├── Task 1: Expand globals.xml font symbols + add fan_rpm_txt [quick]
├── Task 2: Update data_bridge.cpp for fan RPM + three states [quick]
└── Task 3: Regenerate font data from updated globals.xml [quick]

Wave 2 (After Wave 1 — core layout, depends: 1,2,3):
└── Task 4: Rewrite home.xml top-to-bottom [deep]

Wave 3 (After Task 4 — regeneration + verification):
└── Task 5: Regenerate home_gen + build verification [quick]

Wave FINAL (After ALL tasks — parallel reviews):
├── Task F1: Plan compliance audit [oracle]
└── Task F2: Code quality review [quick]
```

### Dependency Matrix

| Task | Depends On | Blocks |
|------|-----------|--------|
| 1 | — | 3, 4 |
| 2 | — | 4, 5 |
| 3 | 1 | 4 |
| 4 | 1, 2, 3 | 5 |
| 5 | 4 | F1, F2 |
| F1 | 5 | — |
| F2 | 5 | — |

### Agent Dispatch Summary

- **Wave 1**: 3 tasks — T1 → `quick`, T2 → `quick`, T3 → `quick`
- **Wave 2**: 1 task — T4 → `deep`
- **Wave 3**: 1 task — T5 → `quick`
- **FINAL**: 2 tasks — F1 → `oracle`, F2 → `quick`

---

## TODOs

- [ ] 1. Expand globals.xml font symbols and add fan_rpm_txt variable

  **What to do**:
  - Edit `ui/globals.xml`:
  - In each `<bin>` font tag's `symbols` attribute, append the missing characters needed by the new home.xml labels
  - Current symbols: `电源功耗运行面板系统温度电压电流功率就绪运行中停止通道℃：`
  - Characters to ADD: `当前占比时间总能效输出风扇转故障` (15 chars)
  - Final symbols for all three fonts: `电源功耗运行面板系统温度电压电流功率就绪运行中停止通道℃：当前占比时间总能效输出风扇转故障`
  - Add new binding variable: `<string name="fan_rpm_txt" value="0 RPM" help="风扇转速" />` in the subjects section
  - Verify the string is well-formed XML

  **Must NOT do**:
  - Do NOT change font src_path, size, bpp, or range attributes — only modify `symbols`
  - Do NOT remove any existing characters from the symbols string

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 2, 3)
  - **Blocks**: Tasks 3, 4
  - **Blocked By**: None

  **References**:

  **Pattern References**:
  - `ui/globals.xml:58-84` — Current font definitions with `symbols` attribute and subjects section
  - `ui/fonts/README.md` — Font regeneration documentation

  **API/Type References**:
  - `ui/globals.xml` — Binding variable names and types (string, int)

  **Why Each Reference Matters**:
  - `globals.xml`: Contains the exact `symbols` strings that must be expanded and the location for adding `fan_rpm_txt`
  - Current characters in symbols are the ONLY ones renderable — all new label characters MUST be added

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY)**:

  ```
  Scenario: Font symbols contain all required characters
    Tool: Bash (grep/python)
    Preconditions: globals.xml modified
    Steps:
      1. Run: python -c "data=open('ui/globals.xml','r',encoding='utf-8').read(); required='当前占比时间总能效输出风扇转故障'; missing=[c for c in required if c not in data]; print(f'Missing: {missing}' if missing else 'All chars present')"
      2. Verify output shows "All chars present"
    Expected Result: All 15 new characters are present in globals.xml
    Failure Indicators: Any characters listed as missing
    Evidence: .sisyphus/evidence/task-1-font-symbols.txt

  Scenario: fan_rpm_txt binding variable added
    Tool: Bash (grep)
    Preconditions: globals.xml modified
    Steps:
      1. Run: grep "fan_rpm_txt" ui/globals.xml
      2. Verify output shows the binding variable declaration
    Expected Result: fan_rpm_txt variable found in globals.xml
    Failure Indicators: Variable not found
    Evidence: .sisyphus/evidence/task-1-fan-binding.txt
  ```

  **Commit**: YES
  - Message: `feat(ui): expand font symbols and add fan_rpm_txt binding`
  - Files: `ui/globals.xml`

- [ ] 2. Update data_bridge.cpp for fan RPM and three-state mapping

  **What to do**:
  - Edit `src/ui_bridge/data_bridge.cpp`:
  - **Fan RPM**: After the `fan_percent` update block (line ~128-136), add a new block that reads `app_state::get_rpm()`, formats it as `"<N> RPM"` (e.g., "900 RPM" or "0 RPM"), and updates a `fan_rpm_txt` LVGL subject
  - Add `lv_subject_t* fan_rpm_txt_subject` declaration (or find the appropriate subject reference from the generated headers)
  - **Three-state mapping**: Modify the `system_state` update (line ~60-61) to check `psu_state_id`:
    ```
    uint8_t state = app_state::get_psu_state();
    const char* state_text;
    if (app_state::is_fault()) state_text = "故障";
    else if (state == PSU_ON) state_text = "运行中";
    else state_text = "停止";
    lv_subject_copy_string(&system_state, state_text);
    ```
  - Include `app_state.h` and `psu_fsm.h` for `PsuState` enum if not already included
  - Verify `fan_rpm_txt` subject is accessible (it's declared in `lof_power_system_gen.h` or needs to be added to globals.xml binding)

  **Must NOT do**:
  - Do NOT change any function signatures
  - Do NOT modify screen_manager.cpp

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 3)
  - **Blocks**: Task 4
  - **Blocked By**: None

  **References**:

  **Pattern References**:
  - `src/ui_bridge/data_bridge.cpp:52-137` — Complete refresh_cb function showing the pattern for all UI updates
  - `src/ui_bridge/data_bridge.cpp:128-136` — Existing fan_percent update block (pattern to follow for fan_rpm_txt)
  - `src/ui_bridge/data_bridge.cpp:59-61` — Current system_state update (two-state, needs expansion)

  **API/Type References**:
  - `src/app/app_state.h:37-38` — `fan_rpm` and `fan_duty` atomic variables
  - `src/app/app_state.h:45` — `psu_state_id` atomic variable
  - `src/app/app_state.h:50-51` — `get_rpm()` and `get_duty()` inline functions
  - `src/power/psu_fsm.h` — `PsuState` enum (PSU_OFF, PSU_STANDBY, PSU_STARTING, PSU_ON, PSU_STOPPING, PSU_FAULT)

  **Why Each Reference Matters**:
  - `data_bridge.cpp` refresh_cb pattern: Must follow existing style for LVGL subject updates
  - `app_state.h`: Source of truth for available atomic variables (fan_rpm, psu_state_id)
  - `psu_fsm.h`: PsuState enum values for three-state mapping logic

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY)**:

  ```
  Scenario: Three-state mapping implemented
    Tool: Bash (grep)
    Preconditions: data_bridge.cpp modified
    Steps:
      1. Run: grep -n "psu_state\|PSU_ON\|PSU_OFF\|停止" src/ui_bridge/data_bridge.cpp
      2. Verify PSU_ON, "停止", "故障" state mapping logic exists
    Expected Result: Lines showing three-state logic with PSU_ON→运行中, Off→停止, Fault→故障
    Failure Indicators: Only two-state logic remains (运行中/故障)
    Evidence: .sisyphus/evidence/task-2-three-state.txt

  Scenario: Fan RPM update added
    Tool: Bash (grep)
    Preconditions: data_bridge.cpp modified
    Steps:
      1. Run: grep -n "fan_rpm_txt\|get_rpm" src/ui_bridge/data_bridge.cpp
      2. Verify fan RPM formatting and subject update are present
    Expected Result: fan_rpm_txt subject update with RPM value found
    Failure Indicators: No fan_rpm_txt reference found
    Evidence: .sisyphus/evidence/task-2-fan-rpm.txt
  ```

  **Commit**: YES
  - Message: `feat(ui): add fan RPM display and three-state system_state mapping`
  - Files: `src/ui_bridge/data_bridge.cpp`

- [ ] 3. Regenerate font data from updated globals.xml

  **What to do**:
  - ⚠️ **PAUSE: Task 1 must be completed first. After editing globals.xml, remind the user to open LVGL Editor and regenerate ALL generated files (fonts + global subjects). Do NOT proceed with compilation until user confirms regeneration is done.**
  - Use LVGL Editor to regenerate font C data files from the updated `globals.xml`
  - The three font files that need regeneration:
    - `ui/fonts/hos_bold_big_data.c` (symbols expanded)
    - `ui/fonts/hos_regular_data.c` (symbols expanded)
    - `ui/fonts/hos_14_data.c` (symbols expanded)
  - If LVGL Editor is not available, this becomes a manual step to be completed before build verification
  - After regeneration, verify that the font data arrays contain glyph entries for the new characters

  **Must NOT do**:
  - Do NOT hand-modify the font C data files — they must be regenerated by the tool
  - Do NOT change the font source TTF files

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: YES (after Task 1)
  - **Parallel Group**: Wave 1 (with Tasks 1, 2 — but depends on Task 1 completion)
  - **Blocks**: Task 4
  - **Blocked By**: Task 1

  **References**:

  **Pattern References**:
  - `ui/fonts/hos_bold_big_data.c` — Current bold big font data (to be regenerated)
  - `ui/fonts/hos_regular_data.c` — Current regular font data (to be regenerated)
  - `ui/fonts/hos_14_data.c` — Current 14px font data (to be regenerated)
  - `ui/fonts/README.md` — Font regeneration documentation if available

  **Why Each Reference Matters**:
  - These are the exact files that must be regenerated from the updated globals.xml symbols
  - Must verify the new characters appear in the generated glyph data

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY)**:

  ```
  Scenario: Font data files contain new glyphs
    Tool: Bash (python/grep)
    Preconditions: Font files regenerated
    Steps:
      1. Run: python -c "import re; data=open('ui/fonts/hos_regular_data.c','r',encoding='utf-8').read(); required='当前占比时间总能效输出风扇转故障'; missing=[c for c in required if c not in data and ord(c)>0x7F]; print(f'Missing from font: {missing}' if missing else 'All chars found in font data')"
      2. Run same check for hos_bold_big_data.c
    Expected Result: All new characters found in font glyph data
    Failure Indicators: Any characters missing from font data
    Evidence: .sisyphus/evidence/task-3-font-glyphs.txt

  Scenario: Font file sizes increased
    Tool: Bash
    Preconditions: Font files regenerated
    Steps:
      1. Check that font data files are larger than originals (new characters = more glyph data)
      2. Run: ls -la ui/fonts/hos_*_data.c
    Expected Result: All three font data files have been updated with larger sizes
    Failure Indicators: Font files unchanged or shrunk
    Evidence: .sisyphus/evidence/task-3-font-sizes.txt
  ```

  **Commit**: YES
  - Message: `feat(ui): regenerate font data with expanded symbol set`
  - Files: `ui/fonts/hos_bold_big_data.c`, `ui/fonts/hos_regular_data.c`, `ui/fonts/hos_14_data.c`

- [ ] 4. Rewrite home.xml layout top-to-bottom

  **What to do**:
  - Rewrite `ui/screens/home.xml` from scratch following the mockup design
  - **Status bar (~8%)**: flex row — left: small `lv_obj` (width=8, height=8, radius=4, bg_color=0x00e68a for running / 0xff4444 for stopped — NOTE: dynamic color via data_bridge later) as status indicator dot + `system_state` label. Right: `uptime` label + `device_temp` label. Background `0x111820`, separator bottom border
  - **Power hero section (~18%)**: Centered flex column — top row: "当前功率" label + "750W" static badge (lv_obj with colored bg), main row: `device_current_power` bound label with `hos_bold_big` font
  - **Progress + fan section (~10%)**: Two rows — Row 1: "功率占比" label + `device_power_percent_txt` label + `lv_bar` bound to `device_power_percent`. Row 2: "风扇" small icon (lv_obj paddle/propeller shape or small label) + `fan_percent` label + `fan_rpm_txt` label
  - **Stats row (~10%)**: Three equal columns — 运行时间 / 总电能 / 效率, each in bordered box with label above value
  - **Divider**: 1px horizontal line, gradient bg `0x2a3a4a`
  - **Channel section (flex_grow=1)**: "输出通道" header + flex row of 3 equal channel cards. Each card: CH tag header (colored bottom border: CH1=0x00e68a, CH2=0xffb020, CH3=0x3d9eff), then voltage/current rows (label left, value right), then power value centered
  - **Icon strategy**: Use `lv_obj` with radius and bg_color for visual indicators (status dot, colored badges, separator). No Unicode special characters — only characters in the font symbols list
  - CH3 card MUST bind `ch3_voltage`, `ch3_current`, `ch3_pwer` (NOT `ch2_*`)
  - Overall bg_color `0x0a0e14`, all text uses `hos_regular`, `hos_bold_big`, or `hos_14` fonts
  - Retain existing `style_main` and `no_padding` style definitions
  - Use `system_state` binding for status text

  **Must NOT do**:
  - Do NOT use Unicode characters outside the font's supported range: no `◉` (U+25C9), `⚡` (U+26A1), `●` (U+25CF). Use LVGL `lv_obj` with `radius` and `bg_color` for visual indicators instead
  - Do NOT hand-edit `*_gen.c` / `*_gen.h` files
  - Do NOT modify `data_bridge.cpp` (done in Task 2) or `screen_manager.cpp`

  **Recommended Agent Profile**:
  - **Category**: `deep`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 2 (sole task)
  - **Blocks**: Tasks 5, F1, F2
  - **Blocked By**: Tasks 1, 2, 3

  **References**:

  **Pattern References**:
  - `ui/screens/home.xml` — Current XML to be replaced (full file, 370 lines)
  - `ui/power_monitor_mockup.html` — Mockup reference for visual layout design

  **API/Type References**:
  - `ui/screens/splash.xml` — Another screen XML for LVGL Editor XML syntax reference
  - `ui/globals.xml` — Global variable bindings (now includes `fan_rpm_txt` from Task 1)
  - `src/ui_bridge/data_bridge.cpp:59-61` — Three-state system_state mapping (updated in Task 2)
  - `src/ui_bridge/data_bridge.cpp:128-136` — fan_percent and fan_rpm_txt update pattern (from Task 2)

  **Why Each Reference Matters**:
  - `home.xml`: The exact file being rewritten; must understand current bindings and structure
  - `power_monitor_mockup.html`: Target visual design — CSS layout maps to LVGL flex properties
  - `splash.xml`: Demonstrates valid LVGL Editor XML syntax patterns
  - `globals.xml`: Lists available binding variable names including new `fan_rpm_txt`
  - `data_bridge.cpp`: Shows the three-state mapping and fan data format for correct label binding

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY)**:

  ```
  Scenario: XML structure matches mockup sections
    Tool: Bash (grep)
    Preconditions: home.xml exists
    Steps:
      1. grep for "system_state" in home.xml → found
      2. grep for "device_current_power" in home.xml → found
      3. grep for "lv_bar" in home.xml → found
      4. grep for "fan_percent" in home.xml → found
      5. grep for "fan_rpm_txt" in home.xml → found
      6. grep for "ch3_voltage" in home.xml → found
      7. grep for "ch3_current" in home.xml → found
      8. grep for "ch3_pwer" in home.xml → found
      9. grep for "CH1\|CH2\|CH3" in home.xml → all 3 found
    Expected Result: All bindings and labels present including fan and 3 states
    Failure Indicators: Any grep returns 0 matches
    Evidence: .sisyphus/evidence/task-4-xml-structure.txt

  Scenario: CH3 binding correctness
    Tool: Bash (grep)
    Preconditions: home.xml exists
    Steps:
      1. Run: grep -n "ch2_voltage\|ch2_current\|ch2_pwer" ui/screens/home.xml
      2. Verify no ch2_* references appear after "CH3" or "通道3" line
    Expected Result: Zero references to ch2_* in CH3 card portion
    Failure Indicators: Any ch2_* found in last channel card
    Evidence: .sisyphus/evidence/task-4-ch3-binding.txt

  Scenario: No unsupported Unicode characters
    Tool: Bash (python)
    Preconditions: home.xml exists
    Steps:
      1. Run: python -c "data=open('ui/screens/home.xml','r',encoding='utf-8').read(); required='电源功耗运行面板系统温度电压电流功率就绪运行中停止通道℃：当前占比时间总能效输出风扇转故障'; bad=[c for c in data if ord(c)>0x7F and c not in required]; print(f'Unsupported: {bad}' if bad else 'All chars supported')"
      2. Verify output shows "All chars supported"
    Expected Result: Zero unsupported Unicode characters
    Failure Indicators: Any characters listed as unsupported
    Evidence: .sisyphus/evidence/task-4-unicode-check.txt
  ```

  **Commit**: YES (group with Task 5)
  - Message: `refactor(ui): redesign home screen layout with fan display and icons`
  - Files: `ui/screens/home.xml`

- [ ] 5. Regenerate home_gen, verify signature, and build

  **What to do**:
  - ⚠️ **PAUSE: Task 4 must be completed first. After editing home.xml, remind the user to open LVGL Editor and regenerate home_gen.c/h. Do NOT proceed with build verification until user confirms regeneration is done.**
  - Use LVGL Editor to regenerate `ui/screens/home_gen.c` and `ui/screens/home_gen.h` from the updated `home.xml`
  - If LVGL Editor is not available, verify the XML is syntactically correct and document as a manual step
  - Verify `home_create()` function signature is unchanged in `home_gen.h`
  - Verify `screen_manager.cpp` (line 5: `extern "C" lv_obj_t* home_create(void);`) and call at line 26 remain compatible
  - Run `pio run -e esp32s3` to verify the project compiles
  - Run `pio run -e native-smoke` to verify native build compiles

  **Must NOT do**:
  - Do NOT hand-edit `home_gen.c` or `home_gen.h`
  - Do NOT modify C++ source to fix build errors introduced by wrong XML changes

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 3 (sole task)
  - **Blocks**: F1, F2
  - **Blocked By**: Task 4

  **References**:

  **Pattern References**:
  - `ui/screens/home_gen.h` — Current generated header to compare function signature
  - `platformio.ini` — Build environment configurations

  **API/Type References**:
  - `src/ui_bridge/screen_manager.cpp:5` — `extern "C" lv_obj_t* home_create(void);` declaration
  - `src/ui_bridge/screen_manager.cpp:26` — `g_home = home_create();` call site

  **Why Each Reference Matters**:
  - `home_gen.h`: Must verify `home_create()` signature matches what `screen_manager.cpp` expects
  - `platformio.ini`: Needs correct build environment names

  **Acceptance Criteria**:

  **QA Scenarios (MANDATORY)**:

  ```
  Scenario: Function signature preserved
    Tool: Bash (grep)
    Steps:
      1. Run: grep -n "home_create" ui/screens/home_gen.h — verify declaration exists as `lv_obj_t* home_create(void);`
      2. Run: grep -n "home_create" src/ui_bridge/screen_manager.cpp — verify call signature matches
      3. Compare the two signatures — they must be identical (both void parameter, both return lv_obj_t*)
    Expected Result: home_gen.h declares `lv_obj_t* home_create(void);` and screen_manager.cpp:5 declares `extern "C" lv_obj_t* home_create(void);`
    Failure Indicators: home_create signature changed (e.g. added parameters), or declaration missing
    Evidence: .sisyphus/evidence/task-5-signature.txt

  Scenario: ESP32-S3 firmware compiles
    Tool: Bash
    Steps:
      1. Run: pio run -e esp32s3
      2. Check exit code is 0
    Expected Result: Build succeeds with no UI-related errors
    Failure Indicators: Compilation errors in home_gen.c/h, data_bridge references, or font data
    Evidence: .sisyphus/evidence/task-5-build-esp32s3.txt

  Scenario: Native smoke test compiles
    Tool: Bash
    Steps:
      1. Run: pio run -e native-smoke
      2. Check exit code is 0
    Expected Result: Build succeeds
    Failure Indicators: Compilation errors
    Evidence: .sisyphus/evidence/task-5-build-native.txt

  Scenario: XML syntax and bindings validation
    Tool: Bash (grep)
    Steps:
      1. Run: grep -c "bind_text" ui/screens/home.xml — count all bind_text attributes
      2. For each bind_text value, verify it exists in ui/globals.xml subjects section
      3. Run: grep "ch3_" ui/screens/home.xml — verify ch3_voltage, ch3_current, ch3_pwer all present
      4. Run: grep -n "ch2_voltage\|ch2_current\|ch2_pwer" ui/screens/home.xml — verify these appear ONLY in CH2 section
    Expected Result: All bind_text values match globals.xml subjects; CH3 section uses ch3_* only
    Failure Indicators: Unknown binding name or ch2_* in CH3 card section
    Evidence: .sisyphus/evidence/task-5-xml-validation.txt
  ```

  **Commit**: YES (group with Task 4)
  - Message: `refactor(ui): redesign home screen layout with fan display and icons`
  - Files: `ui/screens/home.xml`, `ui/screens/home_gen.c`, `ui/screens/home_gen.h`

---

## Final Verification Wave

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Read the plan end-to-end. For each "Must Have" item, verify implementation exists:
  - Status bar with lv_obj indicator: `grep -c "system_state" ui/screens/home.xml` → ≥1
  - Power hero: `grep -c "device_current_power" ui/screens/home.xml` → ≥1
  - Progress bar: `grep -c "lv_bar" ui/screens/home.xml` → ≥1
  - Fan display: `grep "fan_percent\|fan_rpm_txt" ui/screens/home.xml` → both found
  - Stats row: `grep -c "uptime\|wh\|device_power_percent_txt" ui/screens/home.xml` → ≥3 total
  - Three channel cards: `grep -c "CH[123]" ui/screens/home.xml` → 3
  - CH3 bindings: `grep "ch3_voltage\|ch3_current\|ch3_pwer" ui/screens/home.xml` → all 3 present
  - Font symbols: verify all 15 new characters in `ui/globals.xml` symbols attributes
  - Three-state mapping: `grep -c "PSU_ON\|停止\|故障" src/ui_bridge/data_bridge.cpp` → ≥3
  - Fan RPM: `grep "fan_rpm_txt" src/ui_bridge/data_bridge.cpp` → found

  For each "Must NOT Have" item, search for forbidden patterns:
  - `grep "◉\|⚡\|●" ui/screens/home.xml` → 0 results
  - `python -c "..."` Unicode check as defined in Task 4 QA
  - No hand-edited `*_gen.c/h` (check timestamps match regeneration)
  - No data_bridge changes beyond fan RPM and three-state logic

  Output: `Must Have [N/N] | Must NOT Have [N/N] | Tasks [N/N] | VERDICT: APPROVE/REJECT`

- [ ] F2. **Code Quality Review** — `quick`
  Run: `pio run -e esp32s3` and verify build succeeds.
  Then review all changed files:
  - `ui/globals.xml`: symbols strings identical across all 3 fonts, fan_rpm_txt variable added
  - `src/ui_bridge/data_bridge.cpp`: three-state mapping logic correct, fan RPM formatting present
  - `ui/screens/home.xml`: consistent 2-space indentation, all bind_text values in globals.xml, only `hos_regular`/`hos_bold_big`/`hos_14` font references, no unsupported Unicode
  - Verify CH3 section uses `ch3_*` bindings exclusively
  - Run: `python -c "data=open('ui/screens/home.xml','r',encoding='utf-8').read(); required='电源功耗运行面板系统温度电压电流功率就绪运行中停止通道℃：当前占比时间总能效输出风扇转故障'; bad=[c for c in data if ord(c)>0x7F and c not in required]; print('Unsupported:', bad if bad else 'None')"`
  Output: `Build [PASS/FAIL] | CH3 Bindings [CORRECT/WRONG] | Fonts [VALID/INVALID] | Three-State [YES/NO] | Fan RPM [YES/NO] | VERDICT: APPROVE/REJECT`

---

## Commit Strategy

- **1**: `feat(ui): expand font symbols and add fan_rpm_txt binding` — ui/globals.xml
- **2**: `feat(ui): add fan RPM display and three-state system_state mapping` — src/ui_bridge/data_bridge.cpp
- **3**: `feat(ui): regenerate font data with expanded symbol set` — ui/fonts/hos_*_data.c
- **4+5**: `refactor(ui): redesign home screen layout with fan display and icons` — ui/screens/home.xml, ui/screens/home_gen.c, ui/screens/home_gen.h

---

## Success Criteria

### Verification Commands
```bash
# Structural checks
grep -c "system_state" ui/screens/home.xml        # Expected: ≥1
grep -c "device_current_power" ui/screens/home.xml # Expected: ≥1
grep -c "fan_percent" ui/screens/home.xml          # Expected: ≥1
grep -c "fan_rpm_txt" ui/screens/home.xml          # Expected: ≥1
grep -c "ch3_voltage" ui/screens/home.xml          # Expected: 1
grep -c "ch3_current" ui/screens/home.xml          # Expected: 1
grep -c "ch3_pwer" ui/screens/home.xml             # Expected: 1

# Font coverage check
python -c "data=open('ui/globals.xml','r',encoding='utf-8').read(); required='当前占比时间总能效输出风扇转故障'; missing=[c for c in required if c not in data]; print(f'Missing: {missing}' if missing else 'All chars present')"

# Build check
pio run -e esp32s3  # Expected: SUCCESS
pio run -e native-smoke  # Expected: SUCCESS
```

### Final Checklist
- [ ] All "Must Have" present in home.xml and data_bridge.cpp
- [ ] Font symbols include all required Chinese characters
- [ ] No `ch2_*` in CH3 card section
- [ ] No unsupported Unicode characters in home.xml
- [ ] No hand-edited `*_gen.c/h` files
- [ ] Three-state mapping working (运行中/停止/故障)
- [ ] Fan RPM display binding present
- [ ] ESP32-S3 and native-smoke builds succeed