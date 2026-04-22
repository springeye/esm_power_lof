#pragma once
#include <cstdint>

/**
 * INA226 power rail data (design.md D6)
 */
typedef struct {
    float voltage_v;   // Bus voltage in Volts
    float current_a;   // Current in Amperes
    float power_w;     // Power in Watts
    bool  valid;       // true if last read succeeded
} Ina226Data;

/**
 * Channel index mapping to INA226_ADDR_CH[]
 */
typedef enum {
    INA_CH1 = 0,  // 0x40
    INA_CH2 = 1,  // 0x41
    INA_CH3 = 2   // 0x44
} Ina226Rail;

/**
 * @brief Initialize all 3 INA226 sensors.
 *
 * Configures shunt resistor and max current for each sensor.
 * Must be called after i2c_bus_init().
 */
void ina226_init_all(void);

/**
 * @brief Read power data from one INA226 rail.
 *
 * @param rail   Channel index (INA_CH1/CH2/CH3)
 * @param out    Output data struct
 * @return true on success, false on I2C error
 */
bool ina226_read(Ina226Rail rail, Ina226Data *out);
