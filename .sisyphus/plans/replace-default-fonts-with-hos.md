# Replace Default Fonts with HarmonyOS Sans (hos)

## TL;DR

> **Quick Summary**: 将LVGL默认字体从Montserrat替换为HarmonyOS Sans（hos），提升240x280屏幕上的显示质量，关闭未使用的Montserrat字体宏以减小固件体积。
> 
> **Deliverables**:
> - 修改`include/lv_conf.h`中的字体配置
> - 验证UI布局和显示效果
> - 确认编译通过和测试通过
> 
> **Estimated Effort**: Quick (1-2小时)
> **Parallel Execution**: NO - 顺序执行
> **Critical Path**: 修改lv_conf.h → 编译验证 → 显示效果验证

---

## Context

### Original Request
用户要求修改项目中的XML（LVGL Editor页面文件），把默认字体全部替换为hos字体。屏幕240x280，像素间距0.11655x0.11655，要求不改变显示效果（大小和布局）。

### Current State
- **默认字体**: `include/lv_conf.h` 第240行 `#define LV_FONT_DEFAULT &lv_font_montserrat_14`
- **已启用的Montserrat宏**:
  - `LV_FONT_MONTSERRAT_14 1` (第206行)
  - `LV_FONT_MONTSERRAT_16 1` (第207行)
  - `LV_FONT_MONTSERRAT_20 1` (第209行)
  - `LV_FONT_MONTSERRAT_28 1` (第213行)
- **hos字体文件**: `ui/fonts/` 包含HarmonyOS_Sans_SC_*.ttf和生成的*_data.c文件
- **XML文件**: `ui/screens/*.xml` 已使用hos_14、hos_regular等字体引用

### Research Findings
- hos_14字体数据已存在: `ui/fonts/hos_14_data.c`
- 字体绑定在: `ui/lof_power_system_gen.c` (hos_* = &hos_*_data)
- FontAwesome图标字体独立，不受影响
- 项目使用LVGL 9.x，支持抗锯齿字体（bpp=8）

---

## Work Objectives

### Core Objective
将LVGL默认字体从Montserrat替换为hos_14（HarmonyOS Sans SC Regular 12px），通过配置验证确保字体切换正确。

> **范围说明**: 本计划仅验证配置层面的字体切换（LV_FONT_DEFAULT、Montserrat宏、FontAwesome绑定），不包含视觉/布局自动化验证。UI渲染效果将在实际硬件上验证。

### Concrete Deliverables
- 修改`include/lv_conf.h`第240行: `#define LV_FONT_DEFAULT &hos_14_data`
- 添加`LV_FONT_CUSTOM_DECLARE`声明
- 关闭未使用的Montserrat字体宏（第206、207、209、213行设为0）
- 编译验证通过（native和esp32s3环境）
- 单元测试通过
- 字体配置一致性验证

### Definition of Done
- [ ] `pio run -e native` 编译成功
- [ ] `pio run -e esp32s3` 编译成功
- [ ] `pio test -e native` 所有测试通过
- [ ] 字体配置验证通过（LV_FONT_DEFAULT指向hos_14_data，Montserrat宏已关闭）
- [ ] FontAwesome字体绑定未被修改
- [ ] 固件体积减小或保持不变

> **注**: 用户明确要求忽略native模拟器相关的视觉验证。UI布局和字体渲染效果将在实际硬件上验证。

### Must Have
- LV_FONT_DEFAULT指向hos_14_data
- LV_FONT_CUSTOM_DECLARE已添加
- 关闭LV_FONT_MONTSERRAT_14/16/20/28
- 编译通过
- 测试通过
- FontAwesome字体绑定未被修改

### Must NOT Have (Guardrails)
- 不修改FontAwesome图标字体（font_awesome_14/48）
- 不改变现有UI布局或字体大小
- 不修改LVGL Editor生成的代码（*_gen.c/h）
- 不添加新的字体权重或样式

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** - ALL verification is agent-executed. No exceptions.

### Test Decision
- **Infrastructure exists**: YES (Unity框架)
- **Automated tests**: YES (Tests-after)
- **Framework**: Unity (`pio test -e native`)
- **Test strategy**: 修改后运行现有测试，确保无回归

### QA Policy
Every task MUST include agent-executed QA scenarios.
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

- **编译验证**: Use Bash (pio run) - 执行编译命令，检查退出码和输出
- **测试验证**: Use Bash (pio test) - 执行测试命令，检查测试结果
- **配置验证**: Use PowerShell (Select-String) - 检查配置文件和代码引用

---

## Execution Strategy

### Sequential Execution (单文件修改，顺序验证)

```
Step 0: 基线捕获
├── Task 0.1: 捕获固件大小基线
└── Commit: NO

Step 1: 修改 lv_conf.h 字体配置
├── Task 1.1: 修改 LV_FONT_DEFAULT 宏
├── Task 1.2: 关闭未使用的 Montserrat 宏
└── Commit: YES

Step 2: 编译验证
├── Task 2.1: 验证 Montserrat 引用
├── Task 2.2: 编译 native 环境
├── Task 2.3: 编译 esp32s3 环境
└── Commit: NO (验证步骤)

Step 3: 测试验证
├── Task 3.1: 运行单元测试
└── Commit: NO (验证步骤)

Step 4: 配置验证
├── Task 4.1: 验证字体配置一致性
└── Commit: NO (验证步骤)

Step 5: 固件体积验证
├── Task 5.1: 查看固件大小
├── Task 5.2: 对比固件体积变化
└── Commit: NO (验证步骤)

Step 6: 文档更新
├── Task 6.1: 更新 lv_conf.h 注释
├── Task 6.2: 更新 AGENTS.md
└── Commit: YES

Critical Path: Task 0.1 → Task 1.1 → Task 2.2 → Task 3.1 → Task 4.1
Parallel Speedup: N/A (顺序执行)
```

### Dependency Matrix

- **0.1**: - - 1.1
- **1.1, 1.2**: 0.1 - 2.1, 2.2, 2.3
- **2.1**: 1.1, 1.2 - 2.2, 2.3
- **2.2**: 2.1 - 3.1
- **2.3**: 2.1 - 3.1
- **3.1**: 2.2, 2.3 - 4.1
- **4.1**: 3.1 - 5.1
- **5.1**: 4.1 - 5.2
- **5.2**: 5.1 - 6.1
- **6.1**: 5.2 - 6.2
- **6.2**: 6.1 - -

---

## TODOs

> EVERY task MUST have QA Scenarios. A task WITHOUT QA Scenarios is INCOMPLETE.

- [x] 0.1 捕获固件大小基线

  **What to do**:
  - 运行 `pio run -e esp32s3 -t size` 获取修改前的固件大小
  - 将输出保存到 `.sisyphus/evidence/baseline-firmware-size.txt`
  - 记录关键数据（如RAM/ROM使用量）

  **Must NOT do**:
  - 不修改任何代码

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 1.1
  - **Blocked By**: None (can start immediately)

  **References**:

  **Pattern References**:
  - `platformio.ini` - 构建配置

  **WHY Each Reference Matters**:
  - 确认编译命令正确

  **Acceptance Criteria**:
  - [ ] 固件大小基线已保存到 `.sisyphus/evidence/baseline-firmware-size.txt`

  **QA Scenarios**:

  ```
  Scenario: 验证固件大小基线捕获
    Tool: PowerShell (pio)
    Preconditions: 项目根目录
    Steps:
      1. pio run -e esp32s3 -t size | Out-File .sisyphus/evidence/baseline-firmware-size.txt
    Expected Result: 固件大小数据已保存
    Failure Indicators: 命令执行失败或文件未创建
    Evidence: .sisyphus/evidence/baseline-firmware-size.txt
  ```

  **Commit**: NO

- [x] 1.1 修改 LV_FONT_DEFAULT 宏

  **What to do**:
  - 打开 `include/lv_conf.h`
  - 找到第234-237行的LV_FONT_CUSTOM_DECLARE区域
  - 添加自定义字体声明: `#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(hos_14_data)`
  - 找到第240行: `#define LV_FONT_DEFAULT &lv_font_montserrat_14`
  - 修改为: `#define LV_FONT_DEFAULT &hos_14_data`

  **Must NOT do**:
  - 不修改其他字体配置
  - 不修改FontAwesome相关配置

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 1.2, 2.1, 2.2, 2.3
  - **Blocked By**: 0.1

  **References**:

  **Pattern References**:
  - `include/lv_conf.h:234-237` - LV_FONT_CUSTOM_DECLARE区域（自定义字体声明入口）
  - `include/lv_conf.h:240` - 当前LV_FONT_DEFAULT定义

  **API/Type References**:
  - `ui/fonts/hos_14_data.c` - hos_14_data字体数据定义（const lv_font_t hos_14_data，文件末尾约第2140行）

  **WHY Each Reference Matters**:
  - 第234-237行是LV_FONT_CUSTOM_DECLARE的声明入口，需要在这里声明hos_14_data
  - 第240行是需要修改的目标行
  - hos_14_data.c定义了const lv_font_t hos_14_data，是编译时可见的字体数据

  **Acceptance Criteria**:
  - [ ] `include/lv_conf.h` 第237行后添加 `#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(hos_14_data)`
  - [ ] `include/lv_conf.h` 第240行已修改为 `#define LV_FONT_DEFAULT &hos_14_data`

  **QA Scenarios**:

  ```
  Scenario: 验证 LV_FONT_CUSTOM_DECLARE 和 LV_FONT_DEFAULT 修改
    Tool: PowerShell (Select-String)
    Preconditions: lv_conf.h 文件存在
    Steps:
      1. Select-String -Path "include/lv_conf.h" -Pattern "LV_FONT_CUSTOM_DECLARE|LV_FONT_DEFAULT" | Out-File .sisyphus/evidence/task-1.1-font-default.txt
      2. 检查输出包含 "hos_14_data"
    Expected Result: LV_FONT_CUSTOM_DECLARE行显示 `#define LV_FONT_CUSTOM_DECLARE LV_FONT_DECLARE(hos_14_data)`，LV_FONT_DEFAULT行显示 `#define LV_FONT_DEFAULT &hos_14_data`
    Failure Indicators: 输出不包含 "hos_14_data"
    Evidence: .sisyphus/evidence/task-1.1-font-default.txt
  ```

  **Commit**: YES (groups with 1.2)
  - Message: `feat(font): replace default font from Montserrat to hos_14`
  - Files: `include/lv_conf.h`
  - Pre-commit: `pio run -e native`

- [x] 1.2 关闭未使用的 Montserrat 宏

  **What to do**:
  - 打开 `include/lv_conf.h`
  - 修改以下行:
    - 第206行: `#define LV_FONT_MONTSERRAT_14 1` → `#define LV_FONT_MONTSERRAT_14 0`
    - 第207行: `#define LV_FONT_MONTSERRAT_16 1` → `#define LV_FONT_MONTSERRAT_16 0`
    - 第209行: `#define LV_FONT_MONTSERRAT_20 1` → `#define LV_FONT_MONTSERRAT_20 0`
    - 第213行: `#define LV_FONT_MONTSERRAT_28 1` → `#define LV_FONT_MONTSERRAT_28 0`

  **Must NOT do**:
  - 不修改其他Montserrat宏（已经是0的保持不变）
  - 不修改LV_FONT_MONTSERRAT_28_COMPRESSED

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 2.1, 2.2, 2.3
  - **Blocked By**: 1.1

  **References**:

  **Pattern References**:
  - `include/lv_conf.h:206-213` - Montserrat字体宏定义区域

  **WHY Each Reference Matters**:
  - 这些行是需要修改的目标行

  **Acceptance Criteria**:
  - [ ] LV_FONT_MONTSERRAT_14 设为 0
  - [ ] LV_FONT_MONTSERRAT_16 设为 0
  - [ ] LV_FONT_MONTSERRAT_20 设为 0
  - [ ] LV_FONT_MONTSERRAT_28 设为 0

  **QA Scenarios**:

  ```
  Scenario: 验证 Montserrat 宏已关闭
    Tool: PowerShell (Select-String)
    Preconditions: lv_conf.h 文件存在
    Steps:
      1. Select-String -Path "include/lv_conf.h" -Pattern "LV_FONT_MONTSERRAT_14|LV_FONT_MONTSERRAT_16|LV_FONT_MONTSERRAT_20|LV_FONT_MONTSERRAT_28" | Out-File .sisyphus/evidence/task-1.2-montserrat-disabled.txt
      2. 检查输出中这4个宏的值都是0
    Expected Result: 4个宏都显示为 `... 0`
    Failure Indicators: 任何宏显示为 `... 1`
    Evidence: .sisyphus/evidence/task-1.2-montserrat-disabled.txt
  ```

  **Commit**: YES (groups with 1.1)
  - Message: `feat(font): disable unused Montserrat font macros`
  - Files: `include/lv_conf.h`
  - Pre-commit: `pio run -e native`

- [x] 2.1 验证 Montserrat 引用

  **What to do**:
  - 全局搜索项目中是否有其他代码引用Montserrat字体
  - 使用Select-String搜索"montserrat"（不区分大小写）
  - 确认没有其他地方依赖Montserrat字体

  **Must NOT do**:
  - 不修改任何文件

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 2.2, 2.3
  - **Blocked By**: 1.1, 1.2

  **References**:

  **Pattern References**:
  - 项目根目录 - 需要全局搜索

  **WHY Each Reference Matters**:
  - 确保没有遗漏的Montserrat引用

  **Acceptance Criteria**:
  - [ ] 全局搜索确认无其他Montserrat引用

  **QA Scenarios**:

  ```
  Scenario: 验证无其他 Montserrat 引用
    Tool: PowerShell (Select-String)
    Preconditions: 项目根目录
    Steps:
      1. Get-ChildItem -Recurse -Include "*.c","*.h","*.cpp" -Path "include","src","ui","test" | Select-String -Pattern "montserrat" -CaseSensitive:$false | Out-File .sisyphus/evidence/task-2.1-montserrat-search.txt
      2. 检查输出是否为空或仅包含lv_conf.h中的定义
    Expected Result: 无输出或仅包含lv_conf.h相关行
    Failure Indicators: 输出包含其他项目源码文件的Montserrat引用（排除.pio依赖库）
    Evidence: .sisyphus/evidence/task-2.1-montserrat-search.txt
  ```

  **Commit**: NO

- [x] 2.2 编译 native 环境

  **What to do**:
  - 运行 `pio run -e native` 编译native环境
  - 检查编译是否成功，无错误

  **Must NOT do**:
  - 不修改任何代码

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 3.1
  - **Blocked By**: 2.1

  **References**:

  **Pattern References**:
  - `platformio.ini` - 构建配置

  **WHY Each Reference Matters**:
  - 确认编译命令正确

  **Acceptance Criteria**:
  - [ ] `pio run -e native` 编译成功，退出码为0

  **QA Scenarios**:

  ```
  Scenario: 验证 native 编译成功
    Tool: PowerShell (pio)
    Preconditions: 修改后的lv_conf.h
    Steps:
      1. pio run -e native | Out-File .sisyphus/evidence/task-2.2-native-build.txt
      2. 检查退出码
    Expected Result: 编译成功，退出码为0
    Failure Indicators: 编译失败或退出码非0
    Evidence: .sisyphus/evidence/task-2.2-native-build.txt
  ```

  **Commit**: NO

- [x] 2.3 编译 esp32s3 环境

  **What to do**:
  - 运行 `pio run -e esp32s3` 编译ESP32-S3环境
  - 检查编译是否成功，无错误

  **Must NOT do**:
  - 不修改任何代码

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 3.1
  - **Blocked By**: 2.1

  **References**:

  **Pattern References**:
  - `platformio.ini` - 构建配置

  **WHY Each Reference Matters**:
  - 确认编译命令正确

  **Acceptance Criteria**:
  - [ ] `pio run -e esp32s3` 编译成功，退出码为0

  **QA Scenarios**:

  ```
  Scenario: 验证 esp32s3 编译成功
    Tool: PowerShell (pio)
    Preconditions: 修改后的lv_conf.h
    Steps:
      1. pio run -e esp32s3 | Out-File .sisyphus/evidence/task-2.3-esp32s3-build.txt
      2. 检查退出码
    Expected Result: 编译成功，退出码为0
    Failure Indicators: 编译失败或退出码非0
    Evidence: .sisyphus/evidence/task-2.3-esp32s3-build.txt
  ```

  **Commit**: NO

- [x] 3.1 运行单元测试

  **What to do**:
  - 运行 `pio test -e native` 执行单元测试
  - 确保所有测试通过

  **Must NOT do**:
  - 不修改测试代码

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 4.1
  - **Blocked By**: 2.2, 2.3

  **References**:

  **Pattern References**:
  - `test/native/` - 测试目录

  **WHY Each Reference Matters**:
  - 确认测试命令正确

  **Acceptance Criteria**:
  - [ ] `pio test -e native` 所有测试通过

  **QA Scenarios**:

  ```
  Scenario: 验证单元测试通过
    Tool: PowerShell (pio)
    Preconditions: 编译成功
    Steps:
      1. pio test -e native | Out-File .sisyphus/evidence/task-3.1-unit-tests.txt
      2. 检查测试结果
    Expected Result: 所有测试通过
    Failure Indicators: 任何测试失败
    Evidence: .sisyphus/evidence/task-3.1-unit-tests.txt
  ```

  **Commit**: NO

- [x] 4.1 验证字体配置一致性

  **What to do**:
  - 验证字体配置修改的一致性
  - 确认hos字体引用正确

  **Must NOT do**:
  - 不修改UI代码

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 5.1
  - **Blocked By**: 3.1

  **References**:

  **Pattern References**:
  - `include/lv_conf.h` - LVGL字体配置
  - `ui/screens/*.xml` - UI屏幕定义
  - `ui/lof_power_system_gen.c` - 字体绑定

  **WHY Each Reference Matters**:
  - 确认字体配置修改的一致性

  **Acceptance Criteria**:
  - [ ] LV_FONT_DEFAULT指向hos_14_data
  - [ ] Montserrat字体宏已关闭
  - [ ] XML文件中的字体引用使用hos字体
  - [ ] FontAwesome字体绑定未被修改

  **QA Scenarios**:

  ```
  Scenario: 验证字体配置一致性
    Tool: PowerShell (Select-String)
    Preconditions: 编译成功
    Steps:
      1. Select-String -Path "include/lv_conf.h" -Pattern "LV_FONT_DEFAULT.*hos_14_data" | Out-File .sisyphus/evidence/task-4.1-font-config-consistency.txt
      2. Select-String -Path "ui/screens/*.xml" -Pattern "hos_14|hos_regular|hos_bold_big" | Out-File -Append .sisyphus/evidence/task-4.1-font-config-consistency.txt
      3. Select-String -Path "ui/lof_power_system_gen.c" -Pattern "font_awesome" | Out-File -Append .sisyphus/evidence/task-4.1-font-config-consistency.txt
    Expected Result: 所有字体配置一致，FontAwesome绑定未变
    Failure Indicators: 配置不一致或FontAwesome被修改
    Evidence: .sisyphus/evidence/task-4.1-font-config-consistency.txt
  ```

  **Commit**: NO

- [x] 5.1 查看固件大小

  **What to do**:
  - 运行 `pio run -e esp32s3 -t size` 查看固件大小
  - 记录修改后的固件大小

  **Must NOT do**:
  - 不修改任何代码

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  -   **Blocks**: 5.2
  - **Blocked By**: 4.1

  **References**:

  **Pattern References**:
  - `platformio.ini` - 构建配置

  **WHY Each Reference Matters**:
  - 确认固件大小检查命令

  **Acceptance Criteria**:
  - [ ] 获取固件大小数据

  **QA Scenarios**:

  ```
  Scenario: 验证固件大小
    Tool: PowerShell (pio)
    Preconditions: 编译成功
    Steps:
      1. pio run -e esp32s3 -t size | Out-File .sisyphus/evidence/task-5.1-firmware-size.txt
      2. 记录固件大小
    Expected Result: 固件大小数据已保存
    Failure Indicators: 命令执行失败
    Evidence: .sisyphus/evidence/task-5.1-firmware-size.txt
  ```

  **Commit**: NO

- [x] 5.2 对比固件体积变化

  **What to do**:
  - 运行 `pio run -e esp32s3 -t size` 获取修改后的固件大小
  - 从输出中提取RAM和ROM使用量数值
  - 与基线文件 `.sisyphus/evidence/baseline-firmware-size.txt` 中的数值对比
  - 确认ROM使用量减小或保持不变（关闭Montserrat字体应减小ROM）

  **Must NOT do**:
  - 不修改任何代码

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 6.1
  - **Blocked By**: 5.1

  **References**:

  **Pattern References**:
  - `.sisyphus/evidence/baseline-firmware-size.txt` - 修改前的固件大小基线

  **WHY Each Reference Matters**:
  - 与基线对比确认体积变化

  **Acceptance Criteria**:
  - [ ] ROM使用量减小或保持不变

  **QA Scenarios**:

  ```
  Scenario: 验证固件体积变化
    Tool: PowerShell
    Preconditions: 基线文件存在
    Steps:
      1. pio run -e esp32s3 -t size | Out-File .sisyphus/evidence/task-5.2-modified-firmware-size.txt
      2. $baseline = Get-Content .sisyphus/evidence/baseline-firmware-size.txt | Select-String "Used"
      3. $modified = Get-Content .sisyphus/evidence/task-5.2-modified-firmware-size.txt | Select-String "Used"
      4. Compare-Object $baseline $modified | Out-File .sisyphus/evidence/task-5.2-size-comparison.txt
    Expected Result: ROM使用量减小或保持不变
    Failure Indicators: ROM使用量显著增大
    Evidence: .sisyphus/evidence/task-5.2-size-comparison.txt
  ```

  **Commit**: NO

- [x] 6.1 更新 lv_conf.h 注释

  **What to do**:
  - 在`include/lv_conf.h`中添加注释，说明默认字体已更改为hos_14
  - 注释位置：第240行附近

  **Must NOT do**:
  - 不修改代码逻辑

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: 6.2
  - **Blocked By**: 5.2

  **References**:

  **Pattern References**:
  - `include/lv_conf.h:240` - LV_FONT_DEFAULT定义

  **WHY Each Reference Matters**:
  - 确认注释位置

  **Acceptance Criteria**:
  - [ ] 注释已添加

  **QA Scenarios**:

  ```
  Scenario: 验证注释已添加
    Tool: PowerShell (Select-String)
    Preconditions: lv_conf.h 文件存在
    Steps:
      1. Select-String -Path "include/lv_conf.h" -Pattern "hos_14" | Out-File .sisyphus/evidence/task-6.1-comment-added.txt
      2. 检查注释内容
    Expected Result: 注释说明默认字体已更改为hos_14
    Failure Indicators: 注释缺失
    Evidence: .sisyphus/evidence/task-6.1-comment-added.txt
  ```

  **Commit**: YES (groups with 6.2)
  - Message: `docs(font): add comment for default font change`
  - Files: `include/lv_conf.h`
  - Pre-commit: `pio run -e native`

- [x] 6.2 更新 AGENTS.md

  **What to do**:
  - 在`AGENTS.md`中记录字体配置变更
  - 说明默认字体已从Montserrat更改为hos_14

  **Must NOT do**:
  - 不修改其他文档

  **Recommended Agent Profile**:
  - **Category**: `writing`
  - **Skills**: []

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential
  - **Blocks**: None
  - **Blocked By**: 6.1

  **References**:

  **Pattern References**:
  - `AGENTS.md` - 项目知识库

  **WHY Each Reference Matters**:
  - 确认文档更新位置

  **Acceptance Criteria**:
  - [ ] AGENTS.md已更新

  **QA Scenarios**:

  ```
  Scenario: 验证 AGENTS.md 更新
    Tool: PowerShell (Select-String)
    Preconditions: AGENTS.md 文件存在
    Steps:
      1. Select-String -Path "AGENTS.md" -Pattern "hos_14|default font" | Out-File .sisyphus/evidence/task-6.2-agents-md-updated.txt
      2. 检查更新内容
    Expected Result: 文档记录了字体配置变更
    Failure Indicators: 文档未更新
    Evidence: .sisyphus/evidence/task-6.2-agents-md-updated.txt
  ```

  **Commit**: YES (groups with 6.1)
  - Message: `docs(font): update AGENTS.md with font configuration change`
  - Files: `AGENTS.md`
  - Pre-commit: `pio run -e native`

---

## Final Verification Wave (MANDATORY — after ALL implementation tasks)

> 4 review agents run in PARALLEL. ALL must APPROVE. Present consolidated results to user and get explicit "okay" before completing.

**准备工作**: 在执行Final Verification前，先创建 `.sisyphus/evidence/final-qa/` 目录。

- [ ] F1. **Plan Compliance Audit** — `oracle`
  Read the plan end-to-end. For each "Must Have": verify implementation exists. For each "Must NOT Have": search codebase for forbidden patterns. Check evidence files exist in .sisyphus/evidence/. Compare deliverables against plan.
  Output: `Must Have [N/N] | Must NOT Have [N/N] | Tasks [N/N] | VERDICT: APPROVE/REJECT`
  QA:
    Tool: PowerShell
    Steps:
      1. Select-String -Path "include/lv_conf.h" -Pattern "LV_FONT_DEFAULT.*hos_14_data"
      2. Select-String -Path "include/lv_conf.h" -Pattern "LV_FONT_MONTSERRAT_14.*0|LV_FONT_MONTSERRAT_16.*0|LV_FONT_MONTSERRAT_20.*0|LV_FONT_MONTSERRAT_28.*0"
      3. Get-ChildItem .sisyphus/evidence/ | Out-File .sisyphus/evidence/final-qa/F1-plan-compliance.txt
    Expected Result: LV_FONT_DEFAULT指向hos_14_data，Montserrat宏为0，证据文件存在
    Evidence: .sisyphus/evidence/final-qa/F1-plan-compliance.txt

- [ ] F2. **Code Quality Review** — `unspecified-high`
  Run `pio run -e native` + `pio run -e esp32s3` + `pio test -e native`. Review changed files for compilation errors.
  Output: `Build [PASS/FAIL] | Tests [N pass/N fail] | VERDICT`
  QA:
    Tool: PowerShell (pio)
    Steps:
      1. pio run -e native | Out-File .sisyphus/evidence/final-qa/F2-native-build.txt
      2. pio run -e esp32s3 | Out-File .sisyphus/evidence/final-qa/F2-esp32s3-build.txt
      3. pio test -e native | Out-File .sisyphus/evidence/final-qa/F2-tests.txt
    Expected Result: 所有编译成功，所有测试通过
    Evidence: .sisyphus/evidence/final-qa/F2-code-quality.txt

- [ ] F3. **Real QA** — `unspecified-high`
  Execute EVERY QA scenario from EVERY task — follow exact steps, capture evidence.
  Output: `Scenarios [N/N pass] | VERDICT`
  QA:
    Tool: PowerShell
    Steps:
      1. 执行Task 0.1-6.2的所有QA场景
      2. 将所有结果保存到 .sisyphus/evidence/final-qa/F3-real-qa.txt
    Expected Result: 所有QA场景通过
    Evidence: .sisyphus/evidence/final-qa/F3-real-qa.txt

- [ ] F4. **Scope Fidelity Check** — `deep`
  For each task: read "What to do", read actual diff (git log/diff). Verify 1:1 — everything in spec was built (no missing), nothing beyond spec was built (no creep). Check "Must NOT do" compliance.
  Output: `Tasks [N/N compliant] | VERDICT`
  QA:
    Tool: PowerShell (git)
    Steps:
      1. git diff --stat | Out-File .sisyphus/evidence/final-qa/F4-scope-fidelity.txt
      2. git log --oneline -5 | Out-File -Append .sisyphus/evidence/final-qa/F4-scope-fidelity.txt
    Expected Result: 只有include/lv_conf.h和AGENTS.md被修改
    Evidence: .sisyphus/evidence/final-qa/F4-scope-fidelity.txt

---

## Commit Strategy

- **1.1, 1.2**: `feat(font): replace default font from Montserrat to hos_14` - `include/lv_conf.h`
- **6.1, 6.2**: `docs(font): update documentation for font configuration change` - `include/lv_conf.h`, `AGENTS.md`

---

## Success Criteria

### Verification Commands
```bash
pio run -e native  # Expected: BUILD SUCCESS
pio run -e esp32s3  # Expected: BUILD SUCCESS
pio test -e native  # Expected: ALL TESTS PASSED
pio run -e esp32s3 -t size  # Expected: firmware size data
```

### Final Checklist
- [ ] LV_FONT_DEFAULT 指向 hos_14_data
- [ ] LV_FONT_CUSTOM_DECLARE 已添加
- [ ] LV_FONT_MONTSERRAT_14/16/20/28 设为 0
- [ ] native 编译成功
- [ ] esp32s3 编译成功
- [ ] 所有单元测试通过
- [ ] 字体配置一致性验证通过
- [ ] FontAwesome 字体绑定未被修改
- [ ] 固件体积减小或保持不变
- [ ] 文档已更新
