#include "sensors/ina226/ina226.h"
#include "app_config.h"
#include "pins.h"
#include <Wire.h>

// INA226 register addresses
static const uint8_t REG_CONFIG      = 0x00;
static const uint8_t REG_SHUNT_V     = 0x01;
static const uint8_t REG_BUS_V       = 0x02;
static const uint8_t REG_POWER       = 0x03;
static const uint8_t REG_CURRENT     = 0x04;
static const uint8_t REG_CALIBRATION = 0x05;

// INA226 config: 16 averages, 1.1ms conversion, continuous shunt+bus
static const uint16_t INA226_CONFIG = 0x4727u;

// Calibration = 0.00512 / (shunt_ohms * max_current_a)
// = 0.00512 / (0.002 * 40) = 64
static const uint16_t INA226_CAL = 64u;

// LSB for current: max_current / 2^15 = 40 / 32768 ≈ 1.221mA
static const float CURRENT_LSB_A = 40.0f / 32768.0f;

static const uint8_t s_addrs[3] = {
    INA226_ADDR_CH[0],  // 0x40 CH1
    INA226_ADDR_CH[1],  // 0x41 CH2
    INA226_ADDR_CH[2]   // 0x44 CH3
};

static bool ina226_write_reg(uint8_t addr, uint8_t reg, uint16_t val) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    Wire.write(static_cast<uint8_t>(val >> 8));
    Wire.write(static_cast<uint8_t>(val & 0xFF));
    return Wire.endTransmission() == 0;
}

static bool ina226_read_reg(uint8_t addr, uint8_t reg, int16_t *out) {
    Wire.beginTransmission(addr);
    Wire.write(reg);
    if (Wire.endTransmission(false) != 0) return false;
    if (Wire.requestFrom(static_cast<int>(addr), 2) != 2) return false;
    uint16_t raw = (static_cast<uint16_t>(Wire.read()) << 8)
                   | static_cast<uint16_t>(Wire.read());
    *out = static_cast<int16_t>(raw);
    return true;
}

void ina226_init_all(void) {
    for (uint8_t i = 0; i < 3u; i++) {
        ina226_write_reg(s_addrs[i], REG_CONFIG, INA226_CONFIG);
        ina226_write_reg(s_addrs[i], REG_CALIBRATION, INA226_CAL);
    }
}

bool ina226_read(Ina226Rail rail, Ina226Data *out) {
    if (out == nullptr || rail > INA_CH3) return false;

    uint8_t addr = s_addrs[static_cast<uint8_t>(rail)];
    int16_t raw_bus = 0, raw_cur = 0;

    if (!ina226_read_reg(addr, REG_BUS_V, &raw_bus)) {
        out->valid = false;
        return false;
    }
    if (!ina226_read_reg(addr, REG_CURRENT, &raw_cur)) {
        out->valid = false;
        return false;
    }

    // Bus voltage LSB = 1.25mV
    out->voltage_v = static_cast<float>(raw_bus) * 0.00125f;
    out->current_a = static_cast<float>(raw_cur) * CURRENT_LSB_A;
    out->power_w   = out->voltage_v * out->current_a;
    out->valid     = true;
    return true;
}
