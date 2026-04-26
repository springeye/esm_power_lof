# ST7789 屏幕接线指南（ESP32-S3）

适用屏幕：ST7789 240×280，8 脚 SPI 接口
适用主控：ESP32-S3-DevKitC-1（WROOM-1 模组，非 Octal Flash）

> ⚠️ 若使用 **WROOM-2（Octal PSRAM/Flash）** 模组，GPIO34/35/36 不可用，需改用 GPIO4/5/6/7/8。详见文末「模组兼容性」。

---

## 接线对照表

| 屏幕脚号 | 屏幕信号 | ESP32-S3 GPIO | 说明 |
|:---:|:---|:---:|:---|
| 1 | **VCC** | **3.3V** | ⚠️ 严禁接 5V，ST7789 是 3.3V 逻辑 |
| 2 | **GND** | **GND** | 共地 |
| 3 | **BLK** 背光 | **GPIO21** | `TFT_BL`，由 LEDC 通道 1 输出 PWM 调光 |
| 4 | **RES** 复位 | **GPIO14** | `TFT_RST`，低电平复位 |
| 5 | **CS** 片选 | **GPIO10** | `TFT_CS`，普通 GPIO（本开发板未引出 GPIO34，已改用 GPIO10）|
| 6 | **DC** 数据/命令 | **GPIO13** | `TFT_DC`，高=数据，低=命令 |
| 7 | **WR**（= MOSI/SDA）| **GPIO35** | `TFT_MOSI`，FSPI 数据 |
| 8 | **TE**（= SCLK）| **GPIO36** | `TFT_SCLK`，FSPI 时钟 |

> 注：8 脚 ST7789 模组实际是单向 SPI，不引出 MISO；TFT_eSPI 配置中 `TFT_MISO=-1`。

---

## 引脚名称说明

很多廉价 ST7789 模组把 SPI 信号印成了非标准名字，实际对应关系如下：

| 屏幕标注 | 实际含义 | 对应 SPI 信号 |
|:---:|:---|:---:|
| WR | Write（写数据线）| MOSI / SDA |
| TE | Tearing Effect（被错印）| SCLK / SCL |
| SDA | Serial Data | MOSI |
| SCL | Serial Clock | SCLK |

如果按上表接线后**屏幕没反应**，最可能的原因是厂商把 7 / 8 脚的丝印反了：
**第一步排查：把 GPIO35 和 GPIO36 对调**（即 WR↔TE 互换）。

---

## 故障排查

| 现象 | 可能原因 | 解决方法 |
|:---|:---|:---|
| 黑屏，背光也不亮 | VCC/GND 没接 或 BLK 没接 | 检查 1/2/3 脚 |
| 背光亮但全白屏 | RES 一直低电平 / DC 接错 | 检查 4、6 脚 |
| 背光亮但全黑屏 | MOSI / SCLK 接反 | 对调 7、8 脚 |
| 显示花屏/雪花 | SPI 频率太高 或 接线过长 | 把 `SPI_FREQUENCY` 降到 20MHz |
| 显示偏移几像素 | CGRAM_OFFSET 配置不对 | 检查 `-DCGRAM_OFFSET=1` |
| 颜色反相 | 控制器 INVERSION 配置 | 加 `-DTFT_INVERSION_ON` 或 `-DTFT_INVERSION_OFF` |

---

## 代码配置同步

引脚号在以下两处同时定义，必须保持一致：

1. **`include/pins.h`** — C++ 业务代码使用
2. **`platformio.ini` 的 `build_flags`** — TFT_eSPI 库使用

当前 `platformio.ini` 已加入：

```ini
-DTFT_MOSI=35
-DTFT_SCLK=36
-DTFT_CS=10
-DTFT_DC=13
-DTFT_RST=14
-DTFT_MISO=-1
-DTFT_BL=-1            ; 背光由 src/display/tft_driver.cpp 自行用 LEDC 管理
-DTFT_BACKLIGHT_ON=HIGH
```

> `TFT_BL=-1` 是有意为之：背光由 `tft_driver.cpp` 通过 LEDC 通道 1 输出 PWM，
> 不让 TFT_eSPI 库接管 GPIO21，避免冲突。

---

## 模组兼容性

| ESP32-S3 模组 | GPIO34/35/36 是否可用 | TFT 引脚建议 |
|:---|:---:|:---|
| WROOM-1（Quad Flash，无 PSRAM）| ✅ 可用 | 沿用当前 GPIO34/35/36 |
| WROOM-1U（同上）| ✅ 可用 | 同上 |
| WROOM-2（Octal Flash + Octal PSRAM）| ❌ 被 Flash 占用 | 改用 GPIO4/5/6/7/8 |

如需切换到 WROOM-2，请同步修改 `include/pins.h` 与 `platformio.ini`。

---

## 物理连接示意

```
ESP32-S3 DevKitC-1                ST7789 240×280
┌──────────────────┐              ┌──────────────┐
│              3V3 ├──────────────┤ 1  VCC       │
│              GND ├──────────────┤ 2  GND       │
│           GPIO21 ├──────────────┤ 3  BLK       │
│           GPIO14 ├──────────────┤ 4  RES       │
│           GPIO10 ├──────────────┤ 5  CS        │
│           GPIO13 ├──────────────┤ 6  DC        │
│           GPIO35 ├──────────────┤ 7  WR (MOSI) │
│           GPIO36 ├──────────────┤ 8  TE (SCLK) │
└──────────────────┘              └──────────────┘
```

---

## 上电检查清单

- [ ] VCC 接 3.3V（不是 5V！）
- [ ] GND 共地
- [ ] 7 根信号线全部连接
- [ ] `platformio.ini` 中 TFT_* 宏与本表一致
- [ ] `include/pins.h` 中 TFT_* 宏与本表一致
- [ ] 烧录后串口日志无 `tft_driver init failed`
- [ ] 背光点亮（亮度由 `set_backlight()` 控制，默认 200/255）
