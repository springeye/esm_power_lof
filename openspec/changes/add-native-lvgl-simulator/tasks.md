## 1. 环境准备

- [ ] 1.1 安装 SDL2 开发库（Windows MSYS2: `pacman -S mingw-w64-x86_64-SDL2`，Linux: `sudo apt install libsdl2-dev`）
- [ ] 1.2 验证 SDL2 安装：`pkg-config --cflags --libs sdl2` 能正确输出

## 2. lv_conf.h 条件化

- [ ] 2.1 在 `include/lv_conf.h` 顶部添加 `BUILD_NATIVE` 条件编译块
- [ ] 2.2 将 `LV_TICK_CUSTOM` 相关配置包裹在 `#if !defined(BUILD_NATIVE)` 中
- [ ] 2.3 验证 ESP32 构建不受影响：`pio run -e esp32s3` 编译通过

## 3. platformio.ini 配置

- [ ] 3.1 在 `[env:native]` 添加 `lib_deps = lvgl/lvgl@~9.2.2`
- [ ] 3.2 更新 `build_src_filter` 添加 UI 源文件（`ui/*.c`、`fonts/*.c`）
- [ ] 3.3 更新 `build_src_filter` 添加新增的 native 专用文件
- [ ] 3.4 更新 `build_flags` 添加 `-DBUILD_NATIVE` 和 SDL2 编译参数
- [ ] 3.5 更新 `build_flags` 添加 `-Iui -Iui/screens -Iui/fonts` 包含路径

## 4. SDL2 显示后端

- [ ] 4.1 创建 `src/display/tft_driver_native.cpp`，实现 `init()` 和 `set_backlight()`
- [ ] 4.2 实现 `push_pixels()`：将 RGB565 数据转换为 SDL2 纹理并渲染
- [ ] 4.3 实现 SDL2 窗口创建（240×280，标题 "LVGL Simulator"）
- [ ] 4.4 实现窗口关闭事件处理（SDL_QUIT）

## 5. LVGL 端口层

- [ ] 5.1 创建 `src/display/lvgl_port_native.cpp`，实现 `init()` 调用 `lv_init()`
- [ ] 5.2 实现 `lv_display_create()` 和双缓冲配置（240×28 每缓冲）
- [ ] 5.3 实现 `flush_cb` 回调，调用 `tft_driver::push_pixels()`
- [ ] 5.4 实现 `tick_increment()` 使用 `std::chrono` 推进 LVGL tick
- [ ] 5.5 实现 `task_handler()` 调用 `lv_timer_handler()`

## 6. 主入口

- [ ] 6.1 创建 `src/native/native_main_sim.cpp`，实现 `main()` 函数
- [ ] 6.2 初始化 SDL2、TFT driver、LVGL port
- [ ] 6.3 调用 `lof_power_system_init(NULL)` 初始化 UI
- [ ] 6.4 实现主循环：处理 SDL 事件、推进 tick、调用 `lv_timer_handler()`
- [ ] 6.5 实现清理逻辑：SDL_DestroyRenderer/Window/Quit

## 7. 键盘输入（可选）

- [ ] 7.1 创建 `src/input/keys_native.cpp`，映射 SDL 键盘事件到 KeyEvent
- [ ] 7.2 实现 '1'/'2'/'3' 键映射到 K1/K2/K3
- [ ] 7.3 将 KeyEvent 桥接到 `ui_bridge::input_handle_key()`

## 8. 集成验证

- [ ] 8.1 运行 `pio run -e native` 验证编译通过
- [ ] 8.2 运行生成的可执行文件，验证 SDL2 窗口显示 LVGL UI
- [ ] 8.3 验证 splash → home 屏幕切换正常
- [ ] 8.4 验证键盘输入响应（如已实现）
