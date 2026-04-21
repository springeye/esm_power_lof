## Why

需要从零搭建一个 ESP32-WROOM-32E-N8（Arduino framework + PlatformIO）的电源/散热监控控制器固件，用于实时采集 NTC 温度和三路 INA226 电压电流，按温度闭环控制 4 线 PWM 风扇，并通过 ST7789P3 240×280 LVGL 界面显示状态、通过按键与 PSON/PWOK 信号控制下游开关电源。当前仓库为空，必须先建立可编译运行的项目骨架，后续业务才能迭代。

## What Changes

- 新建 PlatformIO 项目结构（`platformio.ini`、`src/`、`include/`、`lib/`、`.gitignore`），目标板 `esp32dev`，framework `arduino`，flash 配置匹配 N8（8MB）。
- 引入并配置依赖：`TFT_eSPI`（ST7789 240×280 4-SPI）、`lvgl` 9.x、`Wire`、`INA226` 库（如 `robtillaart/INA226`）。
- 建立模块化代码分层：`hal/`（引脚与外设抽象）、`display/`（TFT + LVGL port + 双缓冲）、`ui/`（LVGL 屏幕与控件，抗锯齿字体、无中文）、`sensors/ntc/`、`sensors/ina226/`（三路）、`fan/`（25kHz PWM + 转速测量）、`power/`（PSON 输出 + PWOK 输入 + 防抖）、`input/`（K1/K2/K3 按键）、`app/`（任务调度与温控策略）。
- 实现 NTC 温度采样（ADC1，β 公式）→ 温控曲线 → LEDC 25kHz PWM 风扇调速 + 测速捕获。
- 实现三路 INA226（I2C 共享总线，地址 0x40/0x41/0x44，2mΩ shunt）周期采样并上报到 UI。
- 实现 PSON（输出，低有效）开关下游电源、PWOK（输入）状态监测与上电时序。
- 实现 LVGL 9.x 双缓冲（1/10 屏 = 240×28）刷新，使用抗锯齿字体（Montserrat 16/24，4bpp），仅英文/数字。
- 提供基础 README 与构建/烧录说明。

## Capabilities

### New Capabilities
- `project-scaffold`: PlatformIO + Arduino + ESP32 项目骨架、依赖、构建配置与目录结构。
- `display-lvgl`: ST7789P3 240×280 4-SPI 驱动接入与 LVGL 9.x 双缓冲移植，抗锯齿英文字体。
- `ui-dashboard`: 主仪表盘 UI（温度、风扇转速/占空比、三路电压电流功率、电源状态、按键操作）。
- `temperature-sensing`: NTC 热敏电阻采样、滤波、温度换算与告警阈值。
- `fan-control`: 4 线风扇 25kHz PWM 调速 + 转速反馈 + 温控闭环策略。
- `power-rail-monitoring`: 三路 INA226 电压/电流/功率采集与共享 I2C 总线管理。
- `power-switch-control`: PSON 输出控制 + PWOK 输入监测 + 上电/掉电时序与故障保护。
- `user-input`: K1/K2/K3 按键扫描、消抖与事件分发（短按/长按）。

### Modified Capabilities
（无现有 spec）

## Impact

- 代码：全新仓库，新增 PlatformIO 工程与上述全部模块源码。
- 构建：新增 `platformio.ini`，托管依赖（TFT_eSPI、lvgl、INA226 库），需在 `lib/TFT_eSPI/User_Setups/` 或通过 build_flags 提供 `User_Setup` 以匹配 ST7789 240×280 与 4-SPI 引脚。
- 硬件契约：固定的 GPIO 分配（详见 design.md），需与硬件原理图一致。
- 资源：LVGL 双缓冲约 26.4KB SRAM；25kHz PWM 占用一个 LEDC 通道；I2C 与 SPI 各占一组外设。
- 依赖外部库版本需在 `platformio.ini` 锁定，避免后续不兼容。
