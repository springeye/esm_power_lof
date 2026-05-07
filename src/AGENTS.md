# FIRMWARE SOURCE KNOWLEDGE BASE

## OVERVIEW
`src/` 包含 ESP32 智能风扇控制器全部固件源码（C++17，Arduino 框架）。模块化分层：硬件抽象 → 外设驱动 → 算法逻辑 → 应用协调。由 `platformio.ini` 按环境选择编译哪些文件（`build_src_filter`）。

## STRUCTURE
```
src/
├── main.cpp                # 固件入口（Arduino setup/loop）
├── app/                    # 应用协调层
│   ├── tasks.{h,cpp}       # FreeRTOS 任务定义与启动
│   ├── app_state.{h,cpp}   # 全局应用状态（std::atomic）
│   ├── watchdog.{h,cpp}    # 任务看门狗
│   └── fault_guard.{h,cpp} # 故障检测与保护
├── hal/                    # 硬件抽象层
│   ├── i2c_bus.{h,cpp}     # I2C 总线（互斥 + 扫描）
│   └── spi_bus.{h,cpp}     # SPI 总线（互斥）
├── display/                # 显示驱动
│   ├── tft_driver.{h,cpp}  # TFT_eSPI 包装（背光 LEDC）
│   └── lvgl_port.{h,cpp}   # LVGL 移植层（display, flush_cb, tick）
├── sensors/                # 传感器驱动
│   ├── ntc/ntc.{h,cpp}     # NTC 温度（β 公式，中值滤波）
│   └── ina226/ina226.{h,cpp} # INA226 电流/电压（3 路）
├── fan/                    # 风扇控制
│   ├── fan_curve.{h,cpp}   # 温控曲线 + 滞回
│   ├── fan_pwm.{h,cpp}     # LEDC PWM 输出
│   └── fan_tach.{h,cpp}    # PCNT 转速测量
├── power/                  # 电源管理
│   ├── psu_fsm.{h,cpp}     # ATX 电源状态机
│   └── ps_on.{h,cpp}       # PS_ON/PW_OK 引脚控制
├── input/                  # 按键输入
│   └── keys.{h,cpp}        # 去抖与事件生成
├── ui_bridge/              # UI 胶水层
│   ├── screen_manager.{h,cpp} # 屏幕生命周期（splash→home）
│   ├── data_bridge.{h,cpp}    # app_state → LVGL 数据刷新
│   └── input_bridge.{h,cpp}   # 按键 → LVGL 事件分发
└── compat/                 # 兼容层
    └── lvgl_v8_shim.cpp    # LVGL v8 生成代码适配
```

## WHERE TO LOOK

| Task | Location | Notes |
|------|----------|-------|
| 启动流程 | `main.cpp::setup()` | 初始化外设 → LVGL → UI → 传感器 → 风扇 → 电源 → 启动任务 |
| 添加 FreeRTOS 任务 | `app/tasks.cpp` | `tasks::start_all()` 创建 5 个任务，`task_*()` 为任务主体 |
| 添加全局状态变量 | `app/app_state.h` | `std::atomic` 字段 + `get_*/set_*` 内联函数 |
| 故障检测逻辑 | `app/fault_guard.cpp` | 过温/堵转/过流/PWOK 失稳检查 |
| I2C 通信 | `hal/i2c_bus.cpp` | `i2c_scan()`, `i2c_bus_take/give()` 互斥，所有 I2C 读写须获取总线锁 |
| SPI 通信 | `hal/spi_bus.cpp` | TFT_eSPI 自管 SPI，本模块仅提供互斥 |
| TFT 显示 | `display/tft_driver.cpp` | 初始化 + 背光 PWM（LEDC ch1，GPIO 定义在 `include/pins.h`） |
| LVGL 移植 | `display/lvgl_port.cpp` | 双缓冲、`flush_cb`、`tick_increment`、`task_handler` |
| NTC 温度计算 | `sensors/ntc/ntc.cpp` | `ntc_adc_to_temp()`，参数从 `app_config.h` 读取 |
| INA226 读取 | `sensors/ina226/ina226.cpp` | `ina226_read()` 返回电压/电流，3 路轮询 |
| 温控曲线 | `fan/fan_curve.cpp` | `fan_temp_to_pwm()` 三拐点分段线性，`hysteresis_apply()` 防抖 |
| 风扇 PWM 输出 | `fan/fan_pwm.cpp` | 25kHz LEDC PWM，10-bit，占空比 0-1023 |
| 风扇转速读取 | `fan/fan_tach.cpp` | PCNT 硬件计数，4 线风扇每转 2 脉冲 |
| 电源状态机 | `power/psu_fsm.cpp` | `psu_fsm_transition()`，6 状态枚举 |
| 按键处理 | `input/keys.cpp` | `key_debounce_update()` 5ms 采样，支持短按/长按 |
| UI 数据刷新 | `ui_bridge/data_bridge.cpp` | 200ms 定时器读取 `app_state` 更新 LVGL 标签 |
| UI 输入分发 | `ui_bridge/input_bridge.cpp` | `lv_async_call` 安全跨任务分发按键事件 |
| 屏幕切换 | `ui_bridge/screen_manager.cpp` | splash 显示 → 定时 → 切换到 home |
| LVGL v8 兼容 | `compat/lvgl_v8_shim.cpp` | 类型别名与宏桥接 v8 生成代码到 v9 API |

## CONVENTIONS

- **语言标准**：C++17（`platformio.ini` 中 `-std=c++17`）
- **命名约定**（与项目全局一致）：
  - 宏/枚举值：`UPPER_SNAKE`（`PSU_OFF`, `EVT_BOOT`）
  - 类型（struct/enum/typedef）：`PascalCase`（`Ina226Data`, `KeyState`）
  - 函数/变量：`snake_case`（`get_temp_c`, `fan_pwm_set_duty`）
- **Include 风格**：项目内部用 `"../module/header.h"` 相对路径，系统/库用 `<>`
- **多任务安全**：所有跨任务共享数据在 `app_state` 中用 `std::atomic`
- **模块边界**：每个 `.h` 只暴露模块公共 API，`.cpp` 隐藏实现细节
- **入口分离**：`main.cpp`（ESP32 固件）——由 `platformio.ini` env 选择
- **编译期配置**：`include/app_config.h` 提供 `static constexpr` 常量，`platformio.ini` 提供 `-D` 宏
- **HAL 层**：所有硬件访问经 `hal/` 抽象，上层不直接操作寄存器

### 任务设计
- 5 个 FreeRTOS 任务（lvgl/sensor/ctrl/input/power），每个独立 `vTaskDelay` 控制频率
- 每个任务在循环开始调用 `watchdog::feed()`（防止 WDT 超时）
- 任务栈大小在 `app_config.h` 中配置（TASK_STACK_* 系列）

### 传感器约定
- NTC：16 次采样取中值（`NTC_SAMPLES=16`）
- INA226：3 路轮询，总周期 750ms（每路 250ms）
- 所有传感器数据写入 `app_state`，不在传感器模块内缓存

## ANTI-PATTERNS（本模块）

- 绕过 `hal/` 层直接操作 I2C/SPI 寄存器
- 在任务函数外访问 `app_state` 不加锁（内部已用 `std::atomic`）
- 在 `*_gen.c/h` 所在模块添加编译依赖（生成文件路径由 `platformio.ini` 显式列出）
- 修改 `build_src_filter` 时忘记更新 UI 文件列表

## NOTES

- 本目录是固件业务逻辑唯一位置；UI 源码在 `../ui/`，由 `platformio.ini::build_src_filter` 通过 `+<../ui/...>` 引入编译
- 各模块 `.h` 暴露的公共函数即该模块的稳定接口；新增模块时在 `app/tasks.cpp` 接入任务调度
- `tft_driver.cpp` 的背光控制使用 LEDC ch1 并避免在全局定义 `TFT_BL`（防止 TFT_eSPI 误判）
