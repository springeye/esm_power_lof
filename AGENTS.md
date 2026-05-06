# PROJECT KNOWLEDGE BASE

**Generated:** 2026-05-06
**Commit:** 7dfd47d
**Branch:** master

## OVERVIEW
ESP32 智能风扇控制器固件。C++/Arduino 框架 + PlatformIO 构建，对偶 LVGL Editor 导出的 UI 子工程（C/CMake + Emscripten 预览）。

## STRUCTURE
```
esm_power_lof/
├── platformio.ini          # 主构建配置（envs: esp32s3, native, native-smoke）
├── include/                # 全局头文件与配置
│   ├── pins.h              # 引脚单点定义（ESP32-S3）
│   ├── app_config.h        # 全局常量（NTC/风扇/INA226/任务栈）
│   ├── lv_conf.h           # LVGL 9.x 配置
│   ├── lvgl_v8_shim.h      # LVGL v8 兼容层头文件
│   ├── lvgl/               # LVGL 头文件桩（native 构建用）
│   └── SDL2/               # SDL2 头文件桩（native 构建用）
├── src/                    # 主固件源码（61 文件）
│   ├── main.cpp            # ESP32 固件入口（Arduino setup/loop）
│   ├── app/                # 应用层（tasks, app_state, watchdog, fault_guard, config_manager）
│   ├── hal/                # 硬件抽象（I2C, SPI）
│   ├── display/            # 显示驱动（TFT_eSPI + LVGL port + native 替身）
│   ├── sensors/            # 传感器（ntc/, ina226/）
│   ├── fan/                # 风扇控制（fan_curve, fan_pwm, fan_tach）
│   ├── power/              # 电源管理（PSU FSM, PS_ON）
│   ├── input/              # 按键输入（keys debounce）
│   ├── ui_bridge/          # UI 胶水层（screen_manager, data_bridge, input_bridge, splash_anim, settings_ui）
│   ├── compat/             # LVGL 兼容层（v8 shim + v9.5 compat）
│   ├── ui/_legacy/         # 旧版 UI（已排除构建，保留参考）
│   └── native/             # 本地模拟器入口（native_main_sim.cpp + smoke test）
├── ui/                     # LVGL Editor 导出 UI（已有 AGENTS.md）
│   ├── screens/            # 屏幕 XML + 生成 *_gen.c/h（home, splash, settings）
│   ├── fonts/              # 字体数据（C 数组，.ttf 源文件）
│   ├── lof_power_system.c  # 手写 UI 扩展入口
│   ├── lof_power_system_gen.c/h  # LVGL Editor 生成代码（勿手改）
│   ├── globals.xml         # 全局变量与字体 symbol 定义
│   ├── CMakeLists.txt      # 预览构建入口（Emscripten）
│   ├── preview-build/      # 生成：CMake 中间产物
│   └── preview-bin/        # 生成：预览 runtime.js/wasm
├── test/native/            # Unity 单元测试（native 平台）
├── partitions/             # 固件分区表（8MB Flash CSV）
├── scripts/                # 辅助脚本（MSYS2/MinGW 注入）
└── openspec/               # 规范驱动变更提案（proposal/design/spec/tasks）
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| 固件入口 | `src/main.cpp` | `setup()` 初始化外设，`loop()` 空转——所有逻辑在 FreeRTOS 任务中 |
| 构建/环境选择 | `platformio.ini` | `build_src_filter` 控制哪些源参与编译，`build_flags` 设置编译宏 |
| 修改配置常量 | `include/app_config.h` | NTC参数、风扇阈值、任务栈、`USE_DISPLAY_DEMO` 开关 |
| 修改引脚 | `include/pins.h` | 与 `platformio.ini` build_flags 中的 TFT 引脚保持一致 |
| 修改默认字体 | `include/lv_conf.h` | `LV_FONT_DEFAULT` = `&lv_font_montserrat_14`（仅作后备，UI 中不使用） |
| 可用字体 | `ui/fonts/` | `hos_14`, `hos_regular`, `hos_bold_big`, `hos_bold_splash`, `hos_medium`, `hos_bold`, `hos_black`, `hos_light`, `hos_thin`, `font_medium`, `font_awesome_14`, `font_awesome_48` |
| UI 生成代码（勿手改） | `ui/screens/*_gen.c/h` + `ui/lof_power_system_gen.*` | 修改对应 XML 后重新生成 |
| UI 手写扩展 | `ui/lof_power_system.c` | `lof_power_system_init()` 桥接生成逻辑 |
| 屏幕定义（可编辑） | `ui/screens/*.xml` | 修改后重新生成 `*_gen.c/h`；现有 home.xml、splash.xml、settings.xml |
| 启动动画 | `src/ui_bridge/splash_anim.cpp` | splash 屏幕动画与定时切换逻辑 |
| 本地模拟器 | `src/native/native_main_sim.cpp` | SDL2 LVGL 模拟器，`-DBUILD_NATIVE` |
| 头文件模拟 | `src/native_main.cpp` | CLI 模拟器，用于算法功能测试 |
| 单元测试 | `test/native/<suite>/test_main.cpp` | Unity 框架，`pio test -e native` |
| 状态机实现 | `src/power/psu_fsm.cpp` | `psu_fsm_transition()` |
| 风扇曲线 | `src/fan/fan_curve.cpp` | `fan_temp_to_pwm()`, `hysteresis_apply()` |
| 任务划分 | `src/app/tasks.cpp` | `tasks::start_all()` 启动 5 个 FreeRTOS 任务 |
| 故障检测 | `src/app/fault_guard.cpp` | 过温/堵转/过流/PWOK 失稳保护 |
| 运行时配置 | `src/app/config_manager.{h,cpp}` | 风扇曲线、温度阈值、亮度、功率、传感器校准 |
| 设置页面UI | `src/ui_bridge/settings_ui.{h,cpp}` | 5个分类页面、3按键导航、编辑模式 |
| 需求与设计 | `openspec/changes/*/design.md` | 硬件约束、引脚决策、模块拆分 |
| 分区表 | `partitions/default_8MB.csv` | nvs, otadata, app0/1(OTA), spiffs, coredump |

## CODE MAP

| Symbol | Type | Location | Role |
|--------|------|----------|------|
| `setup()` | function | `src/main.cpp` | 固件启动入口，初始化所有外设 |
| `loop()` | function | `src/main.cpp` | 空转，所有工作由 FreeRTOS 任务执行 |
| `tasks::start_all()` | function | `src/app/tasks.cpp` | 启动 5 个 FreeRTOS 任务（lvgl/sensor/ctrl/input/power） |
| `psu_fsm_transition()` | function | `src/power/psu_fsm.cpp` | 电源状态机核心转换 |
| `fan_temp_to_pwm()` | function | `src/fan/fan_curve.cpp` | 温控曲线：温度→PWM 占空比 |
| `hysteresis_apply()` | function | `src/fan/fan_curve.cpp` | 滞回逻辑，减少频繁抖动 |
| `ntc_adc_to_temp()` | function | `src/sensors/ntc/ntc.cpp` | NTC β 公式温度换算 |
| `ina226_read()` | function | `src/sensors/ina226/ina226.cpp` | INA226 电流/电压读取 |
| `key_debounce_update()` | function | `src/input/keys.cpp` | 按键去抖与事件生成 |
| `tft_driver::init()` | function | `src/display/tft_driver.cpp` | TFT 初始化 + 背光 LEDC |
| `lvgl_port::init()` | function | `src/display/lvgl_port.cpp` | LVGL 移植层：display, flush_cb, tick |
| `config_manager::init()` | function | `src/app/config_manager.cpp` | 初始化配置（从NVS加载或使用默认值） |
| `settings_ui::init()` | function | `src/ui_bridge/settings_ui.cpp` | 初始化设置页面UI |
| `settings_ui::handle_key()` | function | `src/ui_bridge/settings_ui.cpp` | 处理设置页面按键事件 |
| `lof_power_system_init()` | function | `ui/lof_power_system.c` | 手写 UI 入口，调用 `*_init_gen()` |
| `splash_anim_start()` | function | `src/ui_bridge/splash_anim.cpp` | 启动 splash 屏幕动画，n 秒后自动切换到 home |
| `PsuState` | enum | `src/power/psu_fsm.h` | 电源状态（Off/Standby/Starting/On/Stopping/Fault） |
| `app_state` | namespace | `src/app/app_state.cpp` | 全局应用状态（温度/PWM/RPM/PSU），原子变量；含 `fan_rpm`、`fan_duty`、`psu_state_id` |
| `USE_DISPLAY_DEMO` | flag | `include/app_config.h` | true=仅 TFT 测试图案，false=正常 UI |

## CONVENTIONS

- **C++ 标准**：`-std=c++17`
- **静态检查**：cppcheck `--enable=warning,style,performance --inconclusive`（见 `platformio.ini`）
- **编辑器配置**：`.editorconfig` — UTF-8, LF 换行, 缩进 2 空格；C/C++ 文件使用 4 空格
- **生成文件**：`*_gen.c` / `*_gen.h` 由工具生成，**不应手改**；修改在源 XML 或生成流程中进行（包括 `lof_power_system_gen.c/h`）
- **构建产物**：`preview-build/` 与 `preview-bin/` 为生成目录，不应直接编辑其内容
- **编译期配置**：几乎所有配置通过 `app_config.h`（`static constexpr`）和 `platformio.ini`（`-D` flags）控制，无运行时配置文件
- **跨线程数据**：所有多任务共享字段使用 `std::atomic`（见 `src/app/app_state.h`）
- **Main 多入口**：多个 main 由 `platformio.ini` 环境选择（esp32s3→`main.cpp`, native→`native_main_sim.cpp`/`native_main.cpp`）
- **Include 风格**：项目内部头文件使用 `"..."` 和相对路径（如 `"../sensors/ntc/ntc.h"`），系统/库头文件使用 `<>`
- **字体显式指定**：所有 `lv_label` 等文本控件必须通过 `style_text_font` 手动指定字体。可用字体：`hos_14`(14px)、`hos_regular`(16px)、`hos_medium`、`hos_bold`、`hos_bold_big`(44px)、`hos_bold_splash`、`hos_black`、`hos_light`、`hos_thin`、`font_medium`、`font_awesome_14`、`font_awesome_48`。不得依赖 `LV_FONT_DEFAULT`。`lv_conf.h` 中 `LV_FONT_DEFAULT` 保持为 `&lv_font_montserrat_14`（仅作后备，UI 中不使用）。

### 命名约定
- 宏/预处理常量：`UPPER_SNAKE`（`TFT_MOSI`, `INA_CH1_ADDR`, `KEY_K1`）
- 类型（struct/enum/typedef）：`PascalCase`（`Ina226Data`, `PsuState`）
- 函数与变量：`snake_case`（`get_temp_c`, `fan_pwm_set_duty`）
- 枚举值：`UPPER_SNAKE`（`PSU_OFF`, `EVT_BOOT`）
- 生成函数：`*_create()` 用于屏幕创建，`*_init_gen()` 用于初始化

### 测试约定
- 框架：Unity（`<unity.h>`）
- 位置：`test/native/<suite>/test_main.cpp`
- 运行：`pio test -e native`
- 每个测试文件实现 `setUp()`/`tearDown()`（即使为空），用 `RUN_TEST()` 注册用例
- Mock 策略：通过 `platformio.ini` 的 `build_src_filter` 排除硬件实现，替换为 native 替身（`*_native.cpp`）

## ANTI-PATTERNS（本仓库）

- 直接长期手改 `*_gen.c` / `*_gen.h` 或 `lof_power_system_gen.c/h`（生成文件，会被覆盖）
- 手改 `preview-build/**` 中的 `.o/.d/.make/.rsp/link.txt`（构建中间产物）
- 将 `preview-bin/lved-runtime.js` 当作业务源码维护（Emscripten 生成物）
- 在全局宏中定义 `TFT_BL`——会导致 TFT_eSPI 的 `pinMode(-1)` 错误（背光由 `tft_driver.cpp` 自管）
- 绕过 XML 在 `*_gen.c` 中直接大量粘贴修改控件代码
- 假设本目录存在 npm/ts 测试与构建入口（没有）
- 期望支持 WiFi/BLE/OTA（项目不支持，设计层面排除）

## UNIQUE STYLES

- 生成文件采用 `*_gen` 命名，手写入口不带 `_gen`
- 生成屏幕函数以 `*_create()` 暴露，初始化函数以 `*_init_gen()` 命名
- 样式对象常见 `static ... + style_inited` 的一次性初始化模式
- UI 源码不在 `src/` 下——由 `platformio.ini` 的 `build_src_filter` 通过 `+<../ui/...>` 显式引入
- 引脚定义双重维护：`include/pins.h`（C++ 宏）+ `platformio.ini` build_flags（`-D` 宏），两处必须一致
- 文档和 UI 生成产物均遵循电报风格（`TELEGRAPHIC_STYLE_GUIDE`）

## COMMANDS

```bash
# 固件构建（默认目标 esp32s3）
pio run -e esp32s3

# 本地单元测试（native 平台）
pio test -e native

# 本地 LVGL 模拟器（native 构建并运行）
pio run -e native

# 编译验证（smoke 测试）
pio run -e native-smoke

# 静态分析
pio check -e esp32s3 --skip-packages

# 查看固件大小
pio run -e esp32s3 -t size

# 烧录
pio run -e esp32s3 -t upload

# 串口监视
pio device monitor -e esp32s3

# UI 预览构建（需 emsdk/cmake 环境）
cmake --build ui/preview-build
```

## NOTES

- 本固件仅经编译验证，**未经实际硬件测试**；烧录前确认引脚连接
- 无 CI/CD 配置——建议添加 GitHub Actions 工作流（`pio run` + `pio test` + `pio check`）
- 无 Docker 配置——可通过 PlatformIO 官方镜像容器化构建
- Windows native 构建需要 MSYS2/MinGW（`C:\msys64\mingw64\bin`），由 `scripts/use_msys2_mingw.py` 自动注入
- `.gitignore` 仅忽略 `preview-build`；`preview-bin` 是否跟踪由仓库策略决定
- 无代码覆盖率配置——可在 native env 添加 `-fprofile-arcs -ftest-coverage`
- LSP 支持：clangd（需安装），支持 `.c/.cpp/.h/.hpp`
- 分区表：8MB Flash，双 OTA 分区（app0/app1）+ spiffs + coredump
