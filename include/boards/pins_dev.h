#pragma once

// ESP32-S3-DevKitC-1 开发板（N8R8 模组）
// 用于 env: esp32s3 / test

// TFT SPI 显示屏（ST7789 240x280）：TFT_eSPI 库引脚通过 build_flags 注入
// TFT_BL 不传给 TFT_eSPI（避免 pinMode(-1) bug），由 src/display/tft_driver.cpp 自管
#define TFT_BL        21

#define I2C_SDA       41
#define I2C_SCL       42

#define INA_CH1_ADDR  0x40
#define INA_CH2_ADDR  0x41
#define INA_CH3_ADDR  0x44

#define FAN_PWM       16
#define FAN_TACH      17

#define NTC_ADC_CH    1

#define PSON_PIN      15
#define PWOK_PIN      18

#define KEY_K1        38
#define KEY_K2        39
#define KEY_K3        40
