# Init ESP32-WROOM-32E-N8 Fan Controller — 全量交付工作计划

## TL;DR

> **Quick Summary**：基于 OpenSpec 提案 `init-esp32-fan-controller` 实施全量交付，从零搭建 PlatformIO + Arduino + ESP32 风扇控制器固件，覆盖 8 个 capability（工程骨架/显示/UI/温度/风扇/电源监测/电源开关/按键），双 env 测试架构（native Unity + esp32dev 编译验证），无硬件实测前提下确保编译通过、单元测试覆盖核心算法、二进制大小可控。
>
> **Deliverables**：
> - `platformio.ini` + `partitions/default_8MB.csv` + `.gitignore` + `.editorconfig` + `README.md`
> - `include/{pins.h, app_config.h, lv_conf.h}` 单点配置
> - `src/{hal, display, ui, sensors/ntc, sensors/ina226, fan, power, input, app}/` 模块化分层源码
> - `test/native/` Unity 单元测试（β 公式、温控曲线、电源状态机、按键去抖、温控回滞）
> - `pio run -e esp32dev` 与 `pio test -e native` 全部通过
> - 二进制大小报告 + LVGL/TFT_eSPI 集成验证报告
>
> **Estimated Effort**：XL（约 50 个原子 TODO，5 波次执行）
> **Parallel Execution**：YES — 5 波次（Wave 1 7 任务、Wave 2 12 任务、Wave 3 14 任务、Wave 4 8 任务、Wave Final 4 审核任务）
> **Critical Path**：T01 platformio.ini → T05 lv_conf.h → T13 lvgl_port → T34 tasks.cpp → T42 main.cpp 集成 → F1-F4 审核

---

## Context

### Original Request
用户要求基于 `openspec/changes/init-esp32-fan-controller/` 文档生成完整执行计划并启用高精度审核。

### Interview Summary
**用户决策**：
- 范围：**全量交付**（覆盖 tasks.md 全部 10 章节）
- 硬件：**无硬件 — 仅编译验证**（执行 agent 不接触真板，仅 `pio run` + 静态分析 + native 测试）
- 测试：**完整 native + embedded 双环境**（PlatformIO 双 env：`native` 跑 Unity 工具函数测试，`esp32dev` 仅编译）
- 粒度：**最细粒度**（每任务 1-3 文件，最大化并行）

### Research Findings（基于 design.md/spec 文档交叉审查）
- **F1 PCNT API**：`espressif32@^6.7.0` → `arduino-esp32@2.0.14` → 必须使用 legacy `driver/pcnt.h`（v4.x 之前），不能用新版 PCNT API
- **F2 LVGL 配置发现**：LVGL 9.x 需要 `LV_CONF_INCLUDE_SIMPLE=1` + 编译器 include path 包含 `include/` 才能找到 `lv_conf.h`；TFT_eSPI 用 build_flags 注入需 `USER_SETUP_LOADED=1` 阻止库内默认配置生效
- **F3 双缓冲尺寸校验**：240×28×2B = 13,440 字节/缓冲，双缓冲 26.88KB，加 LVGL 内部 buffer，512KB SRAM 余量充足
- **F4 partitions/default_8MB.csv**：Arduino-ESP32 自带 `default_8MB.csv` 在 `tools/partitions/`，可直接复用或自定义
- **F5 ADC1 衰减选择**：`ADC_11db` 衰减下 ESP32 ADC 满量程约 3.1V，NTC 分压用 3.3V 时安全
- **F6 TFT_eSPI 库本地化**：库内 `User_Setup_Select.h` 默认 `#include <User_Setup.h>`，需通过 `USER_SETUP_LOADED=1` 阻断，否则 build_flags 不生效

### 自检识别的隐藏风险
- **R1**：design.md 提及目录 `src/display/{tft_driver, lvgl_port}.{h,cpp}`，但 tasks.md 写 `src/display/display.{h,cpp}` —— 采用 design.md D10 的细分结构
- **R2**：tasks 9.3/9.4/9.5（烧录/联调/长跑）在无硬件下不可执行 → 改写为「编译产物 ELF 符号检查 + 二进制大小预算 + native 状态机模拟」
- **R3**：未规定看门狗（Task WDT/Interrupt WDT）策略 → 在 app_config.h 中显式定义
- **R4**：未规定任务栈深度统一管理 → 在 app_config.h 中集中定义
- **R5**：spec 中「Wave 协议、ISR 安全」未明确 → 在每个 ISR 任务中显式注明 IRAM_ATTR 与 portYIELD_FROM_ISR

---

## Work Objectives

### Core Objective
建立可一次 `pio run -e esp32dev` 编译通过、`pio test -e native` 单元测试全绿、模块化分层清晰、引脚单点定义、双缓冲 LVGL 9.x 已正确移植的 ESP32 风扇控制器固件骨架，覆盖 8 个 capability 的全部业务逻辑。

### Concrete Deliverables
> 在 design.md D10 基础上做轻微模块化拆分：状态机与 GPIO 控制分离为 `psu_fsm`/`ps_on`，PCNT 转速独立为 `fan_tach`，故障/看门狗从 `tasks` 析出为独立单元；以下即 F4 范围审计的唯一权威清单。
- `platformio.ini`（含 `[env:esp32dev]` + `[env:native]`，build_flags 注入 LVGL/TFT_eSPI 配置）
- `partitions/default_8MB.csv`
- `.gitignore`、`.editorconfig`、`README.md`
- `include/pins.h`、`include/app_config.h`、`include/lv_conf.h`
- `src/hal/{i2c_bus, spi_bus, ledc}.{h,cpp}`
- `src/display/{tft_driver, lvgl_port}.{h,cpp}`
- `src/ui/{ui_main, ui_menu, ui_events, ui_styles}.{h,cpp}`
- `src/sensors/ntc/ntc.{h,cpp}`
- `src/sensors/ina226/ina226.{h,cpp}`
- `src/fan/{fan_curve, fan_pwm, fan_tach}.{h,cpp}`
- `src/power/{psu_fsm, ps_on}.{h,cpp}`
- `src/input/keys.{h,cpp}`
- `src/app/{events, tasks, app_state, watchdog, fault_guard}.{h,cpp}`
- `src/main.cpp`（正式入口，位于 src/ 根目录；Wave 1 先放占位，Wave 5 重写）
- `test/native/{test_ntc_beta, test_fan_curve, test_psu_fsm, test_keys_debounce, test_hysteresis}/test_main.cpp`

> **关于 design.md D10 中的辅助文件归并说明（F4 审计权威）**：
> - `src/hal/ledc.{h,cpp}` 已合并到 T22 `src/fan/fan_pwm.{h,cpp}`（LEDC 仅风扇 PWM 使用，无独立模块化必要）
> - `src/ui/ui_styles.{h,cpp}` 已合并到 T30 `src/ui/ui_main.{h,cpp}`（样式与主屏强耦合，单文件管理更清晰）
> - `src/app/events.h` 已合并到 T33 `src/app/app_state.h`（事件枚举与状态结构同模块定义）
> 这 3 个文件不会以 design.md D10 列出的独立路径出现，但其职责完整覆盖于上述合并后的模块中。F4 审计应据此判定为"职责达成"，而非"文件缺失"。

### Definition of Done
- [ ] `pio run -e esp32dev` 退出码 0，无 error/warning（关键警告）
- [ ] `pio test -e native` 所有测试 PASS
- [ ] `pio check -e esp32dev --skip-packages` 静态分析无 high severity issue
- [ ] 二进制大小：固件 .bin < 1.5MB，DRAM 余量 ≥ 100KB（`pio run` 报告）
- [ ] `openspec validate init-esp32-fan-controller --strict` 通过
- [ ] 4 个并行 Final 审核 agent 全部 APPROVE

### Must Have
- ESP32-WROOM-32E 引脚约束零违反（GPIO6-11/strapping/ADC2 在 WiFi 启用时禁用 — 当前未启用 WiFi 但仍按规范）
- TFT_eSPI 通过 build_flags 完整配置，无依赖库内 User_Setup.h
- LVGL 9.x 通过 `include/lv_conf.h` 配置，`LV_CONF_INCLUDE_SIMPLE=1` 注入
- 所有 GPIO 在 `include/pins.h` 单点定义，禁止源码中硬编码
- 所有可调常量（NTC 参数/温控阈值/I2C 地址/按键时间/任务栈/优先级/看门狗超时）集中在 `include/app_config.h`
- HAL 层（驱动）与 Service 层（业务）严格分离，业务逻辑可在 native env 测试
- `volatile` + `std::atomic` 跨任务通信，无 race condition
- 5 个 FreeRTOS 任务严格按 design.md D9 绑核与优先级
- 看门狗（Task WDT 5s）启用，所有任务定期 `esp_task_wdt_reset()`

### Must NOT Have（守门栏）
- ❌ 不实现 WiFi/BLE/OTA/MQTT
- ❌ 不实现 NVS 持久化（保留接口但不接入）
- ❌ 不引入中文字体（节省 flash）
- ❌ 不使用 PID（仅分段线性温控）
- ❌ 源码中硬编码 GPIO（必须用 `pins.h` 宏）
- ❌ 源码中硬编码可调参数（必须用 `app_config.h`）
- ❌ TFT_eSPI 不通过 `lib/TFT_eSPI/User_Setup.h` 配置（必须 build_flags）
- ❌ 不创建 lib/TFT_eSPI/ 目录覆盖物（除非有 User_Setup 兼容包装）
- ❌ 不在 ISR 中调用 `Serial.print` 或 LVGL API
- ❌ 不在 LVGL flush_cb 中分配堆内存
- ❌ 不在 native env 包含任何 Arduino/ESP32 头文件
- ❌ 不允许任何「需要人手动验证」的验收标准（QA 必须 agent 可执行）

---

## Verification Strategy

> **ZERO HUMAN INTERVENTION** — 在「无硬件」约束下，所有验证均通过编译/静态分析/native 单元测试完成。

### Test Decision
- **Infrastructure exists**: NO（新建项目）
- **Automated tests**: YES（TDD for service 层 + 编译验证 for HAL/驱动层）
- **Framework**: PlatformIO Unity（双 env：`native` 跑 service 层、`esp32dev` 仅 build）
- **TDD**: 所有 service 层任务（β 公式、温控曲线、状态机、去抖逻辑、回滞）先写 RED 测试，再实现 GREEN

### QA Policy
每个任务必须有 agent 可执行的 QA 场景，分两类：

**HAL/驱动层任务**（涉及硬件 API）：
- QA 工具：`pio run -e esp32dev` + ELF 符号检查（`xtensa-esp32-elf-nm`）+ `arduino-cli compile --warnings all`
- 评估：编译退出码 == 0、关键 symbol 存在、无 multiple definition

**Service 层任务**（纯业务逻辑）：
- QA 工具：`pio test -e native --filter <test_name>` Unity 框架
- 评估：所有断言通过、覆盖正常路径 + 边界 + 异常

**集成层任务**：
- QA 工具：`pio run -e esp32dev` + `pio run -e esp32dev -t size`（二进制大小检查）+ `pio check -e esp32dev`
- 评估：固件 .bin < 1.5MB、DRAM 余量 ≥ 100KB、静态分析无 high severity issue

证据保存到 `.sisyphus/evidence/task-{N}-{scenario-slug}.{txt|log}`。

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (立即开始 — 工程骨架与配置基线，7 任务并行):
├── T01: 创建 platformio.ini（双 env 骨架）[quick]
├── T02: 创建 partitions/default_8MB.csv [quick]
├── T03: 创建 .gitignore + .editorconfig [quick]
├── T04: 创建 include/pins.h（按 D2 引脚表）[quick]
├── T05: 创建 include/app_config.h（NTC/温控/I2C/按键/任务栈/WDT）[quick]
├── T06: 创建目录骨架 src/{hal,display,ui,sensors,fan,power,input,app}/、test/native/、lib/ [quick]
└── T07: 创建占位 src/main.cpp（仅 setup/loop + Serial）[quick]

Wave 2 (依赖 Wave 1 — 配置与依赖完整化 + service 层 RED 测试，12 任务并行):
├── T08: platformio.ini 注入 lib_deps（TFT_eSPI/lvgl/INA226）[quick]
├── T09: platformio.ini 注入 TFT_eSPI build_flags（D3 全套）[quick]
├── T10: platformio.ini 注入 LVGL build_flags（LV_CONF_INCLUDE_SIMPLE 等）[quick]
├── T11: 创建 include/lv_conf.h（颜色深度/字体/双缓冲/PARTIAL）[unspecified-high]
├── T12: 编写 test_ntc_beta RED 测试（β 公式输入输出表）[quick]
├── T13: 编写 test_fan_curve RED 测试（温控曲线分段 + 回滞）[quick]
├── T14: 编写 test_psu_fsm RED 测试（电源状态机迁移表）[quick]
├── T15: 编写 test_keys_debounce RED 测试（去抖 + 短按/长按时序）[quick]
├── T16: 编写 test_hysteresis RED 测试（温控回滞独立单元）[quick]
├── T17: 创建 README.md（硬件/引脚/构建/测试命令）[writing]
├── T18: 创建 src/hal/i2c_bus.{h,cpp}（Wire 初始化封装）[quick]
└── T19: 创建 src/hal/spi_bus.{h,cpp}（VSPI 初始化封装）[quick]

Wave 3 (依赖 Wave 2 — Service 层 GREEN 实现 + 驱动层创建，14 任务并行):
├── T20: 实现 src/sensors/ntc/ntc.{h,cpp}（β 公式 + Fault 检测，GREEN T12）[deep]
├── T21: 实现 src/fan/fan_curve.{h,cpp}（温控曲线 + 回滞，GREEN T13+T16）[deep]
├── T22: 实现 src/fan/fan_pwm.{h,cpp}（25kHz LEDC 输出封装）[deep]
├── T23: 实现 src/fan/fan_tach.{h,cpp}（PCNT legacy API GPIO35 计数）[deep]
├── T24: 实现 src/sensors/ina226/ina226.{h,cpp}（3 实例错峰 I2C 采样）[deep]
├── T25: 实现 src/power/psu_fsm.{h,cpp}（电源状态机，GREEN T14）[deep]
├── T26: 实现 src/power/ps_on.{h,cpp}（PS_ON/PW_OK GPIO 封装）[quick]
├── T27: 实现 src/input/keys.{h,cpp}（去抖逻辑 + 短按/长按，GREEN T15）[deep]
├── T28: 实现 src/display/tft_driver.{h,cpp}（TFT_eSPI 单例封装）[unspecified-high]
├── T29: 实现 src/display/lvgl_port.{h,cpp}（flush_cb + esp_timer 1ms tick）[unspecified-high]
├── T30: 实现 src/ui/ui_main.{h,cpp}（主仪表盘 4 区域容器）[visual-engineering]
├── T31: 实现 src/ui/ui_menu.{h,cpp}（菜单界面）[visual-engineering]
├── T32: 实现 src/ui/ui_events.{h,cpp}（KeyEvent → UI 切换映射）[visual-engineering]
└── T33: 实现 src/app/app_state.{h,cpp}（全局共享 atomic 数据 + events.h 结构）[quick]

Wave 4 (依赖 Wave 3 — FreeRTOS 任务 + 故障保护，8 任务并行):
├── T34: 实现 src/app/tasks.{h,cpp}（5 任务创建函数 + 绑核 + WDT 注册）[deep]
├── T35: 实现 src/app/watchdog.{h,cpp}（Task WDT 初始化 + 注册/喂狗 API）[quick]
├── T36: 实现 src/app/fault_guard.{h,cpp}（温度/风扇/INA 过流 → power::off 规则集）[deep]
├── T37: sensorTask 业务实现（核0，200ms，NTC + INA226 错峰，写 app_state）[quick]
├── T38: controlTask 业务实现（核0，500ms，调用 fan_curve + 设 PWM + fault_guard）[quick]
├── T39: inputTask 业务实现（核0，5ms，按键扫描 + 事件队列分发）[quick]
├── T40: powerTask 业务实现（核0，事件驱动，psu_fsm 驱动 ps_on）[quick]
└── T41: lvglTask 业务实现（核1，5ms，lv_timer_handler + UI 数据刷新）[visual-engineering]

Wave 5 (依赖 Wave 4 — 主程序与最终编译，3 任务串行):
├── T42: 重写 src/app/main.cpp 正式入口（init 顺序 + tasks::start + WDT 启用）[deep]
├── T43: pio run -e esp32dev 完整编译 + 二进制大小报告 + ELF 符号检查 [quick]
└── T44: pio test -e native 全套 + pio check -e esp32dev 静态分析 + openspec validate [quick]

Wave Final (依赖 Wave 5 — 4 并行审核):
├── F1: 计划合规性审计（plan vs delivered）[oracle]
├── F2: 代码质量审计（AI slop / type safety / 静态分析）[unspecified-high]
├── F3: 编译产物 QA（pio run/test/check + size + ELF symbols）[unspecified-high]
└── F4: 范围保真度审计（spec → 实现一一对应，无 scope creep）[deep]
→ 4 全部 APPROVE → 呈报用户 → 用户 OKAY → 完成
```

### Dependency Matrix

> 本表与 TODO 详细区 T01-T44 一一对齐。T20=ntc、T21=fan_curve、T22=fan_pwm、T23=fan_tach、T24=ina226、T25=psu_fsm、T26=ps_on、T27=keys、T28=tft_driver、T29=lvgl_port、T30=ui_main、T31=ui_menu、T32=ui_events、T33=app_state、T34=tasks、T35=watchdog、T36=fault_guard、T37=sensorTask、T38=controlTask、T39=inputTask、T40=powerTask、T41=lvglTask、T42=main.cpp、T43=pio run+size、T44=pio test+check+openspec validate。

| Task | Depends On | Blocks |
|---|---|---|
| T01-T07 | — | T08-T19 |
| T08 | T01 | T28 |
| T09 | T01, T04 | T28 |
| T10 | T01 | T11, T29 |
| T11 | T10 | T29 |
| T12 | T01, T06 | T20 |
| T13 | T01, T06 | T21 |
| T14 | T01, T06 | T25 |
| T15 | T01, T06 | T27 |
| T16 | T01, T06 | T21 |
| T17 | T01-T07 | F1 |
| T18 | T04, T06 | T24, T28 |
| T19 | T04, T06 | T23 |
| T20 (ntc) | T12, T05 | T37, T44 |
| T21 (fan_curve) | T13, T16, T05 | T38, T44 |
| T22 (fan_pwm) | T04, T05 | T38, T36 |
| T23 (fan_tach) | T04, T19 | T37 |
| T24 (ina226) | T18, T05 | T37, T36 |
| T25 (psu_fsm) | T14, T05 | T40, T44 |
| T26 (ps_on) | T04 | T40 |
| T27 (keys) | T15, T05 | T39, T44 |
| T28 (tft_driver) | T08, T09, T18 | T29, T41 |
| T29 (lvgl_port) | T10, T11, T28 | T30, T31, T41 |
| T30 (ui_main) | T29, T33 | T32, T41 |
| T31 (ui_menu) | T29, T33 | T32, T41 |
| T32 (ui_events) | T30, T31, T33 | T39 |
| T33 (app_state) | T05, T06 | T34, T37-T40 |
| T34 (tasks) | T33, T35 | T37-T41 |
| T35 (watchdog) | T05 | T34 |
| T36 (fault_guard) | T33, T22, T24 | T38 |
| T37 (sensorTask) | T34, T20, T23, T24 | T42 |
| T38 (controlTask) | T34, T21, T22, T36 | T42 |
| T39 (inputTask) | T34, T27, T32 | T42 |
| T40 (powerTask) | T34, T25, T26 | T42 |
| T41 (lvglTask) | T34, T28, T29, T30, T31 | T42 |
| T42 (main.cpp) | T37-T41 | T43 |
| T43 (pio run+size) | T42 | F1-F4 |
| T44 (pio test+check+openspec validate) | T42 | F1-F4 |
| F1-F4 | T43, T44 | — |

### Agent Dispatch Summary

- **Wave 1**：7 任务 × `quick`
- **Wave 2**：12 任务 — T08/T09/T10/T12-T16/T18/T19 × `quick`（10）、T11 lv_conf × `unspecified-high`（1）、T17 README × `writing`（1）
- **Wave 3**：14 任务 — T20(ntc)/T21(fan_curve)/T25(psu_fsm)/T24(ina226)/T29(lvgl_port) × `deep`（5）、T28(tft_driver) × `unspecified-high`（1）、T30(ui_main)/T31(ui_menu)/T32(ui_events) × `visual-engineering`（3）、T22/T23/T26/T27/T33 × `quick`（5）
- **Wave 4**：8 任务 — T34(tasks)/T41(lvglTask) × `deep`（2）、T37(sensorTask)/T38(controlTask)/T40(powerTask) × `unspecified-high`（3）、T35(watchdog)/T36(fault_guard)/T39(inputTask) × `quick`（3）
- **Wave 5**：3 任务 × `quick`（T42/T43/T44）
- **Wave Final**：F1 × `oracle`、F2/F3 × `unspecified-high`、F4 × `deep`

---

## TODOs

### Wave 1 — 工程骨架与配置基线

- [x] **T01. 创建 platformio.ini 骨架（双 env）** — `quick`
  **Depends on**: 无
  **Files**: `platformio.ini`
  **What to do**:
  - 创建 `[platformio]` default_envs = esp32dev
  - `[env:esp32dev]`: platform=espressif32@^6.7.0, board=esp32dev, framework=arduino, monitor_speed=115200, upload_speed=921600, board_build.partitions=partitions/default_8MB.csv, board_build.flash_size=8MB, board_upload.flash_size=8MB
  - `[env:native]`: platform=native, test_framework=unity, build_flags=-std=c++17 -Iinclude
  - 占位 `lib_deps=` 与 `build_flags=`（在 T08-T10 填充）
  - `check_tool=cppcheck`, `check_flags=cppcheck: --enable=warning,style,performance --inconclusive --std=c++17`
  **Must NOT**: 不填 WiFi/BLE 相关配置；不启用 CORE_DEBUG_LEVEL >= 3（减少日志 flash 占用）
  **References**:
  - `openspec/changes/init-esp32-fan-controller/design.md:D1` — 平台选择
  - PlatformIO 文档：env 配置、board_build.partitions
  **Acceptance Criteria**:
  - [ ] 文件存在且语法合法：`pio project config` 退出码 0
  - [ ] `pio project config` 输出包含 `esp32dev` 与 `native` 两个 env
  **QA Scenarios**:
  ```
  Scenario: platformio.ini 合法且 env 齐全
    Tool: Bash (pio)
    Steps:
      1. 执行 `pio project config --json-output > .sisyphus/evidence/task-01-config.json`
      2. `rg '"esp32dev"' .sisyphus/evidence/task-01-config.json` 必须命中
      3. `rg '"native"' .sisyphus/evidence/task-01-config.json` 必须命中
    Expected Result: 两个 env 均存在；pio project config 退出码 0
    Evidence: .sisyphus/evidence/task-01-config.json
  ```

- [x] **T02. 创建 partitions/default_8MB.csv** — `quick`
  **Depends on**: 无
  **Files**: `partitions/default_8MB.csv`
  **What to do**:
  - 复制 arduino-esp32 官方 default_8MB.csv 内容：nvs(20KB)/otadata(8KB)/app0(3.3MB)/app1(3.3MB)/spiffs(1.37MB)/coredump(64KB)
  - 确认 app0/app1 起始地址对齐 0x10000
  **References**:
  - arduino-esp32 `tools/partitions/default_8MB.csv`
  **Acceptance Criteria**:
  - [ ] 文件存在，首行为 `# Name, Type, SubType, Offset, Size, Flags`
  - [ ] 至少包含 nvs/otadata/app0/app1/spiffs 5 个分区
  **QA Scenarios**:
  ```
  Scenario: 分区表内容完整
    Tool: Bash (ripgrep)
    Steps:
      1. `rg -c "^(nvs|otadata|app0|app1|spiffs)," partitions/default_8MB.csv > .sisyphus/evidence/task-02-parts.txt`
      2. 内容必须 == 5
    Expected Result: 5 个关键分区全部命中
    Evidence: .sisyphus/evidence/task-02-parts.txt
  ```

- [x] **T03. 创建 .gitignore 与 .editorconfig** — `quick`
  **Depends on**: 无
  **Files**: `.gitignore`, `.editorconfig`
  **What to do**:
  - `.gitignore`：`.pio/`、`.vscode/`、`build/`、`*.bin`、`*.elf`、`*.map`、`.sisyphus/evidence/`（evidence 本地生成不入库）
  - `.editorconfig`：charset=utf-8, indent_style=space, indent_size=2, end_of_line=lf, insert_final_newline=true；对 `*.{c,cpp,h,hpp}` 设 indent_size=4
  **Acceptance Criteria**:
  - [ ] 两个文件均存在
  **QA Scenarios**:
  ```
  Scenario: 忽略规则与风格配置正确
    Tool: Bash (ripgrep)
    Steps:
      1. `rg -q "\.pio/" .gitignore` 必须命中
      2. `rg -q "indent_style" .editorconfig` 必须命中
    Expected Result: 两个 rg 退出码 0
    Evidence: .sisyphus/evidence/task-03-check.txt（输出 "OK"）
  ```

- [x] **T04. 创建 include/pins.h（D2 引脚表单点定义）** — `quick`
  **Depends on**: 无
  **Files**: `include/pins.h`
  **What to do**:
  - 头部 `#pragma once`，按 design.md D2 表逐条定义宏：
    - TFT_MOSI=23, TFT_SCLK=18, TFT_CS=5, TFT_DC=2, TFT_RST=4, TFT_BL=16
    - I2C_SDA=21, I2C_SCL=22
    - INA_LOAD_ADDR=0x40, INA_12V_ADDR=0x41, INA_5V_ADDR=0x44
    - FAN_PWM=25, FAN_TACH=35（input only）
    - NTC_ADC_CH=36（SENSOR_VP）
    - PSON_PIN=27（输出）、PWOK_PIN=34（input only）
    - KEY_UP=32, KEY_ENTER=33, KEY_DOWN=26
  - 尾部 `static_assert`（如可）或注释声明避免 GPIO6-11、GPIO34-39 输出用途
  **Must NOT**: 不定义业务参数（温控阈值、β 值）— 那是 T05 的职责
  **References**:
  - `openspec/changes/init-esp32-fan-controller/design.md:D2` 表
  **Acceptance Criteria**:
  - [ ] 包含 ≥ 15 个宏定义
  - [ ] 存在 `#pragma once`
  **QA Scenarios**:
  ```
  Scenario: 全部引脚宏存在且唯一
    Tool: Bash (ripgrep)
    Steps:
      1. `rg -c "^#define\s+(TFT_|I2C_|INA_|FAN_|NTC_|PSON_|PWOK_|KEY_)" include/pins.h > .sisyphus/evidence/task-04-count.txt`
      2. 值 ≥ 15
      3. `rg "^#pragma once" include/pins.h` 必须命中
    Expected Result: 15 个以上宏 + pragma once 存在
    Evidence: .sisyphus/evidence/task-04-count.txt
  ```

- [x] 5. **T05 创建 include/app_config.h 全局常量**

  **Depends on**: None（可并行）
  **Files**: `include/app_config.h`（新建）

  **What to do**（严格对齐 design.md D4-D7 与 specs/temperature-sensing、specs/fan-control、specs/power-rail-monitoring）:
  - 定义 NTC（**spec 命名**）：`NTC_R25_OHM=10000`、`NTC_B_VALUE=3950`、`NTC_PULLUP_OHM=10000`、`NTC_T0_K=298.15f`、`NTC_VREF=3.3f`、`NTC_ADC_MAX=4095`、`NTC_SAMPLES=16`、`NTC_TEMP_SHORT_C=150.0f`、`NTC_TEMP_OPEN_C=-40.0f`
  - 定义风扇（**10-bit PWM，对齐 design.md D4-D5**）：`FAN_PWM_FREQ_HZ=25000`、`FAN_PWM_RES_BITS=10`、`FAN_LEDC_CH=0`、`FAN_PWM_MIN=205`（20%）、`FAN_PWM_MAX=1023`、`FAN_TEMP_LOW=30.0f`、`FAN_TEMP_MID=50.0f`、`FAN_TEMP_HIGH=70.0f`、`FAN_TEMP_FORCE=75.0f`、`FAN_HYSTERESIS=2.0f`、`FAN_STALL_DUTY_THRESH=307`（30% of 1023）、`FAN_STALL_TIMEOUT_MS=3000`、`FAN_FAULT_SHUTDOWN_MS=5000`
  - 定义 I2C/INA226（**3 路 + 40A/2mΩ，对齐 design.md D6**）：`I2C_FREQ_HZ=400000`、`INA226_ADDR_RAIL[3]={0x40,0x41,0x44}`、`INA226_SHUNT_OHMS=0.002f`、`INA226_MAX_CURRENT_A=40.0f`、`INA226_POLL_PERIOD_MS=750`（每路 250ms 轮询）
  - 定义按键（对齐 design.md D8）：`KEYS_DEBOUNCE_MS=5`、`KEYS_LONGPRESS_MS=800`、`KEYS_POLL_MS=5`、`PSU_LONGPRESS_MS=2000`（K1 关机长按）
  - 定义 PSU 时序（对齐 design.md D7）：`PSU_START_TIMEOUT_MS=1000`、`PSU_PWOK_LOST_MS=100`
  - 定义任务：`TASK_STACK_LVGL=8192`、`TASK_STACK_SENSOR=4096`、`TASK_STACK_CTRL=4096`、`TASK_STACK_INPUT=3072`、`TASK_STACK_POWER=3072`
  - 定义 WDT：`TASK_WDT_TIMEOUT_S=5`
  - 头文件用 `#pragma once` + `static constexpr`（数组用 constexpr 数组）

  **Must NOT do**:
  - 不要定义引脚宏（那是 pins.h 的职责）
  - 不要放运行时全局状态变量

  **Recommended Agent Profile**:
  - **Category**: `quick`（单文件纯常量）
  - **Skills**: `[]`

  **Parallelization**: Wave 1 并行（与 T01-T04、T06-T07 同批）

  **References**:
  - `openspec/changes/init-esp32-fan-controller/design.md` D3（温控参数）、D9（任务配置）

  **Acceptance Criteria**:
  - [ ] `pio run -e esp32dev` 能找到 include/app_config.h 且无宏重复定义警告
  - [ ] 所有核心算法常量就位（NTC、温控、I2C、按键、任务栈、WDT 六类）

  **QA Scenarios**:
  ```
  Scenario: app_config.h 可被其他模块包含
    Tool: Bash
    Preconditions: include/app_config.h 已创建
    Steps:
      1. echo '#include "app_config.h"' > test_include.cpp && echo 'int main(){ return (int)NTC_B_VALUE + (int)NTC_R25_OHM + (int)FAN_PWM_MAX + (int)INA226_ADDR_RAIL[0]; }' >> test_include.cpp
      2. g++ -I include test_include.cpp -o test_include.exe
    Expected Result: 编译成功，无"undefined reference"
    Evidence: .sisyphus/evidence/task-05-include-check.txt
  ```

- [x] 6. **T06 创建 src/ 目录骨架（占位 .gitkeep）**

  **Depends on**: None（可并行）
  **Files**: `src/hal/.gitkeep`、`src/display/.gitkeep`、`src/ui/.gitkeep`、`src/sensors/ntc/.gitkeep`、`src/sensors/ina226/.gitkeep`、`src/fan/.gitkeep`、`src/power/.gitkeep`、`src/input/.gitkeep`、`src/app/.gitkeep`、`test/native/.gitkeep`

  **What to do**:
  - 在每个目录下放一个空的 `.gitkeep` 文件（Windows 下用 `ni` 或 `New-Item`）
  - 目录结构严格匹配 design.md D10

  **Must NOT do**:
  - 不要在此任务中写任何 C/C++ 源文件
  - 不要创建 design.md 未列出的目录

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**: Wave 1 并行

  **References**:
  - `openspec/changes/init-esp32-fan-controller/design.md` D10（目录布局）

  **Acceptance Criteria**:
  - [ ] `Get-ChildItem -Recurse -Directory src/,test/` 列出 9 个 src 子目录 + 1 个 test/native
  - [ ] 每个目录至少包含 .gitkeep

  **QA Scenarios**:
  ```
  Scenario: 目录树完整
    Tool: Bash (pwsh)
    Steps:
      1. Get-ChildItem -Recurse -Directory src/ | Select-Object -ExpandProperty FullName
    Expected Result: 9 行输出，包含 hal/display/ui/sensors/ntc/sensors/ina226/fan/power/input/app
    Evidence: .sisyphus/evidence/task-06-tree.txt
  ```

- [x] 7. **T07 创建占位 src/main.cpp**

  **Depends on**: T06（目录存在）
  **Files**: `src/main.cpp`（新建占位，**注意：spec 规定入口在 src/ 根目录，不是 src/app/**）

  **What to do**:
  - 写最简 Arduino 框架：`#include <Arduino.h>` + `void setup(){ Serial.begin(115200); Serial.println("boot"); }` + `void loop(){ delay(1000); }`
  - 目的仅为让 Wave 1 结束时 `pio run -e esp32dev` 可出固件（即便功能空）

  **Must NOT do**:
  - 不要引入任何业务模块（sensors/fan/power 等后续波次接入）
  - 不要配置 FreeRTOS 任务（T34 负责）

  **Recommended Agent Profile**:
  - **Category**: `quick`
  - **Skills**: `[]`

  **Parallelization**: Wave 1 并行（依赖 T06 目录）

  **References**:
  - `openspec/changes/init-esp32-fan-controller/design.md` D10 `src/main.cpp`（spec 强制路径）

  **Acceptance Criteria**:
  - [ ] 文件 < 20 行
  - [ ] 仅包含 Arduino.h，无其他依赖

  **QA Scenarios**:
  ```
  Scenario: 占位 main 编译通过（需等 T01-T10 齐备后 Wave 2 末尾验证）
    Tool: Bash
    Steps:
      1. 此任务本身不触发编译，由 T43 在 Wave 5 统一验证
    Expected Result: 文件存在且语法无误（Get-Content src/main.cpp | Select-String "setup" -Quiet 返回 True）
    Evidence: .sisyphus/evidence/task-07-stub.txt
  ```

### Wave 2 — 依赖 & 库配置 & RED 测试（12 任务，依赖 Wave 1）

- [ ] 8. **T08 platformio.ini 追加 lib_deps**

  **Depends on**: T01
  **Files**: `platformio.ini`（修改）

  **What to do**:
  - `[env:esp32dev]` 下 `lib_deps` 增加：`bodmer/TFT_eSPI@^2.5.43`、`lvgl/lvgl@^9.2.2`、`robtillaart/INA226@^0.6.0`、`thomasfredericks/Bounce2@^2.71`
  - 确保 monitor_filters=esp32_exception_decoder

  **Must NOT do**:
  - 不要引入未经 design.md 批准的库
  - 不要固定到 git commit hash

  **Recommended Agent Profile**: `quick`

  **Parallelization**: Wave 2 Group A（串行依赖 T01，但与 T12-T16 测试文件并行）

  **References**: design.md D1（依赖清单）

  **Acceptance Criteria**:
  - [ ] `pio pkg install -e esp32dev` 成功（输出 `Library Manager: Installing ...` 后到 `Installed`）
  - [ ] .pio/libdeps/esp32dev/ 下出现 4 个库目录

  **QA Scenarios**:
  ```
  Scenario: 依赖解析
    Tool: Bash (pwsh)
    Steps: 
      1. pio pkg install -e esp32dev
      2. Get-ChildItem .pio/libdeps/esp32dev/ -Directory
    Expected: 至少包含 TFT_eSPI、lvgl、INA226、Bounce2 四个目录
    Evidence: .sisyphus/evidence/task-08-lib-install.log
  ```

- [ ] 9. **T09 platformio.ini TFT_eSPI build_flags**

  **Depends on**: T01, T04（用 pins.h 中 TFT_* 宏）
  **Files**: `platformio.ini`（修改）

  **What to do**:
  - `[env:esp32dev].build_flags` 增加（严格对齐 design.md D3）：`-DUSER_SETUP_LOADED=1 -DST7789_DRIVER=1 -DTFT_WIDTH=240 -DTFT_HEIGHT=280 -DCGRAM_OFFSET=1 -DTFT_MOSI=23 -DTFT_SCLK=18 -DTFT_CS=5 -DTFT_DC=2 -DTFT_RST=4 -DTFT_BL=16 -DSPI_FREQUENCY=40000000 -DSPI_READ_FREQUENCY=20000000 -DLOAD_GLCD=0 -DLOAD_FONT2=0 -DLOAD_FONT4=0 -DLOAD_GFXFF=0 -DSMOOTH_FONT=1`
  - 通过 `-include include/pins.h` 使 pins.h 优先（可选）

  **Must NOT do**:
  - 不要同时留下 User_Setup.h 自定义文件
  - 不要用低于 27MHz 的 SPI 频率（降低帧率）

  **Recommended Agent Profile**: `quick`

  **Parallelization**: Wave 2 Group A

  **References**: design.md D1 TFT_eSPI 配置、Research F2/F6

  **Acceptance Criteria**:
  - [ ] build_flags 中出现 USER_SETUP_LOADED=1
  - [ ] `pio run -e esp32dev -t envdump | Select-String TFT_WIDTH` 返回 240

  **QA Scenarios**:
  ```
  Scenario: TFT 宏注入
    Tool: Bash (pwsh)
    Steps: pio run -e esp32dev -t envdump 2>&1 | Select-String "USER_SETUP_LOADED|TFT_WIDTH"
    Expected: 两行匹配
    Evidence: .sisyphus/evidence/task-09-tft-flags.txt
  ```

- [ ] 10. **T10 platformio.ini LVGL build_flags**

  **Depends on**: T01
  **Files**: `platformio.ini`（修改）

  **What to do**:
  - `[env:esp32dev].build_flags` 增加：`-DLV_CONF_INCLUDE_SIMPLE=1 -I include`
  - 确保 include/ 被 compiler 识别

  **Must NOT do**:
  - 不要 `-DLV_CONF_PATH=xxx`（与 SIMPLE 模式冲突）

  **Recommended Agent Profile**: `quick`

  **Parallelization**: Wave 2 Group A

  **References**: Research F2、design.md D1

  **Acceptance Criteria**:
  - [ ] LV_CONF_INCLUDE_SIMPLE=1 出现在 envdump
  - [ ] -I include 生效

  **QA Scenarios**:
  ```
  Scenario: LVGL include 路径
    Tool: Bash (pwsh)
    Steps: pio run -e esp32dev -t envdump 2>&1 | Select-String "LV_CONF_INCLUDE_SIMPLE"
    Expected: 1 行匹配
    Evidence: .sisyphus/evidence/task-10-lvgl-flags.txt
  ```

- [ ] 11. **T11 创建 include/lv_conf.h（LVGL 9.x 完整配置）**

  **Depends on**: T08（lvgl 库已安装可参考 lv_conf_template.h）、T10
  **Files**: `include/lv_conf.h`（新建，约 900 行）

  **What to do**:
  - 从 `.pio/libdeps/esp32dev/lvgl/lv_conf_template.h` 复制并修改
  - 设置 `LV_COLOR_DEPTH=16`、`LV_COLOR_16_SWAP=1`、`LV_USE_PERF_MONITOR=0`、`LV_MEM_SIZE=32768`、`LV_TICK_CUSTOM=1`
  - 保留 Arduino `LV_TICK_CUSTOM_INCLUDE="Arduino.h"`、`LV_TICK_CUSTOM_SYS_TIME_EXPR=(millis())`
  - 启用字体：LV_FONT_MONTSERRAT_14/16/20/28 ON
  - 开启 LV_USE_LABEL/BAR/SLIDER/BTN/ARC/CHART/SWITCH

  **Must NOT do**:
  - 不要开 LV_USE_DEMO_*（二进制膨胀）
  - 不要启用文件系统（无 SD 卡）

  **Recommended Agent Profile**: `unspecified-high`（大文件精细改造）
  **Skills**: `[]`

  **Parallelization**: Wave 2 Group B（依赖 T08 拉取的模板）

  **References**: Research F2/F3、design.md D1 LVGL 配置

  **Acceptance Criteria**:
  - [ ] `pio run -e esp32dev` 编译 LVGL 通过
  - [ ] ELF 中 `lv_init` 符号存在

  **QA Scenarios**:
  ```
  Scenario: lv_conf.h 被识别
    Tool: Bash (pwsh)
    Steps: pio run -e esp32dev 2>&1 | Select-String "lv_conf"
    Expected: 无 "Cannot find lv_conf.h" 错误
    Evidence: .sisyphus/evidence/task-11-lvgl-build.log
  ```

- [ ] 12. **T12 test/native/test_ntc_beta.cpp（RED 测试）**

  **Depends on**: T01（native env 就位）
  **Files**: `test/native/test_ntc_beta.cpp`（新建）

  **What to do**:
  - 写 Unity 测试：`test_beta_25c_returns_10k_resistance`、`test_beta_55c_monotonic_decrease`、`test_adc_to_temp_boundary_0_4095`
  - 调用尚未实现的 `ntc_adc_to_temp(uint16_t adc)` → 链接失败即 RED
  - `#include <unity.h>` + `#include "sensors/ntc/ntc.h"`

  **Must NOT do**:
  - 不要直接调 Arduino API（native env 无）

  **Recommended Agent Profile**: `quick`

  **Parallelization**: Wave 2 Group B（并行 T13-T16）

  **References**: design.md D5（NTC β 算法）

  **Acceptance Criteria**:
  - [ ] 测试文件含 3 个 TEST()
  - [ ] `pio test -e native` 在此阶段链接失败（RED），报 `undefined reference to ntc_adc_to_temp`

  **QA Scenarios**:
  ```
  Scenario: RED 状态确认
    Tool: Bash (pwsh)
    Steps: pio test -e native 2>&1 | Select-String "undefined reference|error"
    Expected: 至少 1 行包含 ntc_adc_to_temp
    Evidence: .sisyphus/evidence/task-12-red.log
  ```

- [ ] 13. **T13 test/native/test_fan_curve.cpp（RED）**

  **Depends on**: T01
  **Files**: `test/native/test_fan_curve.cpp`

  **What to do**:
  - 测试 `uint16_t fan_temp_to_pwm(float temp)`（**返回 0-1023，10-bit 分辨率，design.md D4-D5 规定**）：
    - 25°C → ~205（≈20% × 1023）
    - 30°C → ~205（边界，<30°C 恒为 20%）
    - 40°C → ~410（30-50°C 线性段中点 ≈40%）
    - 50°C → ~614（≈60%）
    - 60°C → ~819（50-70°C 线性段中点 ≈80%）
    - 70°C → 1023（100%）
    - 75°C → 1023（≥75°C 强制 100%）
  - 测试 `float hysteresis_apply(float current, float target, float band=2.0)` 回滞 2°C
  - 验证三段曲线斜率与 75°C 强制满速

  **Must NOT do**: 同 T12，**不要测 8-bit (0-255) 旧值**
  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 2 Group B

  **References**: design.md D4-D5（风扇曲线 10-bit + 三段 + 75°C 强制满速 + 2°C 回滞）

  **Acceptance Criteria**:
  - [ ] ≥7 个 TEST() 覆盖三段边界 + 75°C 强制 + 回滞
  - [ ] RED：`undefined reference to fan_temp_to_pwm`

  **QA Scenarios**:
  ```
  Scenario: RED
    Tool: Bash (pwsh)
    Steps: pio test -e native 2>&1 | Select-String fan_temp_to_pwm
    Expected: 匹配未定义引用
    Evidence: .sisyphus/evidence/task-13-red.log
  ```

- [ ] 14. **T14 test/native/test_psu_fsm.cpp（RED）**

  **Depends on**: T01
  **Files**: `test/native/test_psu_fsm.cpp`

  **What to do**:
  - 测试电源状态机 `psu_fsm_transition(PsuState current, PsuEvent evt)`（**design.md D7 规定状态名**）：
    - 状态枚举：`Off / Standby / Starting / On / Stopping / Fault`
    - 事件枚举：`EVT_KEY_SHORT / EVT_KEY_LONG / EVT_PWOK_HIGH / EVT_PWOK_LOW / EVT_PWOK_LOST_100MS / EVT_TIMEOUT_1S`
    - 合法转换：
      - Off + (上电默认) → Standby
      - **Standby + EVT_KEY_SHORT → Starting**（短按 K1 开机，design.md D7）
      - Starting + EVT_PWOK_HIGH → On
      - Starting + EVT_TIMEOUT_1S → Fault（PWOK 1s 内未拉高）
      - **On + EVT_KEY_LONG → Stopping**（长按 K1≥2s 关机）
      - On + EVT_PWOK_LOST_100MS → Fault（运行中 PWOK 失稳 100ms）
      - Stopping + EVT_PWOK_LOW → Off
      - Fault + (任意复位事件) → Off
    - 非法转换：Standby + EVT_KEY_LONG 不应进入 Stopping；On + EVT_KEY_SHORT 不变
  - 覆盖 ≥8 条合法 + ≥3 条非法

  **Must NOT do**: 不依赖 Arduino，**不要使用旧状态名 OFF/ON_WAIT/OFF_WAIT**
  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 2 Group B

  **References**: design.md D7（电源状态机）

  **Acceptance Criteria**:
  - [ ] ≥7 个 TEST()
  - [ ] RED：`undefined reference to psu_fsm_transition`

  **QA Scenarios**:
  ```
  Scenario: RED
    Tool: Bash (pwsh)
    Steps: pio test -e native 2>&1 | Select-String psu_fsm_transition
    Expected: 匹配
    Evidence: .sisyphus/evidence/task-14-red.log
  ```

- [ ] 15. **T15 test/native/test_keys_debounce.cpp（RED）**

  **Depends on**: T01
  **Files**: `test/native/test_keys_debounce.cpp`

  **What to do**:
  - 测试按键去抖函数 `key_debounce_update(KeyState* st, bool raw, uint32_t now_ms)`（**5ms 采样 + 至少 3 次连续相同电平判定**，对齐 specs/user-input/spec.md）：
    - 单次电平翻转不更新稳定状态（< 3 次连续相同样本）
    - 连续 3 次（间隔 ≥ KEYS_DEBOUNCE_MS=5ms）相同电平后才确认状态变化（即 ≥ 15ms 稳定）
    - 长按 800ms 触发 LONG 事件
  - 使用 mock 时间戳（不调 millis()）

  **Must NOT do**: 不调 Bounce2 库（native 无 Arduino）
  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 2 Group B

  **References**: design.md D8（按键去抖）

  **Acceptance Criteria**:
  - [ ] 5 个 TEST() 覆盖短按/长按/抖动/释放
  - [ ] RED：undefined reference

  **QA Scenarios**:
  ```
  Scenario: RED
    Tool: Bash (pwsh)
    Steps: pio test -e native 2>&1 | Select-String key_debounce_update
    Expected: 匹配
    Evidence: .sisyphus/evidence/task-15-red.log
  ```

- [ ] 16. **T16 test/native/test_hysteresis.cpp（RED）**

  **Depends on**: T01
  **Files**: `test/native/test_hysteresis.cpp`

  **What to do**:
  - 测试 `hysteresis_apply(float current, float target, float band)`：
    - 温度上升越过 target+band → 返回 target+band
    - 温度下降回到 target-band 以内 → 保持旧值
    - 验证 FAN_HYSTERESIS=2.0 行为

  **Must NOT do**: 不涉及硬件
  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 2 Group B

  **References**: design.md D3 回滞控制

  **Acceptance Criteria**:
  - [ ] 4 个 TEST() 覆盖上升/下降/边界/稳态
  - [ ] RED

  **QA Scenarios**:
  ```
  Scenario: RED
    Tool: Bash (pwsh)
    Steps: pio test -e native 2>&1 | Select-String hysteresis_apply
    Expected: 匹配
    Evidence: .sisyphus/evidence/task-16-red.log
  ```

- [ ] 17. **T17 创建 README.md**

  **Depends on**: T01（需引用 platformio.ini）
  **Files**: `README.md`

  **What to do**:
  - 章节：项目简介、硬件 BOM 表（对齐 design.md D2）、目录结构、构建命令（`pio run -e esp32dev` / `pio test -e native` / `pio check`）、烧录命令（预留）、LICENSE
  - 中文文档

  **Must NOT do**:
  - 不要写假的性能数据
  - 不要承诺硬件已实测

  **Recommended Agent Profile**: `writing`
  **Skills**: `[chinese-documentation]`

  **Parallelization**: Wave 2 Group C（独立）

  **References**: design.md 全文（D1-D10）、openspec/changes/init-esp32-fan-controller/specs/project-scaffold/spec.md

  **Acceptance Criteria**:
  - [ ] 文件 ≥ 60 行
  - [ ] 包含 `pio run -e esp32dev` 与 `pio test -e native` 两条命令

  **QA Scenarios**:
  ```
  Scenario: README 完备
    Tool: Bash (pwsh)
    Steps: Select-String -Path README.md -Pattern "pio run|pio test" | Measure-Object
    Expected: Count ≥ 2
    Evidence: .sisyphus/evidence/task-17-readme.txt
  ```

- [ ] 18. **T18 创建 src/hal/i2c_bus.{h,cpp}**

  **Depends on**: T04（pins.h）、T05（app_config.h）、T06（目录）
  **Files**: `src/hal/i2c_bus.h`、`src/hal/i2c_bus.cpp`

  **What to do**:
  - 封装 `i2c_bus_init()`（调用 `Wire.begin(I2C_SDA, I2C_SCL, I2C_FREQ_HZ)`）
  - 提供 `i2c_scan()` 调试函数
  - 线程安全：使用 FreeRTOS Mutex 保护多任务访问

  **Must NOT do**:
  - 不要直接暴露 Wire 对象（封装原则）
  - 不要在 ISR 中调用

  **Recommended Agent Profile**: `quick`
  **Skills**: `[]`

  **Parallelization**: Wave 2 Group C（与 T19 并行）

  **References**: design.md D4（I2C 总线）、D6（INA226）

  **Acceptance Criteria**:
  - [ ] 编译通过（Wave 2 末编译验证）
  - [ ] API 含 init/scan/mutex

  **QA Scenarios**:
  ```
  Scenario: 头文件可用
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/hal/i2c_bus.h -Pattern "i2c_bus_init"
    Expected: 匹配
    Evidence: .sisyphus/evidence/task-18-i2c.txt
  ```

- [ ] 19. **T19 创建 src/hal/spi_bus.{h,cpp}**

  **Depends on**: T04、T05、T06
  **Files**: `src/hal/spi_bus.h`、`src/hal/spi_bus.cpp`

  **What to do**:
  - SPI HAL 封装（仅显示屏用）
  - 提供 `spi_bus_init()`（由 TFT_eSPI 内部管理，此处仅做文档与预留 mutex）

  **Must NOT do**:
  - 不要重复初始化（TFT_eSPI 已管理 HSPI/VSPI）
  - 不要占用其他 SPI 外设

  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 2 Group C

  **References**: design.md D4

  **Acceptance Criteria**:
  - [ ] 文件存在，API 可编译

  **QA Scenarios**:
  ```
  Scenario: 头文件存在
    Tool: Bash (pwsh)
    Steps: Test-Path src/hal/spi_bus.h
    Expected: True
    Evidence: .sisyphus/evidence/task-19-spi.txt
  ```

### Wave 3 — 核心模块实现 GREEN（14 任务，依赖 Wave 2）

- [ ] 20. **T20 src/sensors/ntc/ntc.{h,cpp}（GREEN T12）**

  **Depends on**: T04, T05, T12（RED 测试就位）
  **Files**: `src/sensors/ntc/ntc.h`、`src/sensors/ntc/ntc.cpp`

  **What to do**:
  - 实现 `float ntc_adc_to_temp(uint16_t adc)`：β 公式 `1/T = 1/T0 + ln(R/R0)/β`，参数 R25=10kΩ、B=3950、上拉 10kΩ
  - 提供 `ntc_init()` 配置 `analogReadResolution(12)` + `analogSetAttenuation(ADC_11db)`
  - 提供 `float ntc_read()` 返回摄氏度（**仅 1 路 NTC@GPIO36，spec/D2 规定单路**）
  - 内部做 16 次采样取中值（specs/temperature-sensing 要求）
  - 故障判定：温度 >150°C → NTC_SHORT，温度 <-40°C → NTC_OPEN
  - native env 用条件编译去掉 Arduino 调用（仅保留纯算法）

  **Must NOT do**:
  - **不要实现多通道 `ntc_read_ch(ch)`**——spec 只有 1 路 NTC@GPIO36
  - 不要做移动平均（后续 controlTask 做）
  - 不要在 ISR 中调用 analogRead

  **Recommended Agent Profile**: `deep`
  **Skills**: `[test-driven-development]`

  **Parallelization**: Wave 3 Group A（与 T21-T23 并行）

  **References**: design.md D5、Research F5、test_ntc_beta.cpp

  **Acceptance Criteria**:
  - [ ] `pio test -e native -f test_ntc_beta` 全绿
  - [ ] 25°C → 返回值在 24.8-25.2 之间

  **QA Scenarios**:
  ```
  Scenario: GREEN 通过
    Tool: Bash (pwsh)
    Steps: pio test -e native --filter test_ntc_beta 2>&1 | Select-String "PASSED|FAILED"
    Expected: "3 Tests 0 Failures"
    Evidence: .sisyphus/evidence/task-20-green.log
  ```

- [ ] 21. **T21 src/fan/fan_curve.{h,cpp}（GREEN T13+T16）**

  **Depends on**: T05, T13, T16
  **Files**: `src/fan/fan_curve.h`、`src/fan/fan_curve.cpp`

  **What to do**:
  - 实现 `uint16_t fan_temp_to_pwm(float temp)`（**返回 0-1023，10-bit**），三段分段（design.md D4-D5）：
    - `temp < 30°C` → `0.20 × 1023 ≈ 205`
    - `30°C ≤ temp < 50°C` → 线性 20%→60%（即 `205 + (temp-30)/20 × (614-205)`）
    - `50°C ≤ temp < 70°C` → 线性 60%→100%（即 `614 + (temp-50)/20 × (1023-614)`）
    - `temp ≥ 70°C` → `1023`
    - **`temp ≥ 75°C` → 强制 1023（紧急满速）**
  - 实现 `float hysteresis_apply(float current, float target, float band=2.0f)`：回滞带 2°C，避免在阈值附近抖动
  - 纯函数无依赖（native 可测）

  **Must NOT do**:
  - **不要返回 8-bit (0-255)** —— spec 是 10-bit (0-1023)
  - 不要做 PWM 输出（T22 负责）
  - 不要依赖全局状态

  **Recommended Agent Profile**: `deep`
  **Skills**: `[test-driven-development]`

  **Parallelization**: Wave 3 Group A

  **References**: design.md D3

  **Acceptance Criteria**:
  - [ ] `pio test -e native -f test_fan_curve` 全绿
  - [ ] `pio test -e native -f test_hysteresis` 全绿

  **QA Scenarios**:
  ```
  Scenario: 双测试 GREEN
    Tool: Bash (pwsh)
    Steps: pio test -e native --filter "test_fan_curve test_hysteresis" 2>&1 | Select-String Failures
    Expected: "0 Failures"
    Evidence: .sisyphus/evidence/task-21-green.log
  ```

- [ ] 22. **T22 src/fan/fan_pwm.{h,cpp}（LEDC PWM 输出）**

  **Depends on**: T04, T05, T21
  **Files**: `src/fan/fan_pwm.h`、`src/fan/fan_pwm.cpp`

  **What to do**:
  - 用 `ledcSetup(FAN_LEDC_CH=0, 25000, FAN_PWM_RES_BITS=10)` + `ledcAttachPin(FAN_PWM=25, 0)`（**25kHz、10-bit、LEDC 通道 0，design.md D4**）
  - 提供 `fan_pwm_init()`、`fan_pwm_set(uint16_t duty_0_1023)`
  - 低转速保护：duty < FAN_PWM_MIN（≈205，对应 20%）时使用 FAN_PWM_MIN（除非 0 表示主动关停）

  **Must NOT do**:
  - **不要使用 8-bit (`ledcSetup(CH, 25000, 8)`)** —— spec 是 10-bit
  - 不要重复初始化 LEDC 通道
  - 不要在 ISR 调用

  **Recommended Agent Profile**: `quick`

  **Parallelization**: Wave 3 Group A（依赖 T21 fan_curve）

  **References**: design.md D3

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] ELF 含 `ledcWrite` 符号

  **QA Scenarios**:
  ```
  Scenario: 编译与符号
    Tool: Bash (pwsh)
    Steps: pio run -e esp32dev 2>&1; xtensa-esp32-elf-nm .pio/build/esp32dev/firmware.elf 2>&1 | Select-String fan_pwm_set
    Expected: 符号存在
    Evidence: .sisyphus/evidence/task-22-pwm.log
  ```

- [ ] 23. **T23 src/fan/fan_tach.{h,cpp}（PCNT 转速采集）**

  **Depends on**: T04, T05
  **Files**: `src/fan/fan_tach.h`、`src/fan/fan_tach.cpp`

  **What to do**:
  - `#include "driver/pcnt.h"`（legacy API）
  - 配置 PCNT_UNIT_0 输入 FAN_TACH 引脚，上升沿计数
  - 提供 `fan_tach_init()`、`uint32_t fan_tach_rpm()`（每 1s 读取计数 → RPM = count × 30，4 线风扇每转 2 脉冲）
  - 考虑溢出保护

  **Must NOT do**:
  - 不要用新版 PCNT API（arduino-esp32 2.0.x 不支持）
  - 不要在中断中做浮点运算

  **Recommended Agent Profile**: `deep`
  **Skills**: `[]`

  **Parallelization**: Wave 3 Group A

  **References**: design.md D3、Research F1

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] 含 `pcnt_unit_config` 调用

  **QA Scenarios**:
  ```
  Scenario: PCNT 符号
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/fan/fan_tach.cpp -Pattern "pcnt_unit_config|driver/pcnt.h"
    Expected: 两个匹配
    Evidence: .sisyphus/evidence/task-23-pcnt.txt
  ```

- [ ] 24. **T24 src/sensors/ina226/ina226.{h,cpp}**

  **Depends on**: T18, T05
  **Files**: `src/sensors/ina226/ina226.h`、`src/sensors/ina226/ina226.cpp`

  **What to do**:
  - 用 robtillaart/INA226 库包装：**实例化 3 个 `INA226` 对象，I2C 地址 0x40 / 0x41 / 0x44**（design.md D6）
  - 每个实例 `init()` 调用 `setMaxCurrentShunt(40, 0.002)`（**最大 40A、shunt = 2mΩ**，design.md D6）
  - 提供 `ina226_init()` 一次性初始化全部 3 路
  - 提供 `bool ina226_read(uint8_t rail_idx, float* v_out, float* i_ma_out, float* p_mw_out)`（rail_idx ∈ {0,1,2}）
  - I2C NACK 时返回 false 并记录故障标志
  - 封装错误码（返回 bool + out param）

  **Must NOT do**:
  - **不要只实例化 1 路** —— spec 是 3 路 0x40/0x41/0x44
  - **不要使用 `shunt=0.01Ω, max=5A`** —— spec 是 `setMaxCurrentShunt(40, 0.002)`
  - 不要硬编码地址（用 `INA226_ADDR_RAIL[3] = {0x40,0x41,0x44}`）

  **Recommended Agent Profile**: `unspecified-high`

  **Parallelization**: Wave 3 Group B（与 T20-T23 并行）

  **References**: design.md D6

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] API 导出 3 个读取函数

  **QA Scenarios**:
  ```
  Scenario: 符号与依赖
    Tool: Bash (pwsh)
    Steps: pio run -e esp32dev 2>&1 | Select-String "INA226"
    Expected: 无 "not found"
    Evidence: .sisyphus/evidence/task-24-ina226.log
  ```

- [ ] 25. **T25 src/power/psu_fsm.{h,cpp}（GREEN T14）**

  **Depends on**: T14
  **Files**: `src/power/psu_fsm.h`、`src/power/psu_fsm.cpp`

  **What to do**:
  - 实现状态机：`enum PsuState { Off, Standby, Starting, On, Stopping, Fault }`（**design.md D7 规定的 6 状态**）
  - 事件枚举：`enum PsuEvent { EVT_BOOT, EVT_KEY_SHORT, EVT_KEY_LONG, EVT_PWOK_HIGH, EVT_PWOK_LOW, EVT_PWOK_LOST_100MS, EVT_TIMEOUT_1S, EVT_FAULT_RESET }`
  - `PsuState psu_fsm_transition(PsuState current, PsuEvent evt)` 实现完整迁移表：
    - `Off + EVT_BOOT → Standby`
    - `Standby + EVT_KEY_SHORT → Starting`（短按 K1 开机）
    - `Starting + EVT_PWOK_HIGH → On`
    - `Starting + EVT_TIMEOUT_1S → Fault`（1s 内未拉起 PWOK）
    - `On + EVT_KEY_LONG → Stopping`（长按 K1≥2s 关机）
    - `On + EVT_PWOK_LOST_100MS → Fault`（运行中 PWOK 失稳 100ms）
    - `Stopping + EVT_PWOK_LOW → Off`
    - `Fault + EVT_FAULT_RESET → Off`
    - 其他组合保持当前状态
  - 纯函数（native 可测）

  **Must NOT do**:
  - **不要使用旧状态名 OFF/ON_WAIT/OFF_WAIT** —— design.md D7 规定 Off/Standby/Starting/On/Stopping/Fault
  - 不要操作 GPIO（T26 负责）
  - 不要含时间依赖（time/timeout 由调用方判定后作为 event 传入）

  **Recommended Agent Profile**: `deep`
  **Skills**: `[test-driven-development]`

  **Parallelization**: Wave 3 Group B

  **References**: design.md D7

  **Acceptance Criteria**:
  - [ ] `pio test -e native -f test_psu_fsm` 全绿

  **QA Scenarios**:
  ```
  Scenario: GREEN
    Tool: Bash (pwsh)
    Steps: pio test -e native --filter test_psu_fsm 2>&1 | Select-String Failures
    Expected: "0 Failures"
    Evidence: .sisyphus/evidence/task-25-green.log
  ```

- [ ] 26. **T26 src/power/ps_on.{h,cpp}（PS_ON/PW_OK GPIO 控制）**

  **Depends on**: T04, T05, T25
  **Files**: `src/power/ps_on.h`、`src/power/ps_on.cpp`

  **What to do**:
  - `ps_on_init()`: PS_ON 输出（拉高=关/拉低=开），PW_OK 输入（**纯 INPUT，禁用内部上拉**——GPIO34 为只读输入，依赖外部电平转换电路提供有效电平，对齐 design.md D2 + specs/power-switch-control/spec.md）
  - `ps_on_set(bool enable)`、`bool pw_ok_read()`
  - 与 psu_fsm 搭配（fsm 给出决策，此模块执行 IO）

  **Must NOT do**:
  - 不要在此模块做状态机逻辑

  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 3 Group B

  **References**: design.md D7

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] API 3 个函数齐全

  **QA Scenarios**:
  ```
  Scenario: 符号
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/power/ps_on.h -Pattern "ps_on_set|pw_ok_read"
    Expected: 2 行
    Evidence: .sisyphus/evidence/task-26-pson.txt
  ```

- [ ] 27. **T27 src/input/keys.{h,cpp}（GREEN T15）**

  **Depends on**: T04, T05, T15
  **Files**: `src/input/keys.h`、`src/input/keys.cpp`

  **What to do**:
  - 封装 Bounce2 库：init 3 个按键（KEY_UP/KEY_DOWN/KEY_OK）
  - 实现 `key_debounce_update(KeyState*, bool raw, uint32_t now_ms)` 纯函数（通过 T15 测试）
  - Arduino 端 `keys_update()` 调用 Bounce2.update() 后转发到纯函数

  **Must NOT do**:
  - 不在 ISR 中使用

  **Recommended Agent Profile**: `deep`
  **Skills**: `[test-driven-development]`
  **Parallelization**: Wave 3 Group B

  **References**: design.md D8

  **Acceptance Criteria**:
  - [ ] `pio test -e native -f test_keys_debounce` 全绿

  **QA Scenarios**:
  ```
  Scenario: GREEN
    Tool: Bash (pwsh)
    Steps: pio test -e native --filter test_keys_debounce 2>&1 | Select-String Failures
    Expected: "0 Failures"
    Evidence: .sisyphus/evidence/task-27-green.log
  ```

- [ ] 28. **T28 src/display/tft_driver.{h,cpp}**

  **Depends on**: T09, T11, T19
  **Files**: `src/display/tft_driver.h`、`src/display/tft_driver.cpp`

  **What to do**:
  - 封装 TFT_eSPI 单例：`tft_driver_init()`、`TFT_eSPI& tft()`
  - 设置背光 PWM、亮度控制 API `tft_set_brightness(uint8_t)`
  - 初始化 orientation 与 fill

  **Must NOT do**:
  - 不要在此处写 LVGL 代码（T29 负责）

  **Recommended Agent Profile**: `visual-engineering`
  **Skills**: `[]`
  **Parallelization**: Wave 3 Group C（与 T29-T30 串行）

  **References**: design.md D1

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] ELF 含 TFT_eSPI vtable

  **QA Scenarios**:
  ```
  Scenario: 编译与符号
    Tool: Bash (pwsh)
    Steps: pio run -e esp32dev 2>&1; Select-String -Path src/display/tft_driver.cpp -Pattern "TFT_eSPI"
    Expected: 编译 SUCCESS，符号匹配
    Evidence: .sisyphus/evidence/task-28-tft.log
  ```

- [ ] 29. **T29 src/display/lvgl_port.{h,cpp}**

  **Depends on**: T28, T11
  **Files**: `src/display/lvgl_port.h`、`src/display/lvgl_port.cpp`

  **What to do**:
  - 双缓冲：`static lv_color_t buf1[240*28], buf2[240*28]`
  - `lv_init()` + `lv_display_create()` + `lv_display_set_flush_cb(my_flush_cb)`
  - flush_cb 内调用 TFT_eSPI pushColors
  - 触摸接入预留（项目仅按键，无触摸）

  **Must NOT do**:
  - 不要放在 IRAM（LVGL 禁止）
  - 不要启用 LVGL 文件系统

  **Recommended Agent Profile**: `visual-engineering`
  **Parallelization**: Wave 3 Group C

  **References**: design.md D1、Research F3

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] ELF 含 `lv_init`、`lv_display_create`

  **QA Scenarios**:
  ```
  Scenario: LVGL 链接
    Tool: Bash (pwsh)
    Steps: pio run -e esp32dev 2>&1 | Select-String "lv_init|lvgl"
    Expected: 编译成功
    Evidence: .sisyphus/evidence/task-29-lvgl-port.log
  ```

- [ ] 30. **T30 src/ui/ui_main.{h,cpp} 主界面**

  **Depends on**: T29
  **Files**: `src/ui/ui_main.h`、`src/ui/ui_main.cpp`

  **What to do**:
  - 创建主界面：大号温度标签、风扇转速、电压/电流、状态文本
  - 使用 LVGL 9.x API：`lv_label_create()`、`lv_obj_set_style_text_font()`
  - 提供 `ui_main_create(lv_obj_t* parent)`、`ui_main_update(float temp, uint32_t rpm, float v, float mA, const char* status)`

  **Must NOT do**:
  - 不要在此处做动画（项目不需要）
  - 不要硬编码字符串英文（中文使用 Montserrat 不支持，仅显示数字与符号）

  **Recommended Agent Profile**: `visual-engineering`
  **Skills**: `[frontend-design]`
  **Parallelization**: Wave 3 Group C

  **References**: openspec/changes/init-esp32-fan-controller/specs/ui-dashboard/spec.md（主界面布局与字段）、design.md D3（显示驱动）

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] API 含 create + update 两函数

  **QA Scenarios**:
  ```
  Scenario: UI 符号
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/ui/ui_main.h -Pattern "ui_main_create|ui_main_update"
    Expected: 2 行
    Evidence: .sisyphus/evidence/task-30-ui.txt
  ```

- [ ] 31. **T31 src/ui/ui_menu.{h,cpp} 菜单界面**

  **Depends on**: T30
  **Files**: `src/ui/ui_menu.h`、`src/ui/ui_menu.cpp`

  **What to do**:
  - 设置菜单：温控曲线查看、亮度调节、电源开关
  - 长按 OK 进入菜单，上下键选择
  - 使用 LVGL list widget

  **Must NOT do**:
  - 不要持久化设置（无 NVS 任务）

  **Recommended Agent Profile**: `visual-engineering`
  **Parallelization**: Wave 3 Group D

  **References**: openspec/changes/init-esp32-fan-controller/specs/ui-dashboard/spec.md（菜单交互）、specs/user-input/spec.md（长按 OK 进入菜单）

  **Acceptance Criteria**:
  - [ ] 编译通过

  **QA Scenarios**:
  ```
  Scenario: 编译
    Tool: Bash (pwsh)
    Steps: pio run -e esp32dev 2>&1 | Select-String "ui_menu"
    Expected: 成功
    Evidence: .sisyphus/evidence/task-31-menu.log
  ```

- [ ] 32. **T32 src/ui/ui_events.{h,cpp} 事件派发**

  **Depends on**: T30, T31, T27
  **Files**: `src/ui/ui_events.h`、`src/ui/ui_events.cpp`

  **What to do**:
  - 将 keys 事件（SHORT/LONG/UP/DOWN）路由到 LVGL 组件
  - 提供 `ui_events_dispatch(KeyEvent evt)`

  **Must NOT do**:
  - 不要在 ISR 直接调用 LVGL

  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 3 Group D

  **References**: openspec/changes/init-esp32-fan-controller/specs/user-input/spec.md（按键事件类型）、specs/ui-dashboard/spec.md（事件路由）

  **Acceptance Criteria**:
  - [ ] 编译通过

  **QA Scenarios**:
  ```
  Scenario: 符号
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/ui/ui_events.h -Pattern "ui_events_dispatch"
    Expected: 匹配
    Evidence: .sisyphus/evidence/task-32-events.txt
  ```

- [ ] 33. **T33 src/app/app_state.{h,cpp} 全局应用状态**

  **Depends on**: T05
  **Files**: `src/app/app_state.h`、`src/app/app_state.cpp`

  **What to do**:
  - 定义 `struct AppState { float temp_c; uint32_t rpm; struct { float voltage; float current_ma; float power_mw; } rail[3]; PsuState psu; uint32_t fault_flags; ... }`（**temp_c 单路 NTC + rail[3] 对应 INA226 0x40/0x41/0x44**）
  - 提供 mutex 保护的 getter/setter
  - 作为各任务间共享数据的单一来源

  **Must NOT do**:
  - 不要用 volatile（用 mutex）
  - 不要跨 header 暴露可变引用

  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 3 Group D

  **References**: design.md D9（任务通信）

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] 提供 get/set API

  **QA Scenarios**:
  ```
  Scenario: 结构体与 API
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/app/app_state.h -Pattern "struct AppState|app_state_get|app_state_set"
    Expected: ≥3 行
    Evidence: .sisyphus/evidence/task-33-state.txt
  ```

### Wave 4 — FreeRTOS 任务 & 故障保护（8 任务，依赖 Wave 3）

- [ ] 34. **T34 src/app/tasks.{h,cpp} FreeRTOS 任务注册**

  **Depends on**: T20-T33（所有核心模块）
  **Files**: `src/app/tasks.h`、`src/app/tasks.cpp`

  **What to do**:
  - 创建 5 个任务（参数从 app_config.h 读取）：
    - `lvglTask`：Core 1, Priority 2, stack 8192, 5ms 周期 `lv_timer_handler()`
    - `sensorTask`：Core 0, Priority 3, stack 4096, 200ms 周期读 NTC+INA226 写入 AppState
    - `controlTask`：Core 0, Priority 4, stack 4096, 500ms 周期跑温控算法 + 更新 fan_pwm
    - `inputTask`：Core 0, Priority 2, stack 3072, 5ms 周期 keys_update + ui_events_dispatch
    - `powerTask`：Core 0, Priority 5, stack 3072, 事件驱动 psu_fsm
  - 用 `xTaskCreatePinnedToCore`
  - 每任务末尾 `esp_task_wdt_reset()`

  **Must NOT do**:
  - 不要在任务间共享非线程安全资源
  - 不要把 LVGL 调用跨任务

  **Recommended Agent Profile**: `deep`
  **Skills**: `[]`

  **Parallelization**: Wave 4 Group A（关键路径）

  **References**: design.md D9

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] ELF 含 5 个任务函数符号

  **QA Scenarios**:
  ```
  Scenario: 5 任务符号检查
    Tool: Bash (pwsh)
    Steps: xtensa-esp32-elf-nm .pio/build/esp32dev/firmware.elf | Select-String "lvglTask|sensorTask|controlTask|inputTask|powerTask"
    Expected: 5 行匹配
    Evidence: .sisyphus/evidence/task-34-tasks.log
  ```

- [ ] 35. **T35 src/app/watchdog.{h,cpp} Task WDT 初始化**

  **Depends on**: T05, T34
  **Files**: `src/app/watchdog.h`、`src/app/watchdog.cpp`

  **What to do**:
  - `watchdog_init()`: `esp_task_wdt_init(TASK_WDT_TIMEOUT_S, true)`，注册所有任务句柄
  - 提供 `watchdog_register(TaskHandle_t)`

  **Must NOT do**:
  - 不要在任务内重复 init

  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 4 Group A

  **References**: design.md D9

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] 符号 `esp_task_wdt_init`、`esp_task_wdt_add` 被引用

  **QA Scenarios**:
  ```
  Scenario: WDT 符号
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/app/watchdog.cpp -Pattern "esp_task_wdt_init|esp_task_wdt_add"
    Expected: 2 行
    Evidence: .sisyphus/evidence/task-35-wdt.txt
  ```

- [ ] 36. **T36 src/app/fault_guard.{h,cpp} 故障保护逻辑**

  **Depends on**: T33
  **Files**: `src/app/fault_guard.h`、`src/app/fault_guard.cpp`

  **What to do**:
  - 定义 FaultType 枚举：`FAULT_NONE / FAULT_NTC_SHORT / FAULT_NTC_OPEN / FAULT_OVERTEMP / FAULT_PWOK_LOST / FAULT_INA226_OVERCURRENT / FAULT_FAN_STALL`
  - 定义故障条件：
    - NTC：温度 > NTC_TEMP_SHORT_C 或 < NTC_TEMP_OPEN_C
    - 过温：温度 ≥ FAN_TEMP_FORCE（75°C）触发强制满速；持续过高触发关机由 powerTask 决策
    - PW_OK：失稳累计 PSU_PWOK_LOST_MS（100ms）
    - INA226：任意路电流 > INA226_MAX_CURRENT_A
    - **风扇堵转**：PWM 占空 ≥ FAN_STALL_DUTY_THRESH 持续 FAN_STALL_TIMEOUT_MS（3s）且 RPM=0 → FAULT_FAN_STALL
    - **风扇 Fault 升级关电源**：FAULT_FAN_STALL 持续 FAN_FAULT_SHUTDOWN_MS（5s）→ powerTask 强制 Off
  - 提供 `fault_guard_check(AppState&)` 返回 FaultType
  - 触发 FAULT 时通过 Queue 通知 powerTask + 写 AppState.fault 供 UI 显示

  **Must NOT do**:
  - 不要在此模块直接操作 GPIO（通过 powerTask）

  **Recommended Agent Profile**: `deep`
  **Parallelization**: Wave 4 Group B

  **References**: design.md D5（fan stall 检测）、D7（PSU/PW_OK）、D9（任务通信），openspec/changes/init-esp32-fan-controller/specs/fan-control/spec.md、specs/power-switch-control/spec.md

  **Acceptance Criteria**:
  - [ ] 编译通过
  - [ ] FaultType 枚举至少 7 种（含 FAULT_FAN_STALL）
  - [ ] 风扇堵转检测逻辑（占空+RPM 联合判断）存在

  **QA Scenarios**:
  ```
  Scenario: 故障类型
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/app/fault_guard.h -Pattern "enum.*Fault"
    Expected: 1 行
    Evidence: .sisyphus/evidence/task-36-fault.txt
  ```

- [ ] 37. **T37 sensorTask 业务实现**

  **Depends on**: T20, T24, T33, T34
  **Files**: `src/app/tasks.cpp`（补全 sensorTask 函数体）

  **What to do**:
  - 200ms 循环：读 **1 路 NTC@GPIO36** → 移动平均（5 点）→ 写 `AppState.temp_c`
  - 每 200ms 轮询 1 路 INA226（750ms 周期遍历 0x40/0x41/0x44 三路）→ 写 `AppState.rail[i]`（design.md D6）
  - 温度 >150°C 或 <-40°C → 置 NTC_FAULT；I2C NACK → 置对应 rail 故障标志

  **Must NOT do**:
  - 不要在此任务中执行控制逻辑

  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 4 Group B

  **References**: design.md D9

  **Acceptance Criteria**:
  - [ ] sensorTask 函数体 ≥ 30 行有效代码

  **QA Scenarios**:
  ```
  Scenario: 函数完整
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/app/tasks.cpp -Pattern "void sensorTask"
    Expected: 匹配
    Evidence: .sisyphus/evidence/task-37-sensor.txt
  ```

- [ ] 38. **T38 controlTask 业务实现**

  **Depends on**: T21, T22, T33, T34, T36
  **Files**: `src/app/tasks.cpp`（补全 controlTask 函数体）

  **What to do**:
  - 500ms 循环：取 AppState 温度 → `fan_temp_to_pwm` → `hysteresis_apply` → `fan_pwm_set`（10-bit, 0-1023）
  - 读取风扇 RPM（fan_tach_read），写入 AppState.fan_rpm
  - 调用 `fault_guard_check`：温度 ≥ FAN_TEMP_FORCE（75°C）→ `fan_pwm_set(FAN_PWM_MAX)`（**1023**，不是 255）
  - 检测堵转：占空 ≥ FAN_STALL_DUTY_THRESH（307）持续 3s 且 RPM=0 → 设置 FAULT_FAN_STALL，发 Queue 给 powerTask
  - FAULT 时仍维持 PWM 满速保护

  **Must NOT do**:
  - 不要在此直接读 ADC
  - 不要使用 8-bit 占空值（255）—— spec 是 10-bit（1023）

  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 4 Group B

  **References**: design.md D4、D5（曲线 + 堵转）、D9，openspec/changes/init-esp32-fan-controller/specs/fan-control/spec.md

  **Acceptance Criteria**:
  - [ ] 函数体 ≥ 25 行
  - [ ] 包含堵转检测计时逻辑
  - [ ] 强制满速使用 FAN_PWM_MAX（1023）

  **QA Scenarios**:
  ```
  Scenario: 逻辑存在
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/app/tasks.cpp -Pattern "void controlTask|fan_temp_to_pwm"
    Expected: 2 行
    Evidence: .sisyphus/evidence/task-38-control.txt
  ```

- [ ] 39. **T39 inputTask 业务实现**

  **Depends on**: T27, T32, T34
  **Files**: `src/app/tasks.cpp`

  **What to do**:
  - 5ms 循环：keys_update() → 生成事件 → ui_events_dispatch

  **Must NOT do**: 不阻塞
  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 4 Group B

  **References**: design.md D9

  **Acceptance Criteria**:
  - [ ] inputTask 函数体存在

  **QA Scenarios**:
  ```
  Scenario: 函数
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/app/tasks.cpp -Pattern "void inputTask"
    Expected: 匹配
    Evidence: .sisyphus/evidence/task-39-input.txt
  ```

- [ ] 40. **T40 powerTask 业务实现**

  **Depends on**: T25, T26, T34, T36
  **Files**: `src/app/tasks.cpp`

  **What to do**:
  - 事件驱动（QueueHandle_t），从 inputTask 同时消费 **EVT_KEY_SHORT 与 EVT_KEY_LONG**，同时消费 fault_guard 发来的 FAULT 事件：
    - `Standby + EVT_KEY_SHORT → Starting`，**立即拉低 PS_ON**（GPIO27，**低电平=开机**，对齐 T26 与 design.md D7）
    - `On + EVT_KEY_LONG（≥PSU_LONGPRESS_MS=2s）→ Stopping`，**立即拉高 PS_ON**（高电平=关机）
    - `* + EVT_FAULT → Fault`，**立即拉高 PS_ON**（强制关电源）
  - 启动监控：进入 Starting 后启 PSU_START_TIMEOUT_MS（1s）软定时器，未收到 PWOK_HIGH 则发 EVT_TIMEOUT_1S → Fault
  - 运行监控：On 状态下连续监控 PW_OK（GPIO34），失稳累计 PSU_PWOK_LOST_MS（100ms）发 `EVT_PWOK_LOST_100MS` → Fault
  - **风扇堵转升级**：收到 FAULT_FAN_STALL 后启 FAN_FAULT_SHUTDOWN_MS（5s）定时器，到期仍未恢复 → 强制 Fault → 拉高 PS_ON
  - 调用 `psu_fsm_transition` 计算下一状态，写入 AppState.psu 与 AppState.fault

  **Must NOT do**:
  - **不要只处理长按事件** —— spec 要求短按 K1 开机、长按 K1 关机两个事件都要消费
  - **不要把 PS_ON 极性写反** —— design.md D7 + T26 明确：拉低=开机/拉高=关机
  - 不在中断中操作 Queue（或用 FromISR 变体）

  **Recommended Agent Profile**: `quick`
  **Parallelization**: Wave 4 Group B

  **References**: design.md D7（状态机 + PS_ON 极性 + 时序）、D5（fan stall 升级），openspec/changes/init-esp32-fan-controller/specs/power-switch-control/spec.md

  **Acceptance Criteria**:
  - [ ] powerTask 函数体存在且引用 psu_fsm_transition

  **QA Scenarios**:
  ```
  Scenario: FSM 引用
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/app/tasks.cpp -Pattern "psu_fsm_transition"
    Expected: 匹配
    Evidence: .sisyphus/evidence/task-40-power.txt
  ```

- [ ] 41. **T41 lvglTask 业务实现 & UI 数据刷新**

  **Depends on**: T29, T30, T33, T34
  **Files**: `src/app/tasks.cpp`

  **What to do**:
  - 5ms 循环：`lv_timer_handler()`
  - 另起 250ms 定时器（lv_timer_create）读 AppState 并 `ui_main_update(...)`
  - 锁定 LVGL mutex

  **Must NOT do**:
  - 不跨核调用 LVGL 函数

  **Recommended Agent Profile**: `visual-engineering`
  **Parallelization**: Wave 4 Group B

  **References**: openspec/changes/init-esp32-fan-controller/specs/display-lvgl/spec.md（LVGL 移植）、specs/ui-dashboard/spec.md（刷新字段）、design.md D3、D9

  **Acceptance Criteria**:
  - [ ] lvglTask 函数体含 lv_timer_handler

  **QA Scenarios**:
  ```
  Scenario: LVGL 驱动
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/app/tasks.cpp -Pattern "lv_timer_handler|ui_main_update"
    Expected: 2 行
    Evidence: .sisyphus/evidence/task-41-lvgl-task.txt
  ```

### Wave 5 — 集成 & 编译验证（3 任务，依赖 Wave 4）

- [ ] 42. **T42 重写 src/main.cpp 正式入口**

  **Depends on**: T34-T41
  **Files**: `src/main.cpp`（覆盖 T07 占位版，**spec 规定路径在 src/ 根目录**）

  **What to do**:
  - `setup()`：串口 init → i2c_bus_init → spi_bus_init → tft_driver_init → lvgl_port → ntc_init → ina226_init → fan_pwm_init → fan_tach_init → keys_init → ps_on_init → watchdog_init → tasks_start
  - `loop()`: `vTaskDelay(portMAX_DELAY)` 或删除（FreeRTOS 接管）
  - 串口输出版本号 + 分区信息

  **Must NOT do**:
  - 不要在 loop() 做业务
  - 不要跳过任何模块 init

  **Recommended Agent Profile**: `quick`

  **Parallelization**: Wave 5（依赖前序）

  **References**: design.md D10

  **Acceptance Criteria**:
  - [ ] setup() 调用 ≥ 10 个 init 函数
  - [ ] 编译通过

  **QA Scenarios**:
  ```
  Scenario: 初始化完整
    Tool: Bash (pwsh)
    Steps: Select-String -Path src/main.cpp -Pattern "_init\(" | Measure-Object
    Expected: Count ≥ 10
    Evidence: .sisyphus/evidence/task-42-main.txt
  ```

- [ ] 43. **T43 pio run -e esp32dev 完整编译 + 大小报告**

  **Depends on**: T42
  **Files**: `.sisyphus/evidence/task-43-size.log`（生成）

  **What to do**:
  - `pio run -e esp32dev` 完整编译
  - `pio run -e esp32dev -t size` 生成大小报告
  - `xtensa-esp32-elf-size .pio/build/esp32dev/firmware.elf` 输出各 section 大小
  - 验证预算：.text+.data ≤ 1.5MB，DRAM 空闲 ≥ 100KB

  **Must NOT do**:
  - 不接入硬件
  - 不烧录

  **Recommended Agent Profile**: `quick`

  **Parallelization**: Wave 5

  **References**: design.md D1

  **Acceptance Criteria**:
  - [ ] 编译退出码 0
  - [ ] firmware.elf 存在
  - [ ] 二进制 ≤ 1.5MB
  - [ ] DRAM 使用率 ≤ 70%

  **QA Scenarios**:
  ```
  Scenario: 编译大小验证
    Tool: Bash (pwsh)
    Steps:
      1. pio run -e esp32dev 2>&1 | Tee-Object -FilePath .sisyphus/evidence/task-43-build.log
      2. $elf = ".pio/build/esp32dev/firmware.elf"; Test-Path $elf
      3. pio run -e esp32dev -t size 2>&1 | Tee-Object -FilePath .sisyphus/evidence/task-43-size.log
      4. (Get-Item ".pio/build/esp32dev/firmware.bin").Length -lt 1572864
    Expected: 步骤 2 True、步骤 4 True、日志含 "RAM:" 与 "Flash:"
    Evidence: .sisyphus/evidence/task-43-*.log
  ```

- [ ] 44. **T44 pio test -e native + pio check 静态分析**

  **Depends on**: T43
  **Files**: `.sisyphus/evidence/task-44-*.log`

  **What to do**:
  - `pio test -e native` 执行全部单元测试
  - `pio check -e esp32dev --flags "--suppress=unusedStructMember"` cppcheck 静态分析
  - `openspec validate init-esp32-fan-controller --strict`

  **Must NOT do**:
  - 不修改测试让其通过（应真实通过）

  **Recommended Agent Profile**: `quick`

  **Parallelization**: Wave 5

  **References**: design.md 全文

  **Acceptance Criteria**:
  - [ ] 所有 native 测试 0 failures
  - [ ] pio check 无 HIGH 严重问题
  - [ ] openspec validate 通过

  **QA Scenarios**:
  ```
  Scenario: 测试+静态+spec 验证
    Tool: Bash (pwsh)
    Steps:
      1. pio test -e native 2>&1 | Tee-Object .sisyphus/evidence/task-44-test.log
      2. pio check -e esp32dev 2>&1 | Tee-Object .sisyphus/evidence/task-44-check.log
      3. openspec validate init-esp32-fan-controller --strict 2>&1 | Tee-Object .sisyphus/evidence/task-44-spec.log
    Expected: 所有三条命令退出码 0
    Evidence: .sisyphus/evidence/task-44-*.log
  ```

---

## Final Verification Wave (4 并行审核 agent + 用户 OKAY)

- [ ] **F1. 计划合规性审计** — `oracle`
  逐条对照本计划「Must Have」与「Must NOT Have」，检查最终交付物：每条 Must Have 必须能定位到 `pio run` 输出/源码符号/`pio test` 结果；每条 Must NOT Have 必须用 ripgrep 全仓库扫描确认零违反（如 `rg -n "GPIO[0-9]+" src/ --glob '!pins.h'` 必须无命中）。检查 `.sisyphus/evidence/` 下证据文件存在且与任务编号一一对应。
  Output: `Must Have [N/N] | Must NOT Have [N/N] | Tasks [N/44] | Evidence files [N] | VERDICT: APPROVE/REJECT`

- [ ] **F2. 代码质量审计** — `unspecified-high`
  扫描 src/ 与 include/ 全部 .h/.cpp/.hpp：检查是否有 `volatile` 滥用、未受保护的全局变量、ISR 中调用 LVGL/Serial、`new`/`malloc` 在 ISR 或 LVGL flush_cb 中、`as any` 等价物（C 强制转换）、空 catch、TODO/FIXME/XXX、commented-out code、AI slop 模式（过度抽象/无意义注释/泛型变量名 data/result/temp）。运行 `pio check -e esp32dev` 并解析报告。
  Output: `Files scanned [N] | Issues [low N / med N / high N] | pio check [PASS/FAIL] | VERDICT`

- [ ] **F3. 编译产物 QA** — `unspecified-high`
  执行清单：(1) `pio run -e esp32dev` 退出码 0；(2) `pio test -e native` 全 PASS（解析 JUnit XML 输出）；(3) `pio run -e esp32dev -t size` 检查 .text/.data/.bss/.rodata 各段大小，固件 .bin < 1.5MB；(4) `xtensa-esp32-elf-nm .pio/build/esp32dev/firmware.elf | rg "lv_init|tft_setup|app_main|setup"` 验证关键 symbol 链接；(5) 检查 `.pio/build/esp32dev/firmware.elf` DRAM 占用（`xtensa-esp32-elf-size -A`），余量 ≥ 100KB。所有数据写入 `.sisyphus/evidence/final-qa/build_report.txt`。
  Output: `Build [PASS/FAIL] | Tests [N pass / N fail] | Size [.bin XKB / .text XKB / DRAM free XKB] | Symbols [N/M present] | VERDICT`

- [ ] **F4. 范围保真度审计** — `deep`
  逐条对照 8 个 spec 的全部 Requirement：每个 SHALL 必须能在 src/ 找到对应实现（grep 关键 API + 算法）；每个 Scenario 必须能映射到 native 测试用例或编译验证。检查 git diff 全部新增文件：每个文件必须在「Concrete Deliverables」清单中；任何超出清单的文件必须能在 design.md 找到论证。检查跨任务污染（任务 N 是否修改了任务 M 的预期文件）。
  Output: `Requirements covered [N/M] | Scenarios mapped [N/M] | Files in scope [N/M] | Out-of-scope files [N] | Cross-contamination [CLEAN/N issues] | VERDICT`

→ **4 个 VERDICT 全部为 APPROVE 后**：呈报用户 → 等待用户明确说"okay"或"通过" → 才能标记本计划完成。**任何 REJECT 必须修复后重跑该 agent。**

---

## Commit Strategy

> 默认不自动 git commit。本计划全部任务均不包含 commit 步骤。仅当用户明确要求"提交"时才执行 `git add` + `git commit`。

如用户后续要求提交，建议分组：
- C1: Wave 1（工程骨架）— `chore: scaffold PlatformIO ESP32 project`
- C2: Wave 2（配置与 RED 测试）— `chore: configure LVGL/TFT_eSPI build_flags + add unit tests`
- C3: Wave 3（驱动与算法实现）— `feat: implement HAL drivers and service layer`
- C4: Wave 4（FreeRTOS 集成）— `feat: integrate FreeRTOS tasks and fault protection`
- C5: Wave 5（主程序与验证）— `feat: wire up main.cpp and pass all builds/tests`

---

## Success Criteria

### Verification Commands
```bash
# 编译
pio run -e esp32dev                      # 期望：退出码 0
# 单元测试
pio test -e native                       # 期望：所有 test PASS
# 静态分析
pio check -e esp32dev --skip-packages    # 期望：无 high severity
# 大小
pio run -e esp32dev -t size              # 期望：.bin < 1.5MB
# OpenSpec 校验
openspec validate init-esp32-fan-controller --strict   # 期望：valid
```

### Final Checklist
- [ ] 所有 44 个 TODO 标记为完成
- [ ] 所有 Must Have 项可验证为 YES
- [ ] 所有 Must NOT Have 项可验证为 ABSENT
- [ ] 4 个 Final 审核 agent 全部 APPROVE
- [ ] 用户明确 OKAY
