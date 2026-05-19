# 双击按键切换屏幕方向

## TL;DR
> **Summary**: 添加双击 K1/K3 按键切换屏幕方向的功能，支持动态旋转和 NVS 持久化
> **Deliverables**: 双击检测逻辑、屏幕方向管理模块、动态缓冲区重分配、NVS 配置保存
> **Effort**: Medium
> **Parallel**: YES - 3 waves
> **Critical Path**: 双击检测 → 方向管理 → UI 适配

## Context
### Original Request
用户希望增加双击按键切换屏幕方向的功能：
- 双击 K1（上键）：顺时针旋转屏幕
- 双击 K3（下键）：逆时针旋转屏幕
- 立即生效（动态旋转），无需重启

### Interview Summary
- 按键选择：K1 + K3（上键+下键），符合直觉
- 切换策略：动态旋转（运行时切换），实现复杂但用户体验好
- 持久化：屏幕方向保存到 NVS

### Metis Review
待补充

## Work Objectives
### Core Objective
实现双击按键动态切换屏幕方向功能，支持 4 个方向（0°/90°/180°/270°）

### Deliverables
1. 双击检测逻辑（keys.h/cpp）
2. 屏幕方向管理模块（display/screen_rotation.h/cpp）
3. 动态 LVGL 缓冲区重分配
4. NVS 持久化配置
5. 按键事件路由（input_bridge.cpp）

### Definition of Done
- 双击 K1 顺时针旋转 90°
- 双击 K3 逆时针旋转 90°
- 旋转后 UI 正常显示
- 重启后保持上次方向

### Must Have
- 双击检测（300ms 内两次短按）
- 动态旋转（无需重启）
- NVS 持久化

### Must NOT Have
- 不修改 LVGL Editor 生成文件
- 不影响现有按键功能（短按/长按）

## Verification Strategy
> ZERO HUMAN INTERVENTION - all verification is agent-executed.
- Test decision: tests-after + Unity framework
- QA policy: 每个任务有 agent-executed 场景
- Evidence: .sisyphus/evidence/task-{N}-{slug}.{ext}

## Execution Strategy
### Parallel Execution Waves

Wave 1: 基础设施（双击检测、方向管理模块）
Wave 2: 集成（缓冲区重分配、事件路由）
Wave 3: 测试与优化

### Dependency Matrix
- Task 1: 双击检测 → Task 3: 事件路由
- Task 2: 方向管理 → Task 3: 事件路由
- Task 3: 事件路由 → Task 4: NVS 持久化

## TODOs

- [ ] 1. 添加双击检测逻辑

  **What to do**:
  1. 在 `keys.h` 中添加 `KEY_DOUBLE_CLICK` 事件类型
  2. 在 `KeyState` 结构体中添加双击检测字段：
     - `last_release_time`: 上次释放时间戳
     - `click_count`: 短按计数
  3. 修改 `key_debounce_update()` 函数：
     - 释放时记录时间戳
     - 如果 300ms 内再次按下，触发 `KEY_DOUBLE_CLICK`
     - 双击事件优先于短按事件

  **Must NOT do**:
  - 不改变现有短按/长按逻辑
  - 不增加外部依赖

  **Recommended Agent Profile**:
  - Category: `quick` - 单文件改动
  - Skills: [] - 无需特殊技能

  **Parallelization**: Can Parallel: NO | Wave 1 | Blocks: [3] | Blocked By: []

  **References**:
  - Pattern: `src/input/keys.cpp:6-45` - 现有去抖逻辑
  - API/Type: `src/input/keys.h:8-12` - KeyEvent 枚举
  - Config: `include/app_config.h` - KEYS_LONGPRESS_MS 常量

  **Acceptance Criteria**:
  - [ ] `KEY_DOUBLE_CLICK` 枚举值已添加
  - [ ] 双击检测逻辑实现（300ms 窗口）
  - [ ] 现有短按/长按功能不受影响
  - [ ] 编译通过：`pio run -e esp32s3`

  **QA Scenarios**:
  ```
  Scenario: 双击检测
    Tool: interactive_bash
    Steps: 编写单元测试，模拟 300ms 内两次短按
    Expected: 触发 KEY_DOUBLE_CLICK 事件
    Evidence: .sisyphus/evidence/task-1-double-click-test.txt

  Scenario: 单击不触发双击
    Tool: interactive_bash
    Steps: 编写单元测试，模拟单次短按
    Expected: 触发 KEY_SHORT 事件，不触发 KEY_DOUBLE_CLICK
    Evidence: .sisyphus/evidence/task-1-single-click-test.txt
  ```

  **Commit**: YES | Message: `feat(input): add double-click detection for K1/K3 keys` | Files: [src/input/keys.h, src/input/keys.cpp]

---

- [ ] 2. 创建屏幕方向管理模块

  **What to do**:
  1. 创建 `src/display/screen_rotation.h`：
     - 定义 `ScreenRotation` 枚举（ROT_0, ROT_90, ROT_180, ROT_270）
     - 声明 `screen_rotation::init()`, `rotate_cw()`, `rotate_ccw()`, `get_current()`
  2. 创建 `src/display/screen_rotation.cpp`：
     - 实现方向状态管理
     - 调用 TFT_eSPI `setRotation()` 设置方向
     - 调用 LVGL `lv_display_set_rotation()` 同步方向
     - 动态重分配 LVGL 缓冲区（旋转后分辨率变化）

  **Must NOT do**:
  - 不修改 lvgl_port.cpp 的静态缓冲区定义
  - 不使用全局变量存储方向状态

  **Recommended Agent Profile**:
  - Category: `unspecified-high` - 需要理解 LVGL 缓冲区机制
  - Skills: [] - 无需特殊技能

  **Parallelization**: Can Parallel: NO | Wave 1 | Blocks: [3] | Blocked By: []

  **References**:
  - Pattern: `src/display/tft_driver.cpp:28` - setRotation 调用
  - API/Type: `src/display/lvgl_port.cpp:19-20` - 静态缓冲区定义
  - External: LVGL 9.x lv_display_set_rotation() API

  **Acceptance Criteria**:
  - [ ] `screen_rotation.h/cpp` 文件创建
  - [ ] 实现顺时针/逆时针旋转函数
  - [ ] 动态缓冲区重分配实现
  - [ ] 编译通过：`pio run -e esp32s3`

  **QA Scenarios**:
  ```
  Scenario: 顺时针旋转
    Tool: interactive_bash
    Steps: 调用 rotate_cw() 4 次，检查方向变化
    Expected: 0→90→180→270→0 循环
    Evidence: .sisyphus/evidence/task-2-rotate-cw-test.txt

  Scenario: 缓冲区重分配
    Tool: interactive_bash
    Steps: 旋转到 90°，检查缓冲区大小
    Expected: 缓冲区大小为 280×28 像素
    Evidence: .sisyphus/evidence/task-2-buffer-realloc-test.txt
  ```

  **Commit**: YES | Message: `feat(display): add screen rotation manager with dynamic buffer reallocation` | Files: [src/display/screen_rotation.h, src/display/screen_rotation.cpp]

---

- [ ] 3. 集成按键事件路由

  **What to do**:
  1. 修改 `src/ui_bridge/input_bridge.cpp`：
     - 在 `input_handle_key()` 中添加 `KEY_DOUBLE_CLICK` 处理
     - 根据按键来源（K1/K3）调用 `screen_rotation::rotate_cw()` 或 `rotate_ccw()`
     - 使用 `lv_async_call` 确保线程安全
  2. 修改 `src/ui_bridge/input_bridge.h`：
     - 声明新的事件处理函数（如需要）

  **Must NOT do**:
  - 不改变现有按键路由逻辑
  - 不在按键任务中直接调用 LVGL API

  **Recommended Agent Profile**:
  - Category: `quick` - 单文件改动
  - Skills: [] - 无需特殊技能

  **Parallelization**: Can Parallel: NO | Wave 2 | Blocks: [4] | Blocked By: [1, 2]

  **References**:
  - Pattern: `src/ui_bridge/input_bridge.cpp:29-62` - 现有按键路由逻辑
  - API/Type: `src/input/keys.h:8-12` - KeyEvent 枚举
  - Pattern: `src/ui_bridge/input_bridge.cpp:34` - lv_async_call 使用

  **Acceptance Criteria**:
  - [ ] `KEY_DOUBLE_CLICK` 事件处理已添加
  - [ ] K1 双击调用 `rotate_cw()`
  - [ ] K3 双击调用 `rotate_ccw()`
  - [ ] 使用 `lv_async_call` 确保线程安全
  - [ ] 编译通过：`pio run -e esp32s3`

  **QA Scenarios**:
  ```
  Scenario: K1 双击触发顺时针旋转
    Tool: interactive_bash
    Steps: 模拟 K1 双击事件，检查旋转调用
    Expected: 调用 screen_rotation::rotate_cw()
    Evidence: .sisyphus/evidence/task-3-k1-double-click-test.txt

  Scenario: K3 双击触发逆时针旋转
    Tool: interactive_bash
    Steps: 模拟 K3 双击事件，检查旋转调用
    Expected: 调用 screen_rotation::rotate_ccw()
    Evidence: .sisyphus/evidence/task-3-k3-double-click-test.txt
  ```

  **Commit**: YES | Message: `feat(ui_bridge): route double-click events to screen rotation` | Files: [src/ui_bridge/input_bridge.cpp, src/ui_bridge/input_bridge.h]

---

- [ ] 4. 添加 NVS 持久化

  **What to do**:
  1. 修改 `src/app/config_manager.cpp`：
     - 添加屏幕方向配置项 `screen_rotation`
     - 在 `init()` 中加载方向配置
     - 提供 `save_rotation()` 函数
  2. 修改 `src/app/config_manager.h`：
     - 声明 `get_rotation()`, `save_rotation()` 函数
  3. 在 `screen_rotation.cpp` 中：
     - 初始化时从 NVS 加载方向
     - 旋转变化时保存到 NVS

  **Must NOT do**:
  - 不修改 NVS 分区表
  - 不使用额外的 NVS namespace

  **Recommended Agent Profile**:
  - Category: `quick` - 单文件改动
  - Skills: [] - 无需特殊技能

  **Parallelization**: Can Parallel: NO | Wave 2 | Blocks: [] | Blocked By: [3]

  **References**:
  - Pattern: `src/app/config_manager.cpp` - 现有 NVS 配置管理
  - API/Type: `src/app/config_manager.h` - 配置管理接口
  - External: ESP-IDF NVS API

  **Acceptance Criteria**:
  - [ ] `screen_rotation` 配置项已添加
  - [ ] 启动时从 NVS 加载方向
  - [ ] 旋转变化时保存到 NVS
  - [ ] 编译通过：`pio run -e esp32s3`

  **QA Scenarios**:
  ```
  Scenario: NVS 保存方向
    Tool: interactive_bash
    Steps: 旋转屏幕，重启，检查方向
    Expected: 重启后保持旋转后的方向
    Evidence: .sisyphus/evidence/task-4-nvs-persist-test.txt
  ```

  **Commit**: YES | Message: `feat(config): add screen rotation NVS persistence` | Files: [src/app/config_manager.cpp, src/app/config_manager.h]

---

- [ ] 5. UI 重绘优化

  **What to do**:
  1. 在 `screen_rotation.cpp` 中：
     - 旋转后调用 `lv_obj_invalidate(lv_screen_active())` 强制重绘
     - 清空屏幕避免残影
  2. 在 `data_bridge.cpp` 中：
     - 检测方向变化，更新 UI 布局（如需要）
  3. 测试各方向下 UI 显示正常

  **Must NOT do**:
  - 不修改 LVGL Editor 生成文件
  - 不改变 UI 控件结构

  **Recommended Agent Profile**:
  - Category: `unspecified-high` - 需要理解 LVGL 重绘机制
  - Skills: [] - 无需特殊技能

  **Parallelization**: Can Parallel: NO | Wave 3 | Blocks: [] | Blocked By: [3, 4]

  **References**:
  - Pattern: `src/ui_bridge/data_bridge.cpp` - UI 数据刷新逻辑
  - API/Type: LVGL 9.x lv_obj_invalidate() API

  **Acceptance Criteria**:
  - [ ] 旋转后 UI 正确重绘
  - [ ] 无残影或错位
  - [ ] 各方向下文字可读
  - [ ] 编译通过：`pio run -e esp32s3`

  **QA Scenarios**:
  ```
  Scenario: 旋转后 UI 重绘
    Tool: interactive_bash
    Steps: 旋转屏幕，检查 UI 是否正确显示
    Expected: UI 元素位置正确，无残影
    Evidence: .sisyphus/evidence/task-5-ui-redraw-test.txt
  ```

  **Commit**: YES | Message: `fix(ui): optimize UI redraw after screen rotation` | Files: [src/display/screen_rotation.cpp, src/ui_bridge/data_bridge.cpp]

---

- [ ] 6. 单元测试

  **What to do**:
  1. 在 `src/test_main.cpp` 中添加双击检测测试：
     - 测试 300ms 内双击触发
     - 测试超过 300ms 不触发
     - 测试单击不触发双击
  2. 添加屏幕方向管理测试：
     - 测试顺时针/逆时针旋转
     - 测试方向循环（0→90→180→270→0）

  **Must NOT do**:
  - 不修改现有测试用例
  - 不添加硬件依赖的测试

  **Recommended Agent Profile**:
  - Category: `quick` - 测试代码编写
  - Skills: [] - 无需特殊技能

  **Parallelization**: Can Parallel: NO | Wave 3 | Blocks: [] | Blocked By: [1, 2]

  **References**:
  - Pattern: `src/test_main.cpp` - 现有测试结构
  - API/Type: Unity 框架 API

  **Acceptance Criteria**:
  - [ ] 双击检测测试用例添加
  - [ ] 屏幕方向管理测试用例添加
  - [ ] 所有测试通过：`pio test -e esp32s3`

  **QA Scenarios**:
  ```
  Scenario: 运行单元测试
    Tool: interactive_bash
    Steps: 执行 pio test -e esp32s3
    Expected: 所有测试通过
    Evidence: .sisyphus/evidence/task-6-unit-test.txt
  ```

  **Commit**: YES | Message: `test: add unit tests for double-click and screen rotation` | Files: [src/test_main.cpp]

## Final Verification Wave (MANDATORY)
- [ ] F1. Plan Compliance Audit — oracle
- [ ] F2. Code Quality Review — unspecified-high
- [ ] F3. Real Manual QA — unspecified-high
- [ ] F4. Scope Fidelity Check — deep

## Commit Strategy
每个任务完成后提交，使用约定的 commit message 格式

## Success Criteria
1. 双击 K1 顺时针旋转 90°
2. 双击 K3 逆时针旋转 90°
3. 旋转后 UI 正常显示
4. 重启后保持上次方向
5. 所有单元测试通过
6. 编译无警告：`pio run -e esp32s3`
