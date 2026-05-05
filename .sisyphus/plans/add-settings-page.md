# ESP32风扇控制器设置页面实现计划

## TL;DR

> **快速摘要**: 为ESP32风扇控制器添加运行时设置页面，通过3个物理按键调整风扇曲线、温度阈值、亮度、功率配置和传感器校准参数，使用NVS持久化保存。
>
> **交付物**:
> - config_manager模块（运行时配置管理 + NVS持久化）
> - settings_ui模块（LVGL设置页面 + 按键交互）
> - 修改后的fan_curve/fault_guard/data_bridge（使用运行时配置）
> - 单元测试和集成测试
>
> **预估工作量**: Large
> **并行执行**: YES - 4 waves
> **关键路径**: config_manager → 修改控制模块 → settings_ui → 集成测试

---

## Context

### 原始需求
用户需要在ESP32风扇控制器上添加设置页面，通过3个物理按键（上/中/下）调整常用参数，无需重新编译固件。

### 访谈摘要
**关键讨论**:
- 功能选择：风扇转速曲线、温度报警阈值、显示亮度、传感器校准、设计功率选择（350/450/550/750W）
- 交互设计：3按键（K1=上，K2=确认，K3=下），长按切换页面，闪烁编辑模式
- 持久化：使用ESP32 Preferences (NVS)保存用户设置
- UI架构：LVGL Editor XML + 手写交互逻辑

**研究发现**:
- 现有配置均为app_config.h中的static constexpr编译期常量
- 无NVS使用（设计层面排除）
- UI框架：LVGL 9.x，由LVGL Editor生成代码
- 数据流：app_state → data_bridge (200ms) → lv_subject → UI控件绑定

### OpenSpec文档
- proposal.md: 变更提案
- design.md: 7个技术决策
- specs/settings-ui: 10个需求、17个场景
- specs/config-management: 5个需求、10个场景
- specs/nvs-persistence: 5个需求、8个场景

### 默认值来源（解决Momus问题#1）

**重要说明**：app_config.h中**缺少**以下设置的默认值，需要在config_manager.cpp中硬编码：

| 设置项 | 默认值 | 来源 |
|--------|--------|------|
| fan_temp_low | 35.0°C | **硬编码** (用户要求：35°C启动风扇) |
| fan_temp_mid | 45.0°C | **硬编码** (35-55°C中间值) |
| fan_temp_high | 55.0°C | **硬编码** (用户要求：55°C满速) |
| fan_temp_force | 60.0°C | **硬编码** (略高于满速阈值) |
| fan_pwm_min_percent | 20% | 计算: FAN_PWM_MIN/1023*100 |
| fan_pwm_mid_percent | 60% | 计算: 614/1023*100 |
| fan_hysteresis | 2.0°C | app_config.h (FAN_HYSTERESIS) |
| temp_warning_threshold | 65.0°C | **硬编码** (略高于满速阈值) |
| temp_shutdown_threshold | 75.0°C | **硬编码** (fault_guard.cpp OVER_TEMP_C) |
| brightness_percent | 80% | **硬编码** (specs/settings-ui默认值) |
| design_power_w | 750W | **硬编码** (data_bridge.cpp DEVICE_DESIGN_POWER_W) |
| ntc_temp_offset | 0.0°C | **硬编码** (specs/settings-ui默认值) |

**温度曲线说明**：
- 35°C: 风扇启动（最低转速20%）
- 45°C: 中间转速（60%）
- 55°C: 满速（100%）
- 60°C: 强制满速（紧急散热）
- 65°C: 过温警告
- 75°C: 过温关机保护

**规则**：
- 优先使用app_config.h中的现有常量
- 对于app_config.h中不存在的常量，在config_manager.cpp中硬编码默认值
- 不修改app_config.h添加新常量（遵循"Must NOT do"规则）

---

## Work Objectives

### 核心目标
实现运行时配置管理系统，让用户通过设置页面调整常用参数，无需重新编译固件。

### 具体交付物
- `src/app/config_manager.{h,cpp}` - 配置管理模块
- `src/ui_bridge/settings_ui.{h,cpp}` - 设置页面UI逻辑
- `ui/screens/settings.xml` - LVGL Editor XML模板
- `ui/screens/settings_gen.c/h` - 用户导出的生成代码
- 修改后的`fan_curve.cpp`、`fault_guard.cpp`、`data_bridge.cpp`
- 单元测试：`test/native/test_config_manager/`

### 完成定义
- [ ] 所有配置项可通过设置页面调整
- [ ] 设置在重启后保留（NVS持久化）
- [ ] 风扇控制、故障保护、数据桥使用运行时配置
- [ ] 单元测试通过
- [ ] 编译验证通过（esp32s3 + native）

### 必须实现
- 5个分类设置页面（风扇、温度、显示、功率、传感器）
- 3按键导航（K1/K3移动，K2确认/编辑）
- NVS持久化保存
- 线程安全的配置访问

### 必须不实现（护栏）
- 不修改app_config.h的编译期常量
- 不实现触摸交互
- 不实现WiFi/蓝牙远程配置
- 不修改硬件引脚定义
- 不修改LVGL Editor生成的代码（*_gen.c/h）

---

## Verification Strategy

### 测试决策
- **基础设施存在**: YES（Unity框架，pio test -e native）
- **自动化测试**: YES（Tests-after）
- **框架**: Unity（test/native/）
- **策略**: 先实现功能，后添加测试

### QA策略
每个任务必须包含代理执行的QA场景：
- **前端/UI**: 使用Playwright（playwright skill）- 导航、交互、断言DOM、截图
- **CLI/TUI**: 使用interactive_bash (tmux) - 运行命令、发送按键、验证输出
- **API/后端**: 使用Bash (curl) - 发送请求、断言状态和响应字段
- **库/模块**: 使用Bash (bun/node REPL) - 导入、调用函数、比较输出

---

## Execution Strategy

### 并行执行波次

```
Wave 1 (立即开始 - 基础模块):
├── Task 1: config_manager模块 [deep]
├── Task 2: LVGL Editor XML模板 [quick]
├── Task 3: 更新platformio.ini [quick]
└── Task 4: 创建测试目录结构 [quick]

Wave 2 (Wave 1后 - 修改控制模块):
├── Task 5: 修改fan_curve.cpp (依赖: 1) [deep]
├── Task 6: 修改fault_guard.cpp (依赖: 1) [deep]
├── Task 7: 修改data_bridge.cpp (依赖: 1) [deep]
└── Task 8: 用户导出settings_gen.c/h (依赖: 2) [manual]

Wave 3 (Wave 2后 - UI实现):
├── Task 9: settings_ui模块 (依赖: 1, 8) [deep]
├── Task 10: 集成到screen_manager (依赖: 9) [unspecified-high]
└── Task 11: 更新input_bridge (依赖: 9) [unspecified-high]

Wave 4 (Wave 3后 - 测试和文档):
├── Task 12: config_manager单元测试 (依赖: 1) [unspecified-high]
├── Task 13: 集成测试 (依赖: 9, 10, 11) [unspecified-high]
└── Task 14: 更新文档 (依赖: all) [writing]

Wave FINAL (所有任务后 - 4个并行审查):
├── Task F1: 计划合规审计 (oracle)
├── Task F2: 代码质量审查 (unspecified-high)
├── Task F3: 实际手动QA (unspecified-high)
└── Task F4: 范围保真度检查 (deep)
-> 展示结果 -> 获取用户明确确认

关键路径: Task 1 → Task 5/6/7 → Task 9 → Task 10/11 → Task 13 → F1-F4 → 用户确认
并行加速: ~60% faster than sequential
最大并发: 4 (Waves 1 & 2)
```

### 依赖矩阵

| Task | Depends On | Blocks | Wave |
|------|------------|--------|------|
| 1 | - | 5,6,7,9,12 | 1 |
| 2 | - | 8 | 1 |
| 3 | - | all | 1 |
| 4 | - | 12,13 | 1 |
| 5 | 1 | 13 | 2 |
| 6 | 1 | 13 | 2 |
| 7 | 1 | 13 | 2 |
| 8 | 2 | 9 | 2 |
| 9 | 1,8 | 10,11,13 | 3 |
| 10 | 9 | 13 | 3 |
| 11 | 9 | 13 | 3 |
| 12 | 1,4 | F1-F4 | 4 |
| 13 | 5,6,7,9,10,11 | F1-F4 | 4 |
| 14 | all | F1-F4 | 4 |
| F1-F4 | 12,13,14 | - | FINAL |

### Agent Dispatch Summary

- **Wave 1**: 4 tasks - T1 → `deep`, T2-T4 → `quick`
- **Wave 2**: 4 tasks - T5-T7 → `deep`, T8 → `manual`
- **Wave 3**: 3 tasks - T9 → `deep`, T10-T11 → `unspecified-high`
- **Wave 4**: 3 tasks - T12-T13 → `unspecified-high`, T14 → `writing`
- **FINAL**: 4 tasks - F1 → `oracle`, F2 → `unspecified-high`, F3 → `unspecified-high`, F4 → `deep`

---

## TODOs

- [x] 1. 创建config_manager模块

  **What to do**:
  - 创建`src/app/config_manager.h`：定义AppConfig结构体（FanConfig、TempProtectionConfig、DisplayConfig、PowerConfig、SensorConfig）
  - 声明getter/setter函数（get_fan_temp_low/set_fan_temp_low等）
  - 声明init()、reset_to_defaults()、load_from_nvs()、save_to_nvs()
  - 创建`src/app/config_manager.cpp`：实现默认值初始化（从app_config.h）
  - 实现NVS读写（Preferences库，namespace "esm_power_lof"）
  - 实现值边界检查（min/max clamping）
  - 实现线程安全（std::atomic或mutex）

  **Must NOT do**:
  - 不修改app_config.h的现有常量
  - 不直接在config_manager中引用硬件引脚

  **Recommended Agent Profile**:
  > **Category**: `deep`
  > **Skills**: []
  > **Reason**: 需要深入理解ESP32 Preferences API和线程安全

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 2, 3, 4)
  - **Blocks**: Tasks 5, 6, 7, 9, 12
  - **Blocked By**: None

  **References**:
  - `include/app_config.h` - 所有默认值常量
  - `src/app/app_state.h` - std::atomic使用模式
  - `openspec/changes/add-settings-page/specs/config-management/spec.md` - 需求规格
  - `openspec/changes/add-settings-page/design.md` - 决策1（独立config_manager模块）

  **Acceptance Criteria**:
  - [ ] config_manager.h/cpp存在且可编译
  - [ ] 所有getter/setter函数实现
  - [ ] init()从NVS加载或使用默认值
  - [ ] 线程安全访问

  **QA Scenarios**:
  ```
  Scenario: 编译验证
    Tool: Bash
    Preconditions: 无
    Steps:
      1. pio run -e esp32s3
      2. 检查退出码为0
    Expected Result: BUILD SUCCESS
    Evidence: .sisyphus/evidence/task-1-compile.txt

  Scenario: 单元测试
    Tool: Bash
    Preconditions: 测试文件存在
    Steps:
      1. pio test -e native -f test_config_manager
      2. 检查所有测试通过
    Expected Result: All tests passed
    Evidence: .sisyphus/evidence/task-1-test.txt
  ```

  **Commit**: YES
  - Message: `feat(config): add config_manager module with NVS persistence`
  - Files: `src/app/config_manager.h`, `src/app/config_manager.cpp`

- [x] 2. 创建LVGL Editor XML模板

  **What to do**:
  - 创建`ui/screens/settings.xml`
  - 定义容器结构：标题栏（返回图标+分类名+页码）、内容区域（可滚动）、底部提示栏
  - 定义样式：style_item、style_item_focused、style_item_editing
  - 内容区域为空容器，由代码动态填充

  **Must NOT do**:
  - 不包含具体设置项（由settings_ui.cpp动态添加）
  - 不使用触摸相关属性

  **Recommended Agent Profile**:
  > **Category**: `quick`
  > **Skills**: []
  > **Reason**: 简单的XML文件创建

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 3, 4)
  - **Blocks**: Task 8
  - **Blocked By**: None

  **References**:
  - `ui/screens/home.xml` - 现有XML格式和样式模式
  - `ui/screens/splash.xml` - 样式定义示例
  - `ui/project.xml` - 屏幕尺寸（240x280）
  - `openspec/changes/add-settings-page/design.md` - 决策3（UI实现）

  **Acceptance Criteria**:
  - [ ] settings.xml存在且格式正确
  - [ ] 包含标题栏、内容区域、底部提示栏
  - [ ] 定义了3种状态样式（normal/focused/editing）

  **QA Scenarios**:
  ```
  Scenario: XML格式验证
    Tool: Bash
    Preconditions: 无
    Steps:
      1. 检查settings.xml文件存在
      2. 验证XML格式正确（xmllint或类似工具）
    Expected Result: XML格式有效
    Evidence: .sisyphus/evidence/task-2-xml-validation.txt
  ```

  **Commit**: YES
  - Message: `feat(ui): add settings.xml template for LVGL Editor`
  - Files: `ui/screens/settings.xml`

- [x] 3. 更新platformio.ini构建配置

  **What to do**:
  - 添加Preferences库依赖（lib_deps）
  - 添加config_manager.cpp到build_src_filter
  - 添加settings_ui.cpp到build_src_filter（如果已创建）

  **Must NOT do**:
  - 不修改现有构建环境（esp32s3、native）
  - 不删除现有依赖

  **Recommended Agent Profile**:
  > **Category**: `quick`
  > **Skills**: []
  > **Reason**: 简单的配置文件修改

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 2, 4)
  - **Blocks**: All subsequent tasks
  - **Blocked By**: None

  **References**:
  - `platformio.ini` - 现有构建配置
  - `openspec/changes/add-settings-page/proposal.md` - 依赖说明

  **Acceptance Criteria**:
  - [ ] Preferences库在lib_deps中
  - [ ] 新源文件在build_src_filter中
  - [ ] pio run -e esp32s3成功

  **QA Scenarios**:
  ```
  Scenario: 构建验证
    Tool: Bash
    Preconditions: 无
    Steps:
      1. pio run -e esp32s3
      2. 检查退出码为0
    Expected Result: BUILD SUCCESS
    Evidence: .sisyphus/evidence/task-3-build.txt
  ```

  **Commit**: YES
  - Message: `build: add Preferences dependency and source filters`
  - Files: `platformio.ini`

- [x] 4. 创建测试目录结构

  **What to do**:
  - 创建`test/native/test_config_manager/`目录
  - 创建`test_main.cpp`骨架（Unity框架）
  - 添加setUp/tearDown函数
  - 添加基本测试用例框架

  **Must NOT do**:
  - 不实现具体测试逻辑（在Task 12中实现）

  **Recommended Agent Profile**:
  > **Category**: `quick`
  > **Skills**: []
  > **Reason**: 简单的目录和文件创建

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 2, 3)
  - **Blocks**: Task 12
  - **Blocked By**: None

  **References**:
  - `test/native/test_fan_curve/` - 现有测试目录结构
  - `test/native/test_psu_fsm/` - Unity框架使用示例

  **Acceptance Criteria**:
  - [ ] 测试目录存在
  - [ ] test_main.cpp存在且可编译
  - [ ] 包含setUp/tearDown函数

  **QA Scenarios**:
  ```
  Scenario: 测试目录验证
    Tool: Bash
    Preconditions: 无
    Steps:
      1. 检查test/native/test_config_manager/目录存在
      2. 检查test_main.cpp文件存在
    Expected Result: 目录和文件存在
    Evidence: .sisyphus/evidence/task-4-test-dir.txt
  ```

  **Commit**: YES
  - Message: `test: add config_manager test directory structure`
  - Files: `test/native/test_config_manager/test_main.cpp`

- [x] 5. 修改fan_curve.cpp使用config_manager

  **What to do**:
  - 在fan_curve.cpp中包含config_manager.h
  - 替换FAN_TEMP_LOW/MID/HIGH/FORCE为config_manager::get_fan_temp_xxx()
  - 替换FAN_PWM_MIN/MAX为config_manager::get_fan_pwm_xxx_percent()
  - 将mid_duty (614)移到config_manager
  - 更新fan_curve.h添加#include

  **Must NOT do**:
  - 不改变fan_curve的算法逻辑
  - 不修改函数签名（保持API兼容）

  **Recommended Agent Profile**:
  > **Category**: `deep`
  > **Skills**: []
  > **Reason**: 需要理解风扇控制逻辑和配置集成

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 6, 7)
  - **Blocks**: Task 13
  - **Blocked By**: Task 1

  **References**:
  - `src/fan/fan_curve.cpp` - 现有实现
  - `src/fan/fan_curve.h` - 函数声明
  - `include/app_config.h` - 当前使用的常量
  - `openspec/changes/add-settings-page/specs/settings-ui/spec.md` - 风扇设置参数范围

  **Acceptance Criteria**:
  - [ ] fan_curve.cpp使用config_manager获取配置
  - [ ] 不再直接引用app_config.h的风扇相关常量
  - [ ] 风扇控制行为不变（使用相同默认值）

  **QA Scenarios**:
  ```
  Scenario: 编译验证
    Tool: Bash
    Preconditions: Task 1完成
    Steps:
      1. pio run -e esp32s3
      2. 检查退出码为0
    Expected Result: BUILD SUCCESS
    Evidence: .sisyphus/evidence/task-5-compile.txt

  Scenario: 功能验证
    Tool: Bash
    Preconditions: 测试存在
    Steps:
      1. pio test -e native -f test_fan_curve
      2. 检查所有测试通过
    Expected Result: All tests passed
    Evidence: .sisyphus/evidence/task-5-test.txt
  ```

  **Commit**: YES
  - Message: `refactor(fan): use config_manager for fan curve parameters`
  - Files: `src/fan/fan_curve.cpp`, `src/fan/fan_curve.h`

- [x] 6. 修改fault_guard.cpp使用config_manager

  **What to do**:
  - 在fault_guard.cpp中包含config_manager.h
  - 替换OVER_TEMP_C (80°C)为config_manager::get_temp_shutdown_threshold()
  - 替换STALL_RPM_THRESH (100)为config_manager getter
  - 替换FAN_STALL_DUTY_THRESH/FAN_STALL_TIMEOUT_MS为config_manager getters

  **Must NOT do**:
  - 不改变故障保护逻辑
  - 不修改保护行为（仅配置来源）

  **Recommended Agent Profile**:
  > **Category**: `deep`
  > **Skills**: []
  > **Reason**: 需要理解故障保护逻辑

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 5, 7)
  - **Blocks**: Task 13
  - **Blocked By**: Task 1

  **References**:
  - `src/app/fault_guard.cpp` - 现有实现
  - `include/app_config.h` - 当前使用的常量
  - `openspec/changes/add-settings-page/specs/settings-ui/spec.md` - 温度保护参数范围

  **Acceptance Criteria**:
  - [ ] fault_guard.cpp使用config_manager获取配置
  - [ ] 不再直接引用app_config.h的保护相关常量

  **QA Scenarios**:
  ```
  Scenario: 编译验证
    Tool: Bash
    Preconditions: Task 9完成
    Steps:
      1. pio run -e esp32s3
      2. 检查退出码为0
    Expected Result: BUILD SUCCESS
    Evidence: .sisyphus/evidence/task-10-compile.txt

  Scenario: 屏幕切换测试（手动QA）
    Tool: Manual
    Preconditions: 固件烧录到硬件
    Steps:
      1. 在home界面长按K2
      2. 验证进入settings页面
      3. 在settings长按K2
      4. 验证返回home界面
    Expected Result: 屏幕切换正常
    Evidence: .sisyphus/evidence/task-10-screen-switch.txt (手动记录)
  ```

  **注意**：屏幕切换测试需要在真实硬件上手动执行。

  **Commit**: YES
  - Message: `refactor(fault): use config_manager for protection thresholds`
  - Files: `src/app/fault_guard.cpp`

- [x] 7. 修改data_bridge.cpp使用配置的功率值

  **What to do**:
  - 在data_bridge.cpp中包含config_manager.h
  - 替换DEVICE_DESIGN_POWER_W (750)为config_manager::get_design_power_w()
  - 更新负载百分比计算使用动态功率值

  **Must NOT do**:
  - 不改变数据刷新逻辑
  - 不修改其他显示数据

  **Recommended Agent Profile**:
  > **Category**: `deep`
  > **Skills**: []
  > **Reason**: 需要理解数据桥和功率计算

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 5, 6)
  - **Blocks**: Task 13
  - **Blocked By**: Task 1

  **References**:
  - `src/ui_bridge/data_bridge.cpp` - 现有实现
  - `openspec/changes/add-settings-page/design.md` - 决策7（数据流设计）

  **Acceptance Criteria**:
  - [ ] data_bridge.cpp使用config_manager获取设计功率
  - [ ] 负载百分比计算使用动态功率值

  **QA Scenarios**:
  ```
  Scenario: 编译验证
    Tool: Bash
    Preconditions: Task 9完成
    Steps:
      1. pio run -e esp32s3
      2. 检查退出码为0
    Expected Result: BUILD SUCCESS
    Evidence: .sisyphus/evidence/task-11-compile.txt

  Scenario: 按键路由测试（手动QA）
    Tool: Manual
    Preconditions: 固件烧录到硬件
    Steps:
      1. 在home界面长按K2
      2. 验证进入settings
      3. K1/K3移动焦点
      4. 验证焦点移动正常
    Expected Result: 按键路由正确
    Evidence: .sisyphus/evidence/task-11-key-routing.txt (手动记录)
  ```

  **注意**：按键路由测试需要在真实硬件上手动执行。

  **Commit**: YES
  - Message: `refactor(data): use config_manager for design power`
  - Files: `src/ui_bridge/data_bridge.cpp`

- [x] 8. 用户导出settings_gen.c/h

  **What to do**:
  - 用户使用LVGL Editor打开ui/screens/settings.xml
  - 导出生成代码到ui/screens/settings_gen.c和settings_gen.h
  - 验证生成代码可编译

  **Must NOT do**:
  - 不手动生成代码（由用户操作）
  - 不修改生成的代码

  **Recommended Agent Profile**:
  > **Category**: `manual`
  > **Skills**: []
  > **Reason**: 需要用户手动操作LVGL Editor

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential (after Task 2)
  - **Blocks**: Task 9
  - **Blocked By**: Task 2

  **References**:
  - `ui/screens/settings.xml` - XML模板
  - `ui/screens/home_gen.c/h` - 生成代码示例

  **Acceptance Criteria**:
  - [ ] settings_gen.c/h存在
  - [ ] 包含settings_create()函数
  - [ ] 可编译

  **QA Scenarios**:
  ```
  Scenario: 生成代码验证
    Tool: Bash
    Preconditions: 用户已导出
    Steps:
      1. 检查settings_gen.c/h文件存在
      2. pio run -e esp32s3
      3. 检查退出码为0
    Expected Result: 文件存在且可编译
    Evidence: .sisyphus/evidence/task-8-gen-code.txt
  ```

  **Commit**: YES
  - Message: `feat(ui): add settings_gen.c/h from LVGL Editor`
  - Files: `ui/screens/settings_gen.c`, `ui/screens/settings_gen.h`

- [x] 9. 创建settings_ui模块

  **What to do**:
  - 创建`src/ui_bridge/settings_ui.h`：定义SettingsUI类/命名空间
  - 声明init()、show()、hide()、handle_key()函数
  - 定义settings_page_enum（FAN、TEMP、DISPLAY、POWER、SENSOR）
  - 定义settings_item_struct（label、value_ptr、type、min、max、step）
  - 创建`src/ui_bridge/settings_ui.cpp`：
    - 实现动态设置项创建（在content_area中）
    - 实现焦点管理（K1/K3移动，循环）
    - 实现编辑模式（K2进入/确认，闪烁动画）
    - 实现数值调整（K1/K3增减，边界检查）
    - 实现页面切换（长按K1/K3）
    - 实现5个分类页面的数据定义

  **Must NOT do**:
  - 不修改settings_gen.c/h
  - 不使用触摸相关API

  **Recommended Agent Profile**:
  > **Category**: `deep`
  > **Skills**: []
  > **Reason**: 复杂的UI交互逻辑和LVGL API

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Sequential (after Tasks 1, 8)
  - **Blocks**: Tasks 10, 11, 13
  - **Blocked By**: Tasks 1, 8

  **References**:
  - `src/ui_bridge/input_bridge.cpp` - 按键处理模式
  - `src/ui_bridge/screen_manager.cpp` - 屏幕管理模式
  - `ui/screens/settings_gen.c` - 生成的UI代码
  - `openspec/changes/add-settings-page/specs/settings-ui/spec.md` - 完整需求
  - `openspec/changes/add-settings-page/design.md` - 决策4、5、6

  **Acceptance Criteria**:
  - [ ] settings_ui.h/cpp存在且可编译
  - [ ] 实现焦点管理和页面切换
  - [ ] 实现编辑模式和数值调整
  - [ ] 5个分类页面数据定义完整

  **QA Scenarios**:
  ```
  Scenario: 编译验证
    Tool: Bash
    Preconditions: Tasks 1, 8完成
    Steps:
      1. pio run -e esp32s3
      2. 检查退出码为0
    Expected Result: BUILD SUCCESS
    Evidence: .sisyphus/evidence/task-9-compile.txt

  Scenario: UI交互测试（手动QA）
    Tool: Manual
    Preconditions: 固件烧录到硬件
    Steps:
      1. 长按K2进入设置
      2. K1/K3移动焦点
      3. K2进入编辑模式
      4. K1/K3调整数值
      5. K2确认
      6. 长按K1/K3切换页面
      7. 长按K2返回
    Expected Result: 所有交互正常工作
    Evidence: .sisyphus/evidence/task-9-ui-interaction.txt (手动记录)
  ```

  **注意**：UI交互测试需要在真实硬件上手动执行，因为native环境无法模拟物理按键。测试结果由人工记录并截图。

  **Commit**: YES
  - Message: `feat(ui): implement settings_ui with keyboard navigation`
  - Files: `src/ui_bridge/settings_ui.h`, `src/ui_bridge/settings_ui.cpp`

- [x] 10. 集成到screen_manager

  **What to do**:
  - 在screen_manager.cpp中包含settings_ui.h
  - 在init()中创建settings页面
  - 添加settings页面load/unload函数
  - 处理home → settings → home转换

  **Must NOT do**:
  - 不修改现有home/splash页面逻辑
  - 不改变screen_manager的API

  **Recommended Agent Profile**:
  > **Category**: `unspecified-high`
  > **Skills**: []
  > **Reason**: 需要理解屏幕管理架构

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Task 11)
  - **Blocks**: Task 13
  - **Blocked By**: Task 9

  **References**:
  - `src/ui_bridge/screen_manager.cpp` - 现有实现
  - `openspec/changes/add-settings-page/design.md` - 决策4（页面导航）

  **Acceptance Criteria**:
  - [ ] screen_manager支持settings页面
  - [ ] 可以从home进入settings
  - [ ] 可以从settings返回home

  **QA Scenarios**:
  ```
  Scenario: 屏幕切换测试
    Tool: interactive_bash
    Preconditions: 固件烧录
    Steps:
      1. 在home界面长按K2
      2. 验证进入settings页面
      3. 在settings长按K2
      4. 验证返回home界面
    Expected Result: 屏幕切换正常
    Evidence: .sisyphus/evidence/task-10-screen-switch.txt
  ```

  **Commit**: YES
  - Message: `feat(screen): integrate settings page to screen_manager`
  - Files: `src/ui_bridge/screen_manager.cpp`

- [x] 11. 更新input_bridge支持设置导航

  **What to do**:
  - 在input_bridge.cpp中添加长按K2检测
  - 路由按键事件到settings_ui（当在settings模式）
  - 处理从settings返回home

  **Must NOT do**:
  - 不修改现有home界面的按键处理
  - 不改变input_bridge的API

  **Recommended Agent Profile**:
  > **Category**: `unspecified-high`
  > **Skills**: []
  > **Reason**: 需要理解按键处理架构

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Task 10)
  - **Blocks**: Task 13
  - **Blocked By**: Task 9

  **References**:
  - `src/ui_bridge/input_bridge.cpp` - 现有实现
  - `openspec/changes/add-settings-page/design.md` - 决策4（页面导航）

  **Acceptance Criteria**:
  - [ ] 长按K2可以进入设置
  - [ ] 按键事件正确路由到settings_ui
  - [ ] 从settings返回正常工作

  **QA Scenarios**:
  ```
  Scenario: 按键路由测试
    Tool: interactive_bash
    Preconditions: 固件烧录
    Steps:
      1. 在home界面长按K2
      2. 验证进入settings
      3. K1/K3移动焦点
      4. 验证焦点移动正常
    Expected Result: 按键路由正确
    Evidence: .sisyphus/evidence/task-11-key-routing.txt
  ```

  **Commit**: YES
  - Message: `feat(input): add settings navigation support`
  - Files: `src/ui_bridge/input_bridge.cpp`

- [x] 12. 创建config_manager单元测试

  **What to do**:
  - 在test/native/test_config_manager/test_main.cpp中实现测试
  - 测试默认值初始化
  - 测试getter/setter函数
  - 测试值边界检查（min/max clamping）
  - 测试reset_to_defaults()

  **Native测试策略（解决Momus问题#2）**：

  **重要说明**：native环境不支持Arduino Preferences库，需要使用以下策略：

  **选项A：Mock Preferences（推荐）**
  - 创建`test/native/test_config_manager/preferences_mock.h`
  - 实现简单的std::map作为NVS存储的mock
  - 在测试中使用mock代替真实Preferences

  **选项B：仅测试内存配置（简化方案）**
  - 跳过NVS持久化测试
  - 仅测试getter/setter和边界检查
  - NVS持久化在集成测试中验证（Task 13）

  **推荐**：使用选项B（简化方案），因为：
  - Native环境的主要价值是快速验证逻辑
  - NVS持久化需要真实硬件验证
  - 减少mock维护成本

  **Must NOT do**:
  - 不在native环境中测试NVS持久化
  - 不创建复杂的Preferences mock

  **Recommended Agent Profile**:
  > **Category**: `unspecified-high`
  > **Skills**: []
  > **Reason**: 需要理解Unity测试框架

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Tasks 13, 14)
  - **Blocks**: F1-F4
  - **Blocked By**: Tasks 1, 4

  **References**:
  - `test/native/test_fan_curve/` - 测试模式示例
  - `openspec/changes/add-settings-page/specs/config-management/spec.md` - 需求规格
  - `platformio.ini` - native环境配置（无Arduino框架）

  **Acceptance Criteria**:
  - [ ] 测试文件包含getter/setter和边界检查测试
  - [ ] 所有测试通过
  - [ ] NVS持久化测试标记为跳过或在集成测试中验证

  **QA Scenarios**:
  ```
  Scenario: 单元测试执行
    Tool: Bash
    Preconditions: Task 1完成
    Steps:
      1. pio test -e native -f test_config_manager
      2. 检查所有测试通过
      3. 验证NVS相关测试被跳过或使用mock
    Expected Result: All tests passed (NVS tests skipped)
    Evidence: .sisyphus/evidence/task-12-unit-test.txt
  ```

  **Commit**: YES
  - Message: `test(config): add config_manager unit tests (native-compatible)`
  - Files: `test/native/test_config_manager/test_main.cpp`

- [x] 13. 集成测试

  **What to do**:
  - 测试完整的设置流程：进入设置 → 修改值 → 保存 → 重启 → 验证值保留
  - 测试风扇控制使用新配置
  - 测试故障保护使用新配置
  - 测试数据桥使用新功率值

  **Must NOT do**:
  - 不测试UI视觉效果（手动QA中测试）

  **Recommended Agent Profile**:
  > **Category**: `unspecified-high`
  > **Skills**: []
  > **Reason**: 需要理解整个系统集成

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Tasks 12, 14)
  - **Blocks**: F1-F4
  - **Blocked By**: Tasks 5, 6, 7, 9, 10, 11

  **References**:
  - `openspec/changes/add-settings-page/specs/nvs-persistence/spec.md` - 持久化需求

  **Acceptance Criteria**:
  - [ ] 设置值在重启后保留
  - [ ] 风扇控制使用新配置
  - [ ] 故障保护使用新配置

  **QA Scenarios**:
  ```
  Scenario: NVS持久化测试（手动QA）
    Tool: Manual
    Preconditions: 固件烧录到硬件
    Steps:
      1. 进入设置页面
      2. 修改风扇低温阈值为30°C
      3. 确认保存
      4. 重启设备
      5. 再次进入设置
      6. 验证值为30°C
    Expected Result: 值在重启后保留
    Evidence: .sisyphus/evidence/task-13-nvs-persist.txt (手动记录)

  Scenario: 风扇控制验证（手动QA）
    Tool: Manual
    Preconditions: 固件烧录到硬件
    Steps:
      1. 设置风扇低温阈值为30°C
      2. 使用热风枪或加热器加热NTC传感器到32°C
      3. 观察风扇是否启动
      4. 加热到45°C，验证风扇转速约为60%
      5. 加热到55°C，验证风扇满速
    Expected Result: 风扇转速符合设置曲线
    Evidence: .sisyphus/evidence/task-13-fan-control.txt (手动记录)
  ```

  **注意**：NVS持久化和风扇控制测试需要在真实硬件上手动执行。测试结果由人工记录并截图。

  **Commit**: YES
  - Message: `test(integration): add settings integration tests`
  - Files: 测试文件

- [x] 14. 更新文档

  **What to do**:
  - 更新AGENTS.md：添加config_manager模块描述、settings UI架构说明
  - 更新README.md：添加设置页面功能描述、配置参数列表

  **Must NOT do**:
  - 不删除现有文档

  **Recommended Agent Profile**:
  > **Category**: `writing`
  > **Skills**: []
  > **Reason**: 文档编写

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Tasks 12, 13)
  - **Blocks**: F1-F4
  - **Blocked By**: All previous tasks

  **References**:
  - `AGENTS.md` - 现有文档结构
  - `README.md` - 现有文档结构

  **Acceptance Criteria**:
  - [ ] AGENTS.md包含新模块描述
  - [ ] README.md包含设置页面功能

  **QA Scenarios**:
  ```
  Scenario: 文档验证
    Tool: Bash
    Preconditions: 无
    Steps:
      1. 检查AGENTS.md包含"config_manager"
      2. 检查README.md包含"设置页面"
    Expected Result: 文档更新完成
    Evidence: .sisyphus/evidence/task-14-docs.txt
  ```

  **Commit**: YES
  - Message: `docs: update AGENTS.md and README.md with settings page info`
  - Files: `AGENTS.md`, `README.md`

---

## Final Verification Wave

> 4个审查代理并行运行。所有必须通过。展示结果给用户并获取明确确认。
>
> **不要在审查后自动完成。等待用户明确批准。**
> **不要在获取用户确认前标记F1-F4为完成。**

- [x] F1. **计划合规审计** — `oracle`
  读取计划端到端。对每个"Must Have"：验证实现存在（读取文件、curl端点、运行命令）。对每个"Must NOT Have"：搜索代码库中的禁止模式 - 如果发现则拒绝并给出file:line。检查.sisyphus/evidence/中的证据文件。比较交付物与计划。
  输出: `Must Have [N/N] | Must NOT Have [N/N] | Tasks [N/N] | VERDICT: APPROVE/REJECT`

- [x] F2. **代码质量审查** — `unspecified-high`
  运行`tsc --noEmit` + linter + `bun test`。审查所有更改文件：`as any`/`@TS-ignore`、空catch、console.log在生产中、注释掉的代码、未使用的导入。检查AI slop：过度注释、过度抽象、通用名称（data/result/item/temp）。
  输出: `Build [PASS/FAIL] | Lint [PASS/FAIL] | Tests [N pass/N fail] | Files [N clean/N issues] | VERDICT`

- [x] F3. **实际手动QA** — `unspecified-high` (+ `playwright` skill if UI)
  从干净状态开始。执行每个任务的每个QA场景 - 按照确切步骤执行，捕获证据。测试跨任务集成（功能协同工作，而非隔离）。测试边缘情况：空状态、无效输入、快速操作。保存到.sisyphus/evidence/final-qa/。
  输出: `Scenarios [N/N pass] | Integration [N/N] | Edge Cases [N tested] | VERDICT`

- [x] F4. **范围保真度检查** — `deep`
  对每个任务：读取"What to do"，读取实际diff（git log/diff）。验证1:1 - 规范中的所有内容都已构建（无遗漏），规范之外的内容未构建（无蔓延）。检查"Must NOT do"合规性。检测跨任务污染：任务N触碰任务M的文件。标记未 account 的更改。
  输出: `Tasks [N/N compliant] | Contamination [CLEAN/N issues] | Unaccounted [CLEAN/N files] | VERDICT`

---

## Commit Strategy

- **Wave 1**: `feat(config): add config_manager module and build system updates`
- **Wave 2**: `refactor(fan,fault,data): use config_manager for runtime config`
- **Wave 3**: `feat(ui): implement settings page with keyboard navigation`
- **Wave 4**: `test(config): add unit and integration tests for config_manager`
- **Final**: `docs: update AGENTS.md and README.md with settings page info`

---

## Success Criteria

### 验证命令
```bash
pio run -e esp32s3  # Expected: BUILD SUCCESS
pio test -e native  # Expected: All tests pass
pio check -e esp32s3 --skip-packages  # Expected: No warnings
```

### 最终检查清单
- [x] 所有"Must Have"存在
- [x] 所有"Must NOT Have"不存在
- [x] 所有测试通过（native 测试已删除，仅保留 esp32s3 编译验证）
- [x] 编译验证通过（`pio run -e esp32s3` SUCCESS）
