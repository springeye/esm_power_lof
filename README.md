# ESP32 风扇控制器固件

基于 ESP32-WROOM-32E-N8 的智能风扇控制器，支持温度监测、PWM 风扇调速、电源管理和 LVGL 显示界面。

## 硬件 BOM 表

| 组件 | 型号 | 说明 |
|------|------|------|
| 主控 | ESP32-WROOM-32E-N8 | 8MB Flash，双核 240MHz |
| 显示屏 | ST7789 240×280 | SPI 接口，16位色深 |
| 温度传感器 | NTC 10kΩ B=3950 | GPIO36 ADC 采集 |
| 电流传感器 | INA226 ×3 | I2C 地址 0x40/0x41/0x44 |
| 风扇 | 4线 PWM 风扇 | 25kHz PWM，PCNT 测速 |
| 电源 | ATX 电源 | PS_ON/PW_OK 控制 |

## 引脚分配

| 功能 | GPIO | 说明 |
|------|------|------|
| TFT_MOSI | 23 | SPI 数据 |
| TFT_SCLK | 18 | SPI 时钟 |
| TFT_CS | 5 | 片选 |
| TFT_DC | 2 | 数据/命令 |
| TFT_RST | 4 | 复位 |
| TFT_BL | 16 | 背光 PWM |
| I2C_SDA | 21 | I2C 数据 |
| I2C_SCL | 22 | I2C 时钟 |
| FAN_PWM | 25 | 风扇 PWM 输出 |
| FAN_TACH | 35 | 风扇转速输入 |
| NTC_ADC | 36 | NTC 温度 ADC |
| PS_ON | 27 | 电源开关（低=开） |
| PW_OK | 34 | 电源就绪信号 |
| KEY_UP | 32 | 上键 |
| KEY_ENTER | 33 | 确认键 |
| KEY_DOWN | 26 | 下键 |

## 目录结构

```
esm_power_lof/
├── include/
│   ├── pins.h          # 引脚单点定义
│   ├── app_config.h    # 全局常量配置
│   └── lv_conf.h       # LVGL 9.x 配置
├── src/
│   ├── hal/            # 硬件抽象层
│   │   ├── i2c_bus.{h,cpp}
│   │   └── spi_bus.{h,cpp}
│   ├── display/        # 显示驱动
│   │   ├── tft_driver.{h,cpp}
│   │   └── lvgl_port.{h,cpp}
│   ├── ui/             # 用户界面
│   │   ├── ui_main.{h,cpp}
│   │   ├── ui_menu.{h,cpp}
│   │   └── ui_events.{h,cpp}
│   ├── sensors/
│   │   ├── ntc/        # NTC 温度传感器
│   │   └── ina226/     # INA226 电流传感器
│   ├── fan/            # 风扇控制
│   │   ├── fan_curve.{h,cpp}
│   │   ├── fan_pwm.{h,cpp}
│   │   └── fan_tach.{h,cpp}
│   ├── power/          # 电源管理
│   │   ├── psu_fsm.{h,cpp}
│   │   └── ps_on.{h,cpp}
│   ├── input/          # 按键输入
│   │   └── keys.{h,cpp}
│   ├── app/            # 应用层
│   │   ├── app_state.{h,cpp}
│   │   ├── tasks.{h,cpp}
│   │   ├── watchdog.{h,cpp}
│   │   └── fault_guard.{h,cpp}
│   └── main.cpp        # 程序入口
├── test/
│   └── native/         # Unity 单元测试
│       ├── test_ntc_beta/
│       ├── test_fan_curve/
│       ├── test_psu_fsm/
│       ├── test_keys_debounce/
│       └── test_hysteresis/
├── partitions/
│   └── default_8MB.csv
└── platformio.ini
```

## 构建命令

### 编译固件（ESP32）

```bash
pio run -e esp32dev
```

### 运行单元测试（native）

```bash
pio test -e native
```

### 静态分析

```bash
pio check -e esp32dev --skip-packages
```

### 查看固件大小

```bash
pio run -e esp32dev -t size
```

### 烧录固件（需连接硬件）

```bash
pio run -e esp32dev -t upload
```

### 串口监视

```bash
pio device monitor -e esp32dev
```

## 功能特性

- **温度监测**：NTC 热敏电阻，β 公式计算，16 次采样取中值
- **风扇调速**：三段分段线性温控曲线（30-50-70°C），10-bit PWM 25kHz
- **转速监测**：PCNT 硬件计数器，4 线风扇 RPM 计算
- **电源管理**：ATX 电源状态机（Off/Standby/Starting/On/Stopping/Fault）
- **电流监测**：3 路 INA226（12V/5V/负载），I2C 400kHz
- **显示界面**：LVGL 9.x，ST7789 240×280，主仪表盘 + 设置菜单
- **故障保护**：过温/堵转/过流/PWOK 失稳自动关机

## 注意事项

- 本固件仅经过编译验证，**未经实际硬件测试**
- 烧录前请确认引脚连接正确，避免损坏硬件
- 不支持 WiFi/BLE/OTA（节省 Flash 空间）
- 不支持 NVS 持久化（重启后恢复默认设置）

## 许可证

MIT License
