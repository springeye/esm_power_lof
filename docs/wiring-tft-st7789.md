# ST7789 屏幕接线指南（ESP32-S3）

适用屏幕：ST7789 240×280，8 脚 SPI 接口
适用主控：**ESP32-S3-DevKitC-1（WROOM-1-N8R8 模组）**

> ⚠️ **N8R8 重要说明**
> N8R8 = 8MB Quad Flash + **8MB Octal PSRAM**。Octal PSRAM 强制占用 **GPIO 33/34/35/36/37**（厂商在丝印上根本不引出，所以"开发板最低 IO 是 38"或部分板"最低 35 但不能用"的现象由此而来）。
> **本指南已切换到 FSPI IO_MUX 默认低位引脚 11/12/9，绕开 PSRAM 冲突。**

---

## 接线对照表

| 屏幕脚号 | 屏幕信号 | ESP32-S3 GPIO | 说明 |
|:---:|:---|:---:|:---|
| 1 | **VCC** | **3.3V** | ⚠️ 严禁接 5V，ST7789 是 3.3V 逻辑 |
| 2 | **GND** | **GND** | 共地 |
| 3 | **BLK** 背光 | **GPIO21** | `TFT_BL`，由 LEDC 通道 1 输出 PWM 调光 |
| 4 | **RES** 复位 | **GPIO14** | `TFT_RST`，低电平复位 |
| 5 | **CS** 片选 | **GPIO10** | `TFT_CS`，普通 GPIO |
| 6 | **DC** 数据/命令 | **GPIO13** | `TFT_DC`，高=数据，低=命令 |
| 7 | **WR**（= MOSI/SDA）| **GPIO11** | `TFT_MOSI`，FSPI IO_MUX 默认 D 脚 |
| 8 | **TE**（= SCLK）| **GPIO12** | `TFT_SCLK`，FSPI IO_MUX 默认 CLK 脚 |

> 注：8 脚 ST7789 模组实际是单向 SPI，不引出 MISO；代码中 `TFT_MISO=9` 仅作占位，不连屏幕。

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
**第一步排查：把 GPIO11 和 GPIO12 对调**（即 WR↔TE 互换）。

---

## 故障排查

| 现象 | 可能原因 | 解决方法 |
|:---|:---|:---|
| 黑屏，背光也不亮 | VCC/GND 没接 或 BLK 没接 | 检查 1/2/3 脚 |
| 背光亮但全白屏 | RES 一直低电平 / DC 接错 | 检查 4、6 脚 |
| 背光亮但全黑屏 | MOSI / SCLK 接反 | 对调 7、8 脚（GPIO11↔GPIO12）|
| 启动循环 panic | 引脚冲突（如误用 33-37）| 确认未占用 PSRAM 引脚 |
| 显示花屏/雪花 | SPI 频率太高 或 接线过长 | 把 `SPI_FREQUENCY` 降到 20MHz |
| 显示偏移几像素 | CGRAM_OFFSET 配置不对 | 检查 `-DCGRAM_OFFSET=1` |
| 颜色反相 | 控制器 INVERSION 配置 | 加 `-DTFT_INVERSION_ON` 或 `-DTFT_INVERSION_OFF` |

---

## 代码配置同步

引脚号在以下两处同时定义，必须保持一致：

1. **`include/pins.h`** — C++ 业务代码使用
2. **`platformio.ini` 的 `build_flags`** — TFT_eSPI 库使用

当前 `platformio.ini` 配置：

```ini
-DUSE_HSPI_PORT          ; ESP32-S3 必需
-DTFT_MOSI=11            ; FSPI IO_MUX 默认
-DTFT_SCLK=12            ; FSPI IO_MUX 默认
-DTFT_CS=10
-DTFT_DC=13
-DTFT_RST=14
-DTFT_MISO=9             ; 占位（单向 SPI）
-DTFT_BL=-1              ; 背光由 tft_driver.cpp 通过 LEDC 自管
-DTFT_BACKLIGHT_ON=HIGH
```

> `TFT_BL=-1` 是有意为之：背光由 `tft_driver.cpp` 通过 LEDC 通道 1 输出 PWM，
> 不让 TFT_eSPI 库接管 GPIO21，避免冲突。
> `TFT_MISO=9` 也是占位 —— Arduino-ESP32 SPI HAL 把 `-1` 转成 `uint8_t=0xFF` 会触发 GPIO 227 错误，必须填一个真实的空闲 GPIO。

---

## 模组兼容性

| ESP32-S3 模组 | GPIO 33-37 | TFT 引脚建议 |
|:---|:---:|:---|
| **WROOM-1-N8R8（本项目）** | ❌ Octal PSRAM 占用 | **当前方案：11/12/9** |
| WROOM-1-N4 / N8（无 PSRAM）| ✅ 可用 | 可改回 35/36/37 或继续用 11/12/9 |
| WROOM-1-N16R8（同 N8R8）| ❌ Octal PSRAM 占用 | 同当前方案 |

切换模组时请同步修改 `include/pins.h` 与 `platformio.ini`。

---

## 物理连接示意

```
ESP32-S3 DevKitC-1 N8R8           ST7789 240×280
┌──────────────────┐              ┌──────────────┐
│              3V3 ├──────────────┤ 1  VCC       │
│              GND ├──────────────┤ 2  GND       │
│           GPIO21 ├──────────────┤ 3  BLK       │
│           GPIO14 ├──────────────┤ 4  RES       │
│           GPIO10 ├──────────────┤ 5  CS        │
│           GPIO13 ├──────────────┤ 6  DC        │
│           GPIO11 ├──────────────┤ 7  WR (MOSI) │
│           GPIO12 ├──────────────┤ 8  TE (SCLK) │
└──────────────────┘              └──────────────┘
   (GPIO9 不接屏幕，仅作 MISO 占位)
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

---

## 关于 STM32 8 位并口参考例程

附带的 `02-1.69IPS显示屏STM32F103C8T6_8位并口例程` 仅作 ST7789 寄存器序列参考。已对比确认：
- COLMOD=0x05 RGB565 ✅ 与 TFT_eSPI 默认一致
- MADCTL=0x00 (rotation 0) ✅ 一致
- Y 坐标 +0x14 偏移 ✅ 由 `-DCGRAM_OFFSET=1` 自动处理
- INVON 0x21 反色 ✅ TFT_eSPI ST7789 驱动默认启用
- Gamma 表 0xE0/0xE1 与 TFT_eSPI 默认略有差异，仅影响色彩饱和度，不影响亮屏

**结论：寄存器层面无配置错误，原"不亮屏"根因是引脚冲突 PSRAM。**
