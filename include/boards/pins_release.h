#pragma once

// Release 量产板（与 dev 板硬件接线不同）
// 用于 env: release

// TFT 引脚（SCLK=6 / MOSI=7 / MISO=-1 / CS=5 / DC=4 / RST=8）经 build_flags 注入 TFT_eSPI
// TFT_BL 不传给 TFT_eSPI（避免 #ifdef TFT_BL 抢占背光、与 LEDC 冲突），
// 由 src/display/tft_driver.cpp 经 LEDC 自管。见 AGENTS.md #2。
#define TFT_BL        15

#define I2C_SDA       9
#define I2C_SCL       10

#define INA_CH1_ADDR  0x40
#define INA_CH2_ADDR  0x41
#define INA_CH3_ADDR  0x44

#define FAN_PWM       18
#define FAN_TACH      13

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
