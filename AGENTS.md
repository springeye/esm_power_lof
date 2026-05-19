# Project Instructions

This file provides context for AI assistants working on this project.

## Project Type

ESP32-S3 嵌入式固件 — 基于 Arduino + FreeRTOS 框架，PlatformIO 构建系统，LVGL 9.x 图形界面，ST7789 240×280 显示屏。

## Build & Test Commands

```bash
# 编译固件（默认 esp32s3 环境）
pio run

# 编译指定环境（esp32s3 / test / release）
pio run -e esp32s3

# 编译并上传到 ESP32-S3
pio run -e esp32s3 -t upload

# 串口监视（含 ESP32 异常解码）
pio device monitor -e esp32s3

# 运行测试（Unity 框架，使用 test 环境）
pio test -e test

# 静态分析（cppcheck）
pio check -e esp32s3 --skip-packages

# 查看固件分区占用
pio run -e esp32s3 -t size

# Release 构建（含自动打包脚本）
pio run -e release

# Secure 构建（Flash Encryption dev-mode 加密打包，需先生成 secure_keys/flash_encryption_key.bin）
pio run -e secure
```

## Project Structure

```
esm_power_lof/
├── platformio.ini            # 主构建配置（4 环境：esp32s3 / test / release / secure）
├── include/                  # 全局头文件
│   ├── pins.h                # 引脚单点定义（ESP32-S3 N8R8）
│   ├── app_config.h          # 编译期常量（NTC/风扇/INA226/任务栈）
│   └── lv_conf.h             # LVGL 9.x 配置
├── src/                      # 主固件源码（C++17）
│   ├── main.cpp              # 固件入口（Arduino setup/loop）
│   ├── app/                  # 应用层：tasks, app_state, config_manager, watchdog, fault_guard
│   ├── hal/                  # 硬件抽象层：i2c_bus, spi_bus
│   ├── display/              # 显示驱动：tft_driver, lvgl_port, tft_demo
│   ├── sensors/              # 传感器：ntc（温度）, ina226（电流/电压）
│   ├── fan/                  # 风扇控制：fan_curve, fan_pwm, fan_tach
│   ├── power/                # 电源管理：psu_fsm, ps_on
│   ├── input/                # 按键输入：keys（去抖 + 短按/长按）
│   ├── wifi/                 # WiFi 管理
│   ├── web/                  # 内置 Web 管理界面
│   ├── ota/                  # 固件 OTA 升级
│   ├── ui_bridge/            # UI 胶水层：screen_manager, data_bridge, input_bridge, settings_ui 等
│   ├── compat/               # LVGL v8 兼容层
│   └── ui/_legacy/           # 旧版 UI（已排除构建，仅保留参考）
├── ui/                       # LVGL Editor 导出 UI
│   ├── screens/              # 屏幕定义（*.xml + *_gen.c/h 生成代码）
│   ├── fonts/                # 字体 C 数组
│   ├── lof_power_system.c/h  # 手写 UI 扩展入口
│   └── lof_power_system_gen.c/h  # 生成代码（勿手改）
├── test/                     # 单元测试（Unity 框架）
├── partitions/               # ESP32 分区表（default_8MB.csv）
├── scripts/                  # 辅助脚本（auto_src_filter.py, release_package.py, secure_package.py, provision_efuse.py）
├── openspec/                 # OpenSpec 规范驱动变更管理
└── release/                  # 固件 bin 发布产物
```

## Environments

| 环境 | 目标 | 说明 |
|------|------|------|
| `esp32s3` | 开发/调试固件 | 默认环境，含 WiFi/Web/OTA，启用调试 |
| `test` | 单元测试 | 排除 main.cpp/web/ota，运行 Unity 测试 |
| `release` | 发布固件 | 关闭 LVGL 调试，含 post-build 打包脚本（明文全量固件）|
| `secure` | 加密发布固件 | 克隆 `release`，Flash Encryption (dev-mode) 主机侧预加密打包，防 dump；详见 `docs/flash-encryption-guide.md` |

## Coding Conventions

- **语言标准**：C++17（`-std=c++17`），UI 层使用 C
- **命名约定**：
  - 宏/枚举值：`UPPER_SNAKE`（`PSU_OFF`, `EVT_BOOT`）
  - 类型（struct/enum/class）：`PascalCase`（`Ina226Data`, `KeyState`）
  - 函数/变量：`snake_case`（`get_temp_c`, `fan_pwm_set_duty`）
- **缩进**：C/C++ 文件用 4 空格，其他文件用 2 空格（见 `.editorconfig`）
- **行尾**：LF（Unix 风格）
- **编码**：UTF-8
- **Include 风格**：项目内部用 `"../module/header.h"` 相对路径，系统/库用 `<>`
- **跨线程数据**：所有多任务共享字段使用 `std::atomic`（定义在 `app/app_state.h`）
- **编译期配置**：常量通过 `include/app_config.h` 的 `static constexpr` 和 `platformio.ini` 的 `-D` 宏控制
- **模块边界**：每个 `.h` 只暴露模块公共 API，`.cpp` 隐藏实现细节
- **HAL 层**：所有硬件访问经 `hal/` 抽象，上层不直接操作寄存器
- **文件尾**：所有文件以换行结尾

## Important Notes

1. **引脚双重维护**：`include/pins.h`（C++ 宏）与 `platformio.ini` 的 `build_flags`（`-D` 宏）必须保持一致。修改引脚时两处都要更新。

2. **TFT_BL 陷阱**：绝对不要在全局宏中定义 `TFT_BL`。TFT_eSPI 的 `ST7789_Init.h` 使用 `#ifdef TFT_BL` 而非 `#if TFT_BL>=0`，一旦定义就会执行 `pinMode(-1)` 触发 GPIO 错误。背光由 `src/display/tft_driver.cpp` 通过 LEDC ch1 自行管理 GPIO21。

3. **Octal PSRAM 引脚**：N8R8 模组的 GPIO 33-37 被 Octal PSRAM 占用，**不可使用**。所有外设已避开此区域。

4. **生成文件勿手改**：`ui/screens/*_gen.c/h`、`ui/lof_power_system_gen.c/h`、`ui/preview-build/`、`ui/preview-bin/` 为生成产物。修改应通过上游 XML 或 LVGL Editor 工具完成。

5. **build_src_filter**：PlatformIO 使用 `build_src_filter` 按环境选择编译源文件。`src/ui/_legacy/` 已被排除（`-<ui/_legacy/>`），仅保留参考。新增源文件时需确认 `platformio.ini` 中各环境的过滤规则。

6. **UI 源码分属两处**：`src/ui_bridge/` 包含胶水层（桥接固件状态到 LVGL 控件），`ui/` 包含 LVGL Editor 导出的屏幕/字体/组件。

7. **测试框架**：Unity（`<unity.h>`）。测试入口在 `src/test_main.cpp`，测试用例在 `test/` 目录。通过 `pio test -e test` 运行。

8. **FreeRTOS 任务**：5 个任务（lvgl/sensor/ctrl/input/power），每个任务在循环开始调用 `watchdog::feed()`。任务栈大小在 `app_config.h` 配置。

9. **无 CI/CD**：项目当前无 CI/CD 配置，也无 WiFi/BLE/OTA 之外的无线功能。固件未经实际硬件验证。

## OpenSpec Workflow

本项目使用 OpenSpec 进行规范驱动变更管理。变更提案位于 `openspec/changes/`，包含 proposal.md、design.md、specs/ 和 tasks.md。

```bash
# 发起新变更 → 使用 openspec-propose skill
# 探索/讨论需求 → 使用 openspec-explore skill
# 实施变更 → 使用 openspec-apply-change skill
# 归档已完成变更 → 使用 openspec-archive-change skill
```
