## Context

当前项目是 ESP32-S3 风扇控制器固件，使用 LVGL 9.x + TFT_eSPI 在 ST7789 240×280 显示屏上运行 GUI。UI 代码由 LVGL Editor 生成（`ui/` 目录下的 `*_gen.c/h`），通过 `src/display/lvgl_port.cpp` 和 `src/display/tft_driver.cpp` 集成到固件。

PlatformIO 已有 `native` 环境（`[env:native]`），但仅用于单元测试（NTC、风扇曲线、PSU 状态机、按键），不包含 LVGL 库或 UI 源码。

关键约束：
- `include/lv_conf.h` 使用 `LV_TICK_CUSTOM=1` 且绑定 Arduino `millis()`，native 编译会失败
- UI 生成源（`ui/*.c`）是纯 C 代码，不依赖 MCU 特性，可直接在 PC 编译
- `LV_USE_XML=0`，已有 `lvgl_xml_stubs.c` 提供弱符号 stub
- 项目使用 MSYS2/MinGW 环境（`scripts/use_msys2_mingw.py`），可安装 SDL2

## Goals / Non-Goals

**Goals:**
- 在 PC 上运行 LVGL UI，实时预览 240×280 窗口
- 复用 LVGL Editor 生成的 UI 源码，无需修改
- 支持键盘输入映射到 LVGL 事件（K1/K2/K3）
- 构建命令简单：`pio run -e native` 或 `pio test -e native`

**Non-Goals:**
- 不模拟 TFT_eSPI 的 DMA 传输细节
- 不模拟 INA226/NTC 等硬件传感器（使用 mock 数据）
- 不实现触摸屏输入
- 不追求与设备完全一致的帧率（PC 上 30fps 即可）

## Decisions

### D1：显示后端选择 SDL2

**选择**：SDL2 作为 PC 显示后端

**理由**：
- 跨平台（Windows/Linux/macOS）
- LVGL 官方提供 SDL2 驱动示例
- 支持键盘/鼠标输入
- MSYS2 环境可直接安装：`pacman -S mingw-w64-x86_64-SDL2`

**备选**：
- PPM 文件输出 → 放弃，无法实时交互
- Qt/GTK → 依赖过重，LVGL 官方无示例

### D2：lv_conf.h 条件化策略

**选择**：使用 `BUILD_NATIVE` 宏进行条件编译

```c
#if defined(BUILD_NATIVE)
    // PC 端配置
    #define LV_TICK_CUSTOM 0  // 不使用自定义 tick，由 native 代码手动调用 lv_tick_inc
#else
    // ESP32 配置（保持不变）
    #define LV_TICK_CUSTOM 1
    #define LV_TICK_CUSTOM_INCLUDE "Arduino.h"
    #define LV_TICK_CUSTOM_SYS_TIME_EXPR (millis())
#endif
```

**理由**：
- 最小化对现有配置的修改
- `LV_TICK_CUSTOM=0` 时 LVGL 不自动推进 tick，由 native 代码通过 `std::chrono` 手动调用 `lv_tick_inc()`
- 保持 ESP32 构建完全不变

### D3：native 环境的 LVGL 依赖方式

**选择**：在 `platformio.ini` 的 `[env:native]` 中添加 `lib_deps = lvgl/lvgl@~9.2.2`

**理由**：
- PlatformIO 的 `native` 平台支持从 PlatformIO Registry 拉取库
- 与 ESP32 环境使用相同版本，确保 UI 兼容
- LVGL 源码会自动下载到 `.pio/libdeps/native/`

**备选**：
- 将 LVGL 源码放入 `lib/` 目录 → 增加仓库体积，版本管理麻烦

### D4：UI 源码纳入方式

**选择**：在 `[env:native]` 的 `build_src_filter` 中添加 UI 源文件

```ini
build_src_filter =
    +<*>
    +<../ui/lof_power_system.c>
    +<../ui/lof_power_system_gen.c>
    +<../ui/screens/home_gen.c>
    +<../ui/screens/splash_gen.c>
    +<../ui/fonts/hos_14_data.c>
    +<../ui/fonts/hos_bold_big_data.c>
    +<../ui/fonts/hos_regular_data.c>
    +<../src/compat/lvgl_v8_shim.cpp>
    +<../src/ui_bridge/lvgl_xml_stubs.c>
    +<display/tft_driver_native.cpp>
    +<display/lvgl_port_native.cpp>
    +<native/native_main_sim.cpp>
    -<native/native_main.cpp>  ; 排除单元测试入口
```

**理由**：
- 与 ESP32 环境保持一致的包含方式
- 明确列出每个 UI 源文件，避免意外包含不需要的文件
- 新增的 native 专用文件也通过 filter 控制

### D5：SDL2 集成方式

**选择**：通过 PlatformIO 的 `build_flags` 和 `extra_scripts` 链接 SDL2

**Windows (MSYS2) 方案**：
```ini
[env:native]
build_flags =
    -DBUILD_NATIVE
    -Iinclude -Isrc -Iui -Iui/screens -Iui/fonts
    -std=c++17
    ; SDL2 通过 pkg-config 或手动指定
    !pkg-config --cflags sdl2
extra_scripts = pre:scripts/use_msys2_mingw.py
```

**Linux 方案**：
```ini
build_flags =
    -DBUILD_NATIVE
    $(pkg-config --cflags sdl2)
build_unflags = -std=gnu++11
```

**理由**：
- 利用系统包管理器安装 SDL2，无需在仓库中捆绑
- `pkg-config` 自动处理 include/lib 路径
- `extra_scripts` 已有 MSYS2 支持，可复用

### D6：入口文件结构

**新增文件**：
- `src/display/tft_driver_native.cpp` — SDL2 显示后端，实现 `push_pixels()`
- `src/display/lvgl_port_native.cpp` — LVGL 初始化、tick、flush 回调
- `src/native/native_main_sim.cpp` — 主入口，SDL 事件循环

**接口复用**：
- `tft_driver.h` 和 `lvgl_port.h` 的接口保持不变
- native 实现通过条件编译（`#ifdef BUILD_NATIVE`）或独立文件实现

## Risks / Trade-offs

- [SDL2 安装依赖] → 在 README 中说明 MSYS2 安装命令；Linux 需要 `libsdl2-dev`
- [lv_conf.h 条件化可能遗漏宏] → 逐一检查 `LV_TICK_CUSTOM` 相关宏，确保 native 下无 Arduino 依赖
- [UI 生成源更新后需同步 native build_src_filter] → 可通过脚本自动提取 `ui/file_list_gen.cmake` 中的源文件列表
- [SDL2 链接在 Windows 上可能复杂] → 已有 `scripts/use_msys2_mingw.py` 可复用，降低风险

## Migration Plan

1. 安装 SDL2：
   - Windows (MSYS2): `pacman -S mingw-w64-x86_64-SDL2`
   - Linux: `sudo apt install libsdl2-dev`
   - macOS: `brew install sdl2`

2. 修改 `platformio.ini`，添加 `[env:native]` 的 LVGL 依赖和 UI 源

3. 修改 `include/lv_conf.h`，添加 `BUILD_NATIVE` 条件化

4. 新增 3 个 native 专用文件

5. 运行 `pio run -e native` 验证编译

6. 运行生成的可执行文件，验证 SDL2 窗口显示 UI

**回滚**：删除新增文件，恢复 `platformio.ini` 和 `lv_conf.h` 的修改

## Open Questions

1. SDL2 在 Windows MSYS2 环境下的链接方式是否需要额外配置 `LDLIBS`？
2. 是否需要为 native 环境单独维护一份 `lv_conf.h`，还是在同一文件中条件化？
3. 键盘映射方案：SDL2 keycode → LVGL key 的对应关系需要确认
