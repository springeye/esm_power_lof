# ESP32-S3 智能风扇控制器固件

基于 **ESP32-S3-WROOM-1-N8R8**（8MB Quad Flash + 8MB Octal PSRAM）的智能风扇控制器，支持温度监测、PWM 风扇调速、电源管理和 LVGL 显示界面。

## 目标硬件

| 组件 | 型号 | 说明 |
|------|------|------|
| 主控 | ESP32-S3-WROOM-1-N8R8 | 8MB Quad Flash + 8MB Octal PSRAM，双核 240MHz |
| 显示屏 | ST7789 240×280 | FSPI 接口（IO_MUX 直连），16位色深 |
| 温度传感器 | NTC 10kΩ B=3950 | GPIO1 (ADC1_CH0) 采集 |
| 电流传感器 | INA226 ×3 | I2C 地址 0x40/0x41/0x44，共直流输出轨 |
| 风扇 | 4线 PWM 风扇 | 25kHz PWM，PCNT 硬件测速 |
| 电源 | ATX 电源 | PS_ON/PW_OK 控制 |

## 引脚分配

> **重要**：N8R8 模组的 Octal PSRAM 占用 GPIO 33-37，这些引脚**不可使用**。所有外设均避开此区域。

### TFT 显示屏（FSPI，IO_MUX 直连）

| 功能 | GPIO | 说明 |
|------|------|------|
| TFT_MOSI | 11 | FSPI MOSI（数据输出） |
| TFT_SCLK | 12 | FSPI SCLK（时钟） |
| TFT_MISO | 9 | FSPI MISO（占空闲脚，单向 SPI 不用） |
| TFT_CS | 10 | FSPI CS（片选） |
| TFT_DC | 13 | 数据/命令选择 |
| TFT_RST | 14 | 硬件复位 |
| TFT_BL | 21 | 背光 PWM（LEDC），避开 GPIO19/20 USB 引脚 |

### I2C 总线（INA226 ×3）

| 功能 | GPIO | 说明 |
|------|------|------|
| I2C_SDA | 41 | I2C 数据线 |
| I2C_SCL | 42 | I2C 时钟线 |

> GPIO41/42 为 S3 专属引脚，远离 Flash/PSRAM 区域。

### 风扇控制

| 功能 | GPIO | 说明 |
|------|------|------|
| FAN_PWM | 16 | 风扇 PWM 输出（LEDC） |
| FAN_TACH | 17 | 风扇转速输入（PCNT，内部上拉） |

### NTC 温度传感器

| 功能 | GPIO | 说明 |
|------|------|------|
| NTC_ADC | 1 | ADC1_CH0，不受 WiFi 干扰 |

### ATX 电源控制

| 功能 | GPIO | 说明 |
|------|------|------|
| PSON_PIN | 15 | 电源开关输出（低=开机，高=关机） |
| PWOK_PIN | 18 | 电源就绪输入（高=正常） |

### 按键输入

| 功能 | GPIO | 说明 |
|------|------|------|
| KEY_K1（上键） | 38 | 低电平有效，内部上拉 |
| KEY_K2（确认键） | 39 | 低电平有效，内部上拉 |
| KEY_K3（下键） | 40 | 低电平有效，内部上拉 |

### GPIO 汇总

```
GPIO 1  - NTC 温度 ADC（ADC1_CH0）
GPIO 9  - TFT MISO（FSPI Q，占位脚）
GPIO 10 - TFT 片选（FSPI CS）
GPIO 11 - TFT 数据（FSPI MOSI）
GPIO 12 - TFT 时钟（FSPI SCLK）
GPIO 13 - TFT 数据/命令（DC）
GPIO 14 - TFT 复位（RST）
GPIO 15 - 电源开关输出（PSON）
GPIO 16 - 风扇 PWM（LEDC）
GPIO 17 - 风扇测速（PCNT）
GPIO 18 - 电源就绪输入（PWOK）
GPIO 19 - [保留] USB D-
GPIO 20 - [保留] USB D+
GPIO 21 - TFT 背光 PWM（LEDC）
GPIO 33-37 - [禁用] Octal PSRAM 占用
GPIO 38 - 上键（KEY_K1）
GPIO 39 - 确认键（KEY_K2）
GPIO 40 - 下键（KEY_K3）
GPIO 41 - I2C 数据（SDA）
GPIO 42 - I2C 时钟（SCL）
```

## 目录结构

```
esm_power_lof/
├── platformio.ini            # 主构建配置（env: esp32s3）
├── include/                  # 全局头文件与配置
│   ├── pins.h                # 引脚单点定义（ESP32-S3 N8R8）
│   ├── app_config.h          # 全局常量（NTC/风扇/INA226/任务栈）
│   └── lv_conf.h             # LVGL 9.x 配置
├── src/                      # 主固件源码（48 文件，含 _legacy/ 共 54）
│   ├── main.cpp              # ESP32-S3 固件入口（Arduino setup/loop）
│   ├── app/                  # 应用层
│   │   ├── app_state.{h,cpp} # 全局状态（原子变量，跨任务共享）
│   │   ├── config_manager.{h,cpp}  # 运行时配置管理（NVS持久化）
│   │   ├── tasks.{h,cpp}     # FreeRTOS 任务启动（lvgl/sensor/ctrl/input/power）
│   │   ├── watchdog.{h,cpp}  # 任务看门狗
│   │   └── fault_guard.{h,cpp} # 故障保护（过温/堵转/过流/PWOK）
│   ├── hal/                  # 硬件抽象层
│   │   ├── i2c_bus.{h,cpp}   # I2C 总线抽象
│   │   └── spi_bus.{h,cpp}   # SPI 总线抽象
│   ├── display/              # 显示驱动
│   │   ├── tft_driver.{h,cpp}      # TFT_eSPI 初始化 + LEDC 背光
│   │   ├── tft_demo.{h,cpp}        # 底层测试图案（彩条/文本）
│   │   └── lvgl_port.{h,cpp}       # LVGL 移植层（display flush + tick）
│   ├── sensors/              # 传感器
│   │   ├── ntc/              # NTC 温度传感器（β 公式 + 中值滤波）
│   │   └── ina226/           # INA226 电流/电压/功率监测
│   ├── fan/                  # 风扇控制
│   │   ├── fan_curve.{h,cpp} # 温控曲线（温度→PWM）
│   │   ├── fan_pwm.{h,cpp}   # LEDC PWM 输出
│   │   └── fan_tach.{h,cpp}  # PCNT 硬件测速
│   ├── power/                # 电源管理
│   │   ├── psu_fsm.{h,cpp}   # ATX 电源状态机（6 态）
│   │   └── ps_on.{h,cpp}     # PS_ON 引脚控制
│   ├── input/                # 按键输入
│   │   └── keys.{h,cpp}      # 按键去抖 + 短按/长按事件
│   ├── app/                  # 应用层
│   │   ├── app_state.{h,cpp} # 全局状态（原子变量，跨任务共享）
│   │   ├── tasks.{h,cpp}     # FreeRTOS 任务启动（lvgl/sensor/ctrl/input/power）
│   │   ├── watchdog.{h,cpp}  # 任务看门狗
│   │   └── fault_guard.{h,cpp} # 故障保护（过温/堵转/过流/PWOK）
│   ├── ui_bridge/            # UI 胶水层
│   │   ├── screen_manager.{h,cpp}  # 屏幕切换管理
│   │   ├── data_bridge.{h,cpp}     # 应用状态→UI 控件数据绑定
│   │   └── input_bridge.{h,cpp}    # 按键事件→LVGL 事件转发
│   ├── compat/               # LVGL v8 兼容层
│   │   └── lvgl_v8_shim.cpp  # v8 API → v9 API 适配
│   └── ui/_legacy/           # 旧版 UI（已排除构建，保留参考）
│       ├── ui_events.{h,cpp}
│       ├── ui_main.{h,cpp}
│       └── ui_menu.{h,cpp}
├── ui/                       # LVGL Editor 导出 UI（含 AGENTS.md）
│   ├── screens/              # 屏幕定义：home.xml, splash.xml + 生成代码 *_gen.c/h
│   ├── fonts/                # 字体数据（C 数组）：hos_14/regular/bold_big
│   ├── images/               # 图片资源
│   ├── components/           # 组件文件
│   ├── widgets/              # 控件定义
│   ├── lof_power_system.{c,h}      # 手写 UI 扩展入口（桥接生成逻辑）
│   ├── lof_power_system_gen.{c,h}  # LVGL Editor 生成代码（勿手改！）
│   ├── project.xml           # LVGL Editor 项目描述
│   ├── globals.xml           # 全局变量定义
│   ├── CMakeLists.txt        # Emscripten 预览构建入口
│   ├── preview-build/        # CMake 构建中间产物（勿手改）
│   └── preview-bin/          # 生成的预览 WASM/JS（勿手改）
├── test/                     # 单元测试
├── partitions/               # ESP32 分区表
│   └── default_8MB.csv       # 8MB Flash：nvs, otadata, app0/1(OTA), spiffs, coredump
├── scripts/                  # 辅助脚本
├── openspec/                 # OpenSpec 规范驱动变更管理
│   └── changes/              # 变更提案（proposal/design/spec/tasks）
├── .editorconfig             # 编辑器配置（UTF-8, LF, 缩进）
└── AGENTS.md                 # AI Agent 项目知识库
```

## 构建与开发

### 环境（platformio.ini）

| 环境 | 目标 | 说明 |
|------|------|------|
| `esp32s3` | ESP32-S3 固件 | 默认构建目标，完整固件 |

### 命令

```bash
# 编译固件（默认 esp32s3）
pio run

# 编译指定环境
pio run -e esp32s3

# 静态分析（cppcheck）
pio check -e esp32s3 --skip-packages

# 查看固件大小
pio run -e esp32s3 -t size

# 烧录固件
pio run -e esp32s3 -t upload

# 串口监视
pio device monitor -e esp32s3
```

## 功能特性

- **温度监测**：NTC 10kΩ 热敏电阻，β 公式计算（B=3950），16 次采样取中值滤波
- **风扇调速**：三段分段线性温控曲线（30-50-70°C），10-bit PWM 25kHz，2°C 滞回防抖
- **强制满速**：75°C 紧急散热，风扇全速输出
- **转速监测**：PCNT 硬件计数器，4 线风扇每转 2 脉冲，RPM 实时显示
- **堵转保护**：占空比 >30% 时转速异常持续 3s → 5s 后自动关机
- **电源管理**：ATX 电源 6 态状态机（Off → Standby → Starting → On → Stopping → Fault）
- **电流监测**：3 路 INA226 并联监测同一直流输出电压轨，独立计量各接口电流（0-40A，2mΩ 分流电阻）
- **故障保护**：过温关机 / 风扇堵转关机 / 过流保护 / PWOK 失稳关机（100ms 消抖）
- **运行时设置**：通过3个物理按键调整风扇曲线、温度阈值、亮度、功率配置和传感器校准，NVS持久化保存
- **显示界面**：LVGL 9.x，ST7789 240×280，主仪表盘（温度/转速/功率/电压），40MHz SPI
- **按键交互**：3 键导航（上/确认/下），5ms 轮询去抖，800ms 长按判定，2s 超长按关机
- **任务看门狗**：5 个 FreeRTOS 任务均有看门狗保护，5s 超时自动复位

## 设置页面

通过长按 K2（确认键）进入设置页面，包含 5 个分类：

| 分类 | 设置项 | 范围 | 默认值 |
|------|--------|------|--------|
| 风扇设置 | 低温阈值 | 20-50°C | 35°C |
| 风扇设置 | 中温阈值 | 30-60°C | 45°C |
| 风扇设置 | 高温阈值 | 40-70°C | 55°C |
| 风扇设置 | 强制阈值 | 50-80°C | 60°C |
| 风扇设置 | 最低转速% | 0-100% | 20% |
| 风扇设置 | 中间转速% | 0-100% | 60% |
| 风扇设置 | 滞回温度 | 0.5-5.0°C | 2.0°C |
| 温度保护 | 警告阈值 | 50-80°C | 65°C |
| 温度保护 | 关机阈值 | 60-90°C | 75°C |
| 显示设置 | 亮度% | 10-100% | 80% |
| 功率配置 | 设计功率W | 350/450/550/750W | 750W |
| 传感器校准 | 温度偏移°C | -5.0~5.0°C | 0.0°C |

### 按键操作
- **K1/K3**：移动焦点（循环）
- **K2 短按**：进入/确认编辑
- **编辑态 K1/K3**：调整数值
- **长按 K1/K3**：切换页面（循环）
- **长按 K2**：返回主页面

## 技术栈

- **语言**：C++17 / C（UI 层）
- **框架**：Arduino + FreeRTOS
- **构建**：PlatformIO
- **UI 引擎**：LVGL 9.x（ESP32-S3: ~9.5.0；UI 由 LVGL Editor 导出）
- **测试**：Unity 框架（`<unity.h>`）
- **静态检查**：cppcheck（`--enable=warning,style,performance --inconclusive`）
- **外部库**：TFT_eSPI, INA226, Bounce2

## 字体优化

- `hos_14`：14px 常规字体，用于状态栏、通道标签等关键信息
- `hos_regular`：16px 常规字体，用于通道数值等重要数据
- `hos_bold_big`：44px 粗体，用于主功率数字显示
- 所有字体使用 8bpp 抗锯齿，确保在 240×280 屏幕上的清晰度

## 项目约定

- **引脚双重维护**：`include/pins.h`（C++ 宏）与 `platformio.ini` build_flags（`-D` 宏）必须保持一致
- **生成文件勿手改**：`ui/screens/*_gen.c/h`、`ui/lof_power_system_gen.c/h` 和 `preview-build/`、`preview-bin/` 为生成产物
- **跨线程数据**：所有多任务共享字段使用 `std::atomic`
- **编译期配置**：所有常量通过 `app_config.h` 的 `static constexpr` 和 `-D` flags 控制
- **UI 显示调试开关**：`USE_DISPLAY_DEMO = true` 仅运行 TFT 底层测试图案，不启动 UI/任务
- **遗留代码排除**：`src/ui/_legacy/` 已通过 `build_src_filter` 排除构建（`-<ui/_legacy/>`），仅保留参考
- **命名约定**：宏 `UPPER_SNAKE`，类型 `PascalCase`，函数/变量 `snake_case`

## 注意事项

- 本固件仅经过编译验证，**未经实际硬件验证**
- 烧录前请仔细确认引脚连接，避免损坏硬件
- N8R8 模组的 GPIO 33-37 被 Octal PSRAM 占用，切勿连接到这些引脚
- 不要在全局宏中定义 `TFT_BL`（会导致 TFT_eSPI 触发 `pinMode(-1)` 错误），背光由 `tft_driver.cpp` 自行管理
- 不支持 WiFi/BLE/OTA（设计层面排除）；NVS 持久化仅用于运行时配置保存
- 无 CI/CD 配置（建议添加 GitHub Actions：`pio run` + `pio check`）

## OpenSpec 工作流

本项目使用 OpenSpec 进行规范驱动的变更管理：

```bash
# 发起新变更
/opsx-propose

# 探索/讨论需求
/opsx-explore

# 实施变更
/opsx-apply

# 归档已完成变更
/opsx-archive
```

变更提案位于 `openspec/changes/` 目录，包含 proposal.md（需求）、design.md（设计决策）、specs/（功能规格）和 tasks.md（实现任务）。

## 许可证

MIT License
