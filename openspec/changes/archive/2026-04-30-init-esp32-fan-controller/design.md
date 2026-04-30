## Context

目标硬件为 ESP32-WROOM-32E-N8（双核 Xtensa LX6，520KB SRAM，8MB flash，无 PSRAM），固件框架 Arduino on PlatformIO。外设清单：

- 显示：ST7789P3 240×280，4-SPI（SDA/SCL/CS/DC/RES）+ 背光 BLK
- 监测：3× INA226（共享 I2C，地址 0x40/0x41/0x44，shunt 2mΩ）
- 温度：1× NTC 接 ADC
- 风扇：4 线 PWM 风扇（PWM 输出 25kHz + SPEED 测速输入）
- 电源：PSON（输出，控开关电源）+ PWOK（输入，电源就绪反馈）
- 输入：3 个按键 K1/K2/K3
- UI：LVGL 9.x，抗锯齿英文字体，无中文

ESP32-WROOM-32E 引脚约束：
- GPIO6–11 占用内部 SPI flash，禁止使用
- GPIO34–39 仅输入、无内部上拉
- ADC2 在 WiFi 启用时不可读（NTC 必须用 ADC1：GPIO32–39）
- Strapping：GPIO0/2/5/12/15 上电状态敏感，避免外部强驱动
- LEDC 高速通道支持 25kHz 8/10-bit PWM

当前仓库为空，需建立完整 PlatformIO 工程。OpenSpec 提案要求把全部 8 个能力一次性立起来。

## Goals / Non-Goals

**Goals:**
- 提供可一次 `pio run` 编译通过、`pio run -t upload` 烧录运行的最小可用固件骨架。
- 引脚分配遵守 ESP32-WROOM-32E 限制，并文档化为常量头文件 `include/pins.h`，单点修改。
- 模块化分层：HAL（引脚/总线）↔ Driver（TFT/INA226/NTC/Fan/PSU/Keys）↔ Service（温控策略/UI 数据模型）↔ App（任务调度），便于后续单元替换。
- LVGL 9.x 双缓冲（每缓冲 240×28×2B = 13.44KB，共 26.88KB）+ TFT_eSPI DMA 刷新。
- 25kHz 4-pin 风扇 PWM（10-bit 分辨率 = 0–1023）+ 转速脉冲计数（每转 2 脉冲）→ RPM。
- 三路 INA226 共享 I2C@400kHz，独立采样并发布到 UI 数据总线。
- PSON 上电/掉电时序与 PWOK 监测，PWOK 异常 → 立即拉低 PSON 进入安全态。
- 字体高清抗锯齿：使用 LVGL 自带 Montserrat 16/24（4bpp），不引入中文字模以节省 flash。

**Non-Goals:**
- 不实现 Wi-Fi/BLE/OTA/MQTT 等联网能力（保留扩展空间但本次不接）。
- 不实现 PID 高级温控（先用分段线性曲线，后续可替换）。
- 不实现按键组合/旋转编码器/触摸屏。
- 不实现配置持久化（NVS）—— 保留 hook，本次默认硬编码参数。
- 不做中文字体 / 复杂图形主题。

## Decisions

### D1：构建系统与目标板

- 选择 PlatformIO，`platform = espressif32@^6.7.0`（Arduino-ESP32 v2.0.x，稳定且 LVGL 兼容性好）。
- `board = esp32dev`，`framework = arduino`。
- `board_build.f_flash = 80000000L`，`board_build.flash_mode = qio`，`board_upload.flash_size = 8MB`，分区 `default_8MB.csv`。
- `monitor_speed = 115200`。
- `build_flags` 内联 LVGL 与 TFT_eSPI 的编译宏（避免维护两份 User_Setup）。

**备选**：ESP-IDF 直接接入 → 放弃，业务以 Arduino 生态库为主，迁移成本高。

### D2：引脚分配（写入 `include/pins.h`）

遵守 ESP32-WROOM-32E 限制下，按"高速 SPI 用原生 IOMUX、ADC 用 ADC1、避开 strapping/flash"原则分配：

| 信号 | GPIO | 说明 |
|---|---|---|
| TFT SCK (SPI_SCL) | 18 | VSPI CLK 原生引脚 |
| TFT MOSI (SPI_SDA) | 23 | VSPI MOSI 原生引脚 |
| TFT CS | 5 | VSPI CS（strapping，上电默认上拉，安全） |
| TFT DC | 2 | strapping，上电低/高均可，注意烧录时不要外部强驱动 |
| TFT RES | 4 | 通用 IO |
| TFT BLK | 16 | 通用 IO，PWM 调光（LEDC ch1） |
| INA226 I2C SDA | 21 | 默认 Wire SDA |
| INA226 I2C SCL | 22 | 默认 Wire SCL |
| FAN PWM | 25 | LEDC ch0，25kHz |
| FAN SPEED | 35 | 仅输入，外部上拉到 +5V→ 分压到 3.3V，PCNT 计数 |
| NTC ADC | 36 (SVP) | ADC1_CH0，仅输入 |
| PSON | 27 | 输出，开漏到电源（外部上拉到 +5VSB），低有效 |
| PWOK | 34 | 仅输入，外部上拉/分压 |
| K1 | 32 | 输入 + 内部上拉 |
| K2 | 33 | 输入 + 内部上拉 |
| K3 | 26 | 输入 + 内部上拉 |

**说明**：
- GPIO0/12/15 全部空闲，避免 strapping 误触发。
- TFT DC 选 GPIO2 是常见做法；若烧录出现问题，可在硬件加 1kΩ 串阻或改到 GPIO17。
- FAN SPEED 选 GPIO35（输入专用，无内部上拉），需要外部上拉 + 5V→3.3V 电平转换。
- PSON 用开漏输出避免在 ESP32 复位时误触发电源。

### D3：显示驱动 = TFT_eSPI + LVGL 9.x

- 选 `bodmer/TFT_eSPI@^2.5.43`：对 ST7789（含 240×240/240×280 偏移）支持完善，DMA 性能最佳。
- 选 `lvgl/lvgl@^9.2.2`：API 已稳定，性能与字体子系统比 8.x 更好。
- 通过 PlatformIO `build_flags` 注入 TFT_eSPI 的 `User_Setup`：
  - `-DUSER_SETUP_LOADED=1`
  - `-DST7789_DRIVER=1`
  - `-DTFT_WIDTH=240 -DTFT_HEIGHT=280`
  - `-DCGRAM_OFFSET=1`（240×280 需要 X+0/Y+20 偏移，由 TFT_eSPI 自动处理）
  - `-DTFT_MOSI=23 -DTFT_SCLK=18 -DTFT_CS=5 -DTFT_DC=2 -DTFT_RST=4 -DTFT_BL=16`
  - `-DSPI_FREQUENCY=40000000 -DSPI_READ_FREQUENCY=20000000`
  - `-DLOAD_GLCD=0 -DLOAD_FONT2=0 -DLOAD_FONT4=0 -DLOAD_GFXFF=0 -DSMOOTH_FONT=1`
- LVGL 通过工程根 `include/lv_conf.h` 配置（设置 `LV_CONF_INCLUDE_SIMPLE`、`LV_FONT_MONTSERRAT_16/24=1`、关闭中文字体、抗锯齿 `LV_USE_FONT_SUBPX=0`，开启 `LV_FONT_SUBPX_BGR=0`，`LV_COLOR_DEPTH=16`、`LV_COLOR_16_SWAP=1`）。
- `disp_buf` 双缓冲，每缓冲 `240×28` × `lv_color_t(2B)` ≈ 13.44KB，使用 `LV_DISPLAY_RENDER_MODE_PARTIAL`。
- `lv_tick` 由 `esp_timer` 周期回调 1ms 推进。
- `lv_timer_handler` 在专用任务（核 1，优先级 2，5ms 周期）调用。

**备选**：LovyanGFX → 放弃，因 LVGL 集成样例较少；Arduino_GFX → 放弃，性能略低于 TFT_eSPI DMA。

### D4：温度采样与温控策略

- NTC 分压：上拉 10kΩ → NTC（B 值假设 3950，R25 = 10kΩ）→ GND，ADC 读取分压点。
- ADC：`analogReadResolution(12)`，多次采样（16 次中值平均）。
- 温度换算：β 公式 `1/T = 1/T0 + 1/B * ln(R/R0)`。
- 温控曲线（写在 `app/fan_policy.cpp`，可调）：
  - <30°C → 占空比 20%（最小转速保活）
  - 30–50°C → 线性 20%→60%
  - 50–70°C → 线性 60%→100%
  - ≥75°C → 100% + 触发告警 UI
  - 低于回滞 2°C 才降速，避免震荡
- 采样周期 200ms，PWM 更新周期 500ms。

### D5：风扇 PWM 与测速

- LEDC channel 0，频率 25000Hz，分辨率 10-bit（1024 级），符合 Intel 4-pin 风扇标准。
- 测速：使用 ESP32 PCNT 单元绑定 GPIO35，每 1s 读取计数→ `RPM = pulses / 2 * 60`，复位计数。
- 故障判定：占空比 >30% 持续 3s 仍 RPM=0 → 报警。

### D6：INA226 总线

- I2C @ 400kHz，三路实例 `INA226 ina[3]`，地址 0x40/0x41/0x44，分流电阻 2mΩ → 校准 `ina.setMaxCurrentShunt(40, 0.002)`。
- 选 `robtillaart/INA226@^0.6.0`（轻量、API 稳定）。
- 采样：每 250ms 轮询一路（错峰），3 路完整周期 750ms。
- 数据通过单生产者单消费者 `volatile struct PowerSample[3]` + spinlock-free `std::atomic` 暴露给 UI 任务。

### D7：电源时序

- 上电流程：
  1. boot → MCU 自检 → UI 显示 "Standby"
  2. 用户短按 K1 或满足自动条件 → PSON 拉低 → 等待 PWOK 高电平（超时 1s 失败）
  3. PWOK 持续高 → 进入 "On" 状态
- 掉电流程：长按 K1 2s 或 PWOK 失稳 100ms → PSON 拉高 → "Off"
- 所有电源状态通过事件队列分发到 UI 与日志。

### D8：按键

- 简易扫描：GPIO 输入 + 内部上拉，5ms 软件去抖，识别短按（<800ms）/长按（≥800ms）。
- 默认绑定：K1 = 电源开关，K2 = UI 切页/确认，K3 = 风扇手/自动模式切换（占位）。

### D9：任务划分（FreeRTOS）

| 任务 | 核 | 优先级 | 周期 | 职责 |
|---|---|---|---|---|
| `lvglTask` | 1 | 2 | 5ms | `lv_timer_handler` |
| `sensorTask` | 0 | 3 | 200ms | NTC 采样 + INA226 轮询 |
| `controlTask` | 0 | 4 | 500ms | 温控策略 → 更新 PWM |
| `inputTask` | 0 | 2 | 5ms | 按键扫描 + 事件分发 |
| `powerTask` | 0 | 5 | 事件驱动 | PSON/PWOK 状态机 |

跨任务通信：`QueueHandle_t` 事件队列 + 共享 `std::atomic` 数据。

### D10：目录结构

```
.
├─ platformio.ini
├─ partitions/default_8MB.csv
├─ include/
│  ├─ pins.h
│  ├─ lv_conf.h
│  └─ app_config.h
├─ src/
│  ├─ main.cpp
│  ├─ hal/{i2c_bus,spi_bus,ledc}.{h,cpp}
│  ├─ display/{tft_driver,lvgl_port}.{h,cpp}
│  ├─ ui/{ui_main,ui_styles,ui_events}.{h,cpp}
│  ├─ sensors/ntc/{ntc.h,ntc.cpp}
│  ├─ sensors/ina226/{power_monitor.h,power_monitor.cpp}
│  ├─ fan/{fan_pwm.h,fan_pwm.cpp,tach.h,tach.cpp}
│  ├─ power/{psu.h,psu.cpp}
│  ├─ input/{keys.h,keys.cpp}
│  └─ app/{fan_policy.h,fan_policy.cpp,events.h,tasks.h,tasks.cpp}
└─ lib/  (留空，或放本地化的 TFT_eSPI User_Setup 包装)
```

## Risks / Trade-offs

- [TFT_eSPI 与 LVGL 9.x 集成示例较少] → 在 `display/lvgl_port.cpp` 内自实现 `flush_cb`（`tft.startWrite() / pushImageDMA / endWrite()`），并在 README 标注版本组合。
- [GPIO2/5 是 strapping 引脚] → 上电瞬间 TFT 不应被驱动；硬件层在 BLK 上加 RC 延时或软件先拉 RES 低 50ms 再启动 SPI。
- [无 PSRAM，LVGL 双缓冲 + TFT_eSPI DMA buffer 占用 ~30KB SRAM] → 避免在堆栈上分配大缓冲；任务栈预算：lvglTask 8KB，其它 4KB。
- [ADC1 非线性] → 使用 `esp_adc_cal` 或多点校准，温度精度±2°C 足够。
- [INA226 共享 I2C 与 TFT 不同总线，但若总线挂死会阻塞 sensorTask] → I2C 操作设置 50ms 超时，失败计数 ≥5 触发软复位 I2C。
- [PWOK 未上拉到 ESP32 电平] → 硬件需电平转换，固件假设已转换为 0/3.3V。
- [PSON 在复位瞬间状态未定] → 用开漏输出 + 外部上拉至 +5VSB，复位时默认电源 OFF。
- [LVGL 9.x 仍在演进] → 锁定 `^9.2.2`，在 `platformio.ini` 注释版本约束理由。

## Migration Plan

新仓库无迁移负担。部署流程：
1. `pio run` → 验证依赖能拉取并编译。
2. `pio run -t upload` → 烧录到目标板。
3. `pio device monitor` → 观察启动日志（应有 TFT init OK / LVGL ready / I2C scan / fan tach 0rpm 等）。
4. UI 出现 Standby 仪表盘 → 短按 K1 → 听到风扇启动 + PWOK 上升 → "On" 状态。
5. 回滚：`git revert` 或重新烧录上一固件，无持久化数据需要清理。

## Open Questions

1. NTC 实际 B 值与 R25 是否就是 3950/10k？若不同需要在 `app_config.h` 修改常量。
2. 风扇 SPEED 引脚硬件是否已做 5V→3.3V 电平转换？（设计假设已转换）
3. PWOK 上电后多久应稳定（不同电源 100ms~1s 不等），决定超时阈值。
4. INA226 是否需要平均/转换时间高级配置（默认 1.1ms × 4 平均 = 4.4ms 已足够）。
5. 是否需要在 `lib/` 提供 TFT_eSPI 的 `User_Setup_Select.h` 封装以避免依赖 build_flags？（当前选 build_flags 路线，简单）。
