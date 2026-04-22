#pragma once

// TFT SPI Display (design.md D2)
#define TFT_MOSI  23
#define TFT_SCLK  18
#define TFT_CS     5
#define TFT_DC     2
#define TFT_RST    4
#define TFT_BL    16

// I2C Bus (design.md D2)
#define I2C_SDA   21
#define I2C_SCL   22

// INA226 I2C Addresses (design.md D2)
// 7-bit addr / 8-bit write / 8-bit read
#define INA_CH1_ADDR  0x40  // 0x40 / 0x80 / 0x81
#define INA_CH2_ADDR  0x41  // 0x41 / 0x82 / 0x83
#define INA_CH3_ADDR  0x44  // 0x44 / 0x88 / 0x89

// Fan PWM & Tach (design.md D2)
#define FAN_PWM   25
#define FAN_TACH  35  // input only, ADC1_CH7

// NTC Temperature Sensor (design.md D2)
#define NTC_ADC_CH  36  // SENSOR_VP, input only

// PSU Control (design.md D2)
#define PSON_PIN  27  // output: LOW=on, HIGH=off
#define PWOK_PIN  34  // input only, no internal pullup

// Keys (design.md D2)
#define KEY_K1  32
#define KEY_K2  33
#define KEY_K3  26

// Note: GPIO6-11 reserved for flash, GPIO34-39 input-only
