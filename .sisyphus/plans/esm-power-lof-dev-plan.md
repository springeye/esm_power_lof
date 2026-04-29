# ESM Power LOF 开发计划

## 总览

本计划整合了三个 OpenSpec 变更的实施任务，按依赖关系和优先级组织。

```
┌─────────────────────────────────────────────────────────────────┐
│                    开发阶段规划                                   │
└─────────────────────────────────────────────────────────────────┘

Phase 1: rename-ina226-ch-and-keys-k     ✅ 已完成
Phase 2: init-esp32-fan-controller        🔄 核心固件
Phase 3: add-native-lvgl-simulator        🔄 PC 模拟器
```

---

## Phase 1: INA226 通道与按键重命名 ✅

**状态**: 已完成 (16/16 任务)

已完成的重命名：
- INA226 通道: LOAD/12V/5V → CH1/CH2/CH3
- 按键: UP/ENTER/DOWN → K1/K2/K3
- 所有调用点已更新，33/33 测试通过

---

## Phase 2: ESP32 风扇控制器固件初始化

**状态**: 进行中 (0/52 任务)

**依赖**: 无（基础固件）

### 任务清单

#### 2.1 工程骨架 (project-scaffold)

| # | 任务 | 状态 |
|---|------|------|
| 1.1 | 创建 `platformio.ini` 配置 | ⬜ |
| 1.2 | 创建 `partitions/default_8MB.csv` | ⬜ |
| 1.3 | 创建目录骨架 | ⬜ |
| 1.4 | 创建 `include/pins.h` GPIO 宏 | ⬜ |
| 1.5 | 创建 `include/app_config.h` 配置常量 | ⬜ |
| 1.6 | 创建占位 `src/main.cpp` | ⬜ |
| 1.7 | 添加 lib_deps 依赖 | ⬜ |
| 1.8 | 创建 `README.md` | ⬜ |

#### 2.2 显示与 LVGL 移植 (display-lvgl)

| # | 任务 | 状态 |
|---|------|------|
| 2.1 | TFT_eSPI build_flags 配置 | ⬜ |
| 2.2 | 创建 `include/lv_conf.h` | ⬜ |
| 2.3 | 实现 `display::init()` 双缓冲 | ⬜ |
| 2.4 | 实现 `display::tick()` lv_timer_handler | ⬜ |
| 2.5 | 烧录验证 Hello 标签 | ⬜ |

#### 2.3 主仪表盘 UI (ui-dashboard)

| # | 任务 | 状态 |
|---|------|------|
| 3.1 | 创建主屏幕容器 4 区域 | ⬜ |
| 3.2 | Montserrat 16/24 字体渲染 | ⬜ |
| 3.3 | 实现 update API | ⬜ |
| 3.4 | 告警颜色规则 | ⬜ |
| 3.5 | 底部按键功能提示 | ⬜ |

#### 2.4 NTC 温度采样 (temperature-sensing)

| # | 任务 | 状态 |
|---|------|------|
| 4.1 | `ntc::init()` ADC 配置 | ⬜ |
| 4.2 | `ntc::sampleOnce()` 16次中值 | ⬜ |
| 4.3 | `ntc::computeTempC()` β公式 | ⬜ |
| 4.4 | 短路/开路保护 | ⬜ |
| 4.5 | 挂入 sensorTask 200ms | ⬜ |

#### 2.5 风扇控制 (fan-control)

| # | 任务 | 状态 |
|---|------|------|
| 5.1 | LEDC 25kHz PWM 配置 | ⬜ |
| 5.2 | PCNT 转速计数 | ⬜ |
| 5.3 | `setDuty()`/`getRpm()` | ⬜ |
| 5.4 | 温控曲线 + 2°C 回滞 | ⬜ |
| 5.5 | 堵转检测 | ⬜ |
| 5.6 | 挂入 controlTask 500ms | ⬜ |

#### 2.6 INA226 电源监测 (power-rail-monitoring)

| # | 任务 | 状态 |
|---|------|------|
| 6.1 | Wire.begin() I2C 初始化 | ⬜ |
| 6.2 | 3 个 INA226 实例化 | ⬜ |
| 6.3 | 250ms 错峰轮询 | ⬜ |
| 6.4 | I2C 错误计数与恢复 | ⬜ |
| 6.5 | 数据发布到 UI | ⬜ |

#### 2.7 电源开关控制 (power-switch-control)

| # | 任务 | 状态 |
|---|------|------|
| 7.1 | GPIO27/34 配置 | ⬜ |
| 7.2 | 状态机实现 | ⬜ |
| 7.3 | turnOn()/turnOff() | ⬜ |
| 7.4 | PWOK 失稳检测 | ⬜ |
| 7.5 | 故障保护规则 | ⬜ |
| 7.6 | powerTask 事件驱动 | ⬜ |

#### 2.8 按键输入 (user-input)

| # | 任务 | 状态 |
|---|------|------|
| 8.1 | K1/K2/K3 INPUT_PULLUP | ⬜ |
| 8.2 | 5ms 去抖逻辑 | ⬜ |
| 8.3 | 短按/长按事件 | ⬜ |
| 8.4 | FreeRTOS Queue 发布 | ⬜ |

#### 2.9 系统集成

| # | 任务 | 状态 |
|---|------|------|
| 9.1 | 5 个 FreeRTOS 任务创建 | ⬜ |
| 9.2 | main.cpp 初始化链 | ⬜ |
| 9.3 | 编译烧录 | ⬜ |
| 9.4 | 完整功能联调 | ⬜ |
| 9.5 | 30 分钟长跑测试 | ⬜ |

#### 2.10 收尾

| # | 任务 | 状态 |
|---|------|------|
| 10.1 | openspec validate | ⬜ |
| 10.2 | README 更新 | ⬜ |
| 10.3 | 用户审阅归档 | ⬜ |

---

## Phase 3: PC 原生 LVGL 模拟器

**状态**: 进行中 (0/31 任务)

**依赖**: Phase 2 的 UI 代码和 LVGL 集成

### 任务清单

#### 3.1 环境准备

| # | 任务 | 状态 |
|---|------|------|
| 1.1 | 安装 SDL2 开发库 | ⬜ |
| 1.2 | 验证 pkg-config | ⬜ |

#### 3.2 lv_conf.h 条件化

| # | 任务 | 状态 |
|---|------|------|
| 2.1 | 添加 BUILD_NATIVE 条件块 | ⬜ |
| 2.2 | LV_TICK_CUSTOM 条件化 | ⬜ |
| 2.3 | 验证 ESP32 构建不受影响 | ⬜ |

#### 3.3 platformio.ini 配置

| # | 任务 | 状态 |
|---|------|------|
| 3.1 | native 添加 lvgl lib_deps | ⬜ |
| 3.2 | build_src_filter 添加 UI 源 | ⬜ |
| 3.3 | build_src_filter 添加 native 文件 | ⬜ |
| 3.4 | build_flags 添加 BUILD_NATIVE | ⬜ |
| 3.5 | build_flags 添加 UI include 路径 | ⬜ |

#### 3.4 SDL2 显示后端

| # | 任务 | 状态 |
|---|------|------|
| 4.1 | 创建 tft_driver_native.cpp | ⬜ |
| 4.2 | 实现 push_pixels() RGB565→SDL2 | ⬜ |
| 4.3 | SDL2 窗口创建 240×280 | ⬜ |
| 4.4 | 窗口关闭事件处理 | ⬜ |

#### 3.5 LVGL 端口层

| # | 任务 | 状态 |
|---|------|------|
| 5.1 | 创建 lvgl_port_native.cpp | ⬜ |
| 5.2 | lv_display_create() 双缓冲 | ⬜ |
| 5.3 | flush_cb 回调 | ⬜ |
| 5.4 | tick_increment() std::chrono | ⬜ |
| 5.5 | task_handler() lv_timer_handler | ⬜ |

#### 3.6 主入口

| # | 任务 | 状态 |
|---|------|------|
| 6.1 | 创建 native_main_sim.cpp | ⬜ |
| 6.2 | 初始化 SDL2/TFT/LVGL | ⬜ |
| 6.3 | lof_power_system_init() | ⬜ |
| 6.4 | 主循环 SDL 事件处理 | ⬜ |
| 6.5 | 清理逻辑 | ⬜ |

#### 3.7 键盘输入（可选）

| # | 任务 | 状态 |
|---|------|------|
| 7.1 | 创建 keys_native.cpp | ⬜ |
| 7.2 | 1/2/3 键映射 K1/K2/K3 | ⬜ |
| 7.3 | KeyEvent 桥接 | ⬜ |

#### 3.8 集成验证

| # | 任务 | 状态 |
|---|------|------|
| 8.1 | pio run -e native 编译 | ⬜ |
| 8.2 | SDL2 窗口显示验证 | ⬜ |
| 8.3 | splash→home 切换验证 | ⬜ |
| 8.4 | 键盘输入验证 | ⬜ |

---

## 实施建议

### 优先级

1. **先完成 Phase 2** — 核心固件是基础，PC 模拟器依赖其 UI 代码
2. **Phase 3 可并行** — 当 Phase 2 的 UI 部分完成后，可开始 Phase 3

### 验证命令

```bash
# Phase 2 验证
pio run -e esp32s3          # 编译固件
pio run -e esp32s3 -t upload # 烧录
pio device monitor           # 串口监视

# Phase 3 验证
pio run -e native            # 编译 PC 模拟器
./.pio/build/native/program  # 运行模拟器
```

---

## 进度追踪

| Phase | 总任务 | 已完成 | 进度 |
|-------|--------|--------|------|
| Phase 1 | 16 | 16 | 100% |
| Phase 2 | 52 | 0 | 0% |
| Phase 3 | 31 | 0 | 0% |
| **总计** | **99** | **16** | **16%** |
