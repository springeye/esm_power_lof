#pragma once

// Release 量产板（与 dev 板硬件接线不同）
// 用于 env: release

// TFT 引脚（SCLK=6 / MOSI=7 / MISO=-1 / CS=5 / DC=4 / RST=19 / BL=15）
// 通过 platformio.ini build_flags 注入 TFT_eSPI 库

#define I2C_SDA       9
#define I2C_SCL       10

#define INA_CH1_ADDR  0x40
#define INA_CH2_ADDR  0x41
#define INA_CH3_ADDR  0x44

#define FAN_PWM       18
#define FAN_TACH      8

#define NTC1_ADC_PIN  16
#define NTC2_ADC_PIN  17
#define NTC_ADC_CH    NTC1_ADC_PIN

#define PSON_PIN      20
#define PWOK_PIN      14

#define KEY_K1        1
#define KEY_K2        2
#define KEY_K3        12

#define BOARD_BOOT_PIN  0
#define BOARD_EN_PIN    3
