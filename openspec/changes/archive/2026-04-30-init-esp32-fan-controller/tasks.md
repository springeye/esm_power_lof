# 实施任务清单

> 任务按拓扑顺序排列，先骨架后业务，先驱动后应用，最后联调。每完成一项请打勾。

## 1. 工程骨架（capability: project-scaffold）

- [ ] 1.1 在仓库根创建 `platformio.ini`，按 design.md D1 配置 platform/board/framework/partitions/monitor_speed/upload_speed
- [ ] 1.2 创建 `partitions/default_8MB.csv`（含 nvs/otadata/2×app/spiffs，匹配 8MB Flash）
- [ ] 1.3 创建目录骨架：`include/`、`src/{hal,display,ui,sensors/ntc,sensors/ina226,fan,power,input,app}/`、`lib/`
- [ ] 1.4 创建 `include/pins.h`，按 design.md D2 集中定义所有 GPIO 宏
- [ ] 1.5 创建 `include/app_config.h`，定义 NTC 参数、温控阈值、INA226 地址、按键时间常量等
- [ ] 1.6 创建占位 `src/main.cpp`（仅 `setup()`/`loop()`，串口打印 "Boot OK"），编译通过
- [ ] 1.7 在 `platformio.ini` 的 `lib_deps` 添加：`bodmer/TFT_eSPI@^2.5.43`、`lvgl/lvgl@^9.2.2`、`robtillaart/INA226@^0.6.0`
- [ ] 1.8 创建 `README.md`，包含硬件清单、引脚表、构建命令

## 2. 显示与 LVGL 移植（capability: display-lvgl）

- [ ] 2.1 在 `platformio.ini` 的 `build_flags` 注入 TFT_eSPI 用户配置（USER_SETUP_LOADED、ST7789_DRIVER、TFT_WIDTH=240、TFT_HEIGHT=280、CGRAM_OFFSET=1、SPI_FREQUENCY=40000000、SMOOTH_FONT=1，及 SCK/MOSI/CS/DC/RST/BL 引脚）
- [ ] 2.2 创建 `include/lv_conf.h`，按 design.md D3 配置颜色深度/字体/双缓冲/PARTIAL 渲染
- [ ] 2.3 在 `src/display/display.{h,cpp}` 实现 `display::init()`：初始化 TFT_eSPI、注册 LVGL 显示驱动（双缓冲 240×28）、esp_timer 1ms 推 lv_tick
- [ ] 2.4 实现 `display::tick()`：周期 5ms 调用 `lv_timer_handler()`，挂入 lvglTask
- [ ] 2.5 烧录验证：屏幕背光点亮，LVGL 显示一个 Hello 标签且无撕裂

## 3. 主仪表盘 UI（capability: ui-dashboard）

- [ ] 3.1 在 `src/ui/ui_dashboard.{h,cpp}` 创建主屏幕容器与 4 个区域（温度/风扇/电源 ×3/电源状态条）
- [ ] 3.2 使用 `LV_FONT_MONTSERRAT_16` 与 `LV_FONT_MONTSERRAT_24` 渲染数值
- [ ] 3.3 实现 `ui_dashboard::updateTemp/updateFan/updateRail/updatePowerState` API
- [ ] 3.4 实现告警颜色规则（正常绿、警告黄、错误红）与 "--" 占位
- [ ] 3.5 在屏幕底部显示 K1/K2/K3 当前功能提示

## 4. NTC 温度采样（capability: temperature-sensing）

- [ ] 4.1 在 `src/sensors/ntc/ntc.{h,cpp}` 实现 `ntc::init()`：`analogReadResolution(12)`、`analogSetPinAttenuation(NTC_PIN, ADC_11db)`
- [ ] 4.2 实现 `ntc::sampleOnce()`：16 次采样取中值
- [ ] 4.3 实现 `ntc::computeTempC()`：β 公式换算
- [ ] 4.4 添加短路/开路保护与 Fault 状态输出
- [ ] 4.5 挂入 sensorTask（200ms 周期），并将结果通过共享结构发布

## 5. 风扇控制（capability: fan-control）

- [ ] 5.1 在 `src/fan/fan.{h,cpp}` 用 `ledcSetup`/`ledcAttachPin` 配置 ch0、25kHz、10-bit、GPIO25
- [ ] 5.2 配置 PCNT 单元在 GPIO35 计数 SPEED 上升沿
- [ ] 5.3 实现 `fan::setDuty(percent)`、`fan::getRpm()`
- [ ] 5.4 实现温控曲线（含 2°C 回滞）：`fan::applyAutoCurve(tempC)`
- [ ] 5.5 实现堵转检测：duty>30% 持续 3s 且 RPM=0 → Fault
- [ ] 5.6 挂入 controlTask（500ms 周期）

## 6. INA226 电源监测（capability: power-rail-monitoring）

- [ ] 6.1 `Wire.begin(SDA, SCL); Wire.setClock(400000);`
- [ ] 6.2 在 `src/sensors/ina226/ina_rail.{h,cpp}` 实例化 3 个 INA226（0x40/0x41/0x44），调用 `setMaxCurrentShunt(40, 0.002)`
- [ ] 6.3 实现 250ms 错峰轮询读 V/I/P
- [ ] 6.4 实现 I2C 错误计数与总线恢复
- [ ] 6.5 通过共享结构发布数据并推送到 UI

## 7. 电源开关控制（capability: power-switch-control）

- [ ] 7.1 在 `src/power/power.{h,cpp}` 配置 GPIO27 为开漏输出（默认高），GPIO34 为输入
- [ ] 7.2 实现状态机 `Off/Standby/Starting/On/Stopping/Fault`
- [ ] 7.3 实现 `power::turnOn()/turnOff()` 与 1s PWOK 等待超时
- [ ] 7.4 实现运行中 PWOK 失稳 100ms 检测 → Fault
- [ ] 7.5 接入故障保护规则（温度 ≥80°C / 风扇 Fault 5s / INA226 过流）
- [ ] 7.6 powerTask 事件驱动，与 inputTask 通过 queue 通信

## 8. 按键输入（capability: user-input）

- [ ] 8.1 在 `src/input/keys.{h,cpp}` 配置 K1/K2/K3 为 `INPUT_PULLUP`
- [ ] 8.2 实现 5ms 周期采样 + 3 次连续判定的去抖逻辑
- [ ] 8.3 实现短按（<800ms）/长按（≥800ms）事件分发
- [ ] 8.4 通过 FreeRTOS Queue 发布 `KeyEvent` 给 powerTask 与 UI

## 9. 系统集成（capability: project-scaffold + 全部）

- [ ] 9.1 在 `src/app/app.{h,cpp}` 创建 5 个 FreeRTOS 任务（lvgl/sensor/control/input/power），按 design.md D9 绑核与优先级
- [ ] 9.2 在 `main.cpp` 调用 `display::init()` → `ntc::init()` → `fan::init()` → `ina_rail::init()` → `power::init()` → `keys::init()` → `app::start()`
- [ ] 9.3 编译并烧录到目标板
- [ ] 9.4 完整功能联调：温度/风扇/3 路电源/开机关机/按键全部正常
- [ ] 9.5 长跑测试：连续运行 ≥ 30 分钟，无 watchdog 复位、UI 不卡顿

## 10. 收尾

- [ ] 10.1 运行 `openspec validate init-esp32-fan-controller --strict` 确保零错误
- [ ] 10.2 更新 `README.md` 写入实测引脚连线、烧录命令、注意事项
- [ ] 10.3 通知用户审阅，决定是否归档变更
