## Why

当前项目仅支持在 ESP32-S3 硬件上运行 LVGL UI。每次 UI 调整都需要烧录到设备才能验证，迭代周期长。需要在 PC 上原生运行 LVGL 模拟器，实现 UI 的快速开发和调试，无需依赖硬件。

## What Changes

- 在 PlatformIO 的 `native` 环境中集成 LVGL 9.x 库和 LVGL Editor 生成的 UI 源码
- 新增 SDL2 显示后端，实现 `flush_cb` 将 LVGL 渲染输出到 PC 窗口
- 新增主机端 tick 实现（替代 Arduino `millis()`）
- 条件化 `lv_conf.h`，使其同时支持 ESP32 和 PC 编译
- 新增 `native_main_sim.cpp` 作为 PC 模拟器入口
- 支持键盘输入映射到 LVGL 事件（可选）

## Capabilities

### New Capabilities
- `native-lvgl-simulator`: PC 端 LVGL 模拟器，使用 SDL2 显示后端，支持实时 UI 预览和键盘交互

### Modified Capabilities
（无现有 spec 需要修改）

## Impact

- **代码**：新增 3 个文件（`tft_driver_native.cpp`、`lvgl_port_native.cpp`、`native_main_sim.cpp`），修改 2 个文件（`platformio.ini`、`lv_conf.h`）
- **依赖**：native 环境需要 SDL2 开发库（通过 MSYS2 或系统包管理器安装）
- **构建**：`platformio.ini` 的 `[env:native]` 需要添加 LVGL 依赖和 UI 源文件路径
- **兼容性**：`lv_conf.h` 需要条件化处理，确保 ESP32 构建不受影响
