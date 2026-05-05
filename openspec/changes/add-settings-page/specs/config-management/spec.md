## ADDED Requirements

### Requirement: Runtime configuration storage

The system SHALL provide a config_manager module that stores runtime configuration values, replacing direct references to compile-time constants.

#### Scenario: Default values initialization
- **WHEN** config_manager::init() is called
- **THEN** all configuration values are initialized from app_config.h defaults
- **THEN** all values are available via getter functions

#### Scenario: Thread-safe access
- **WHEN** multiple FreeRTOS tasks access config_manager concurrently
- **THEN** all reads and writes are atomic and consistent
- **THEN** no data races occur

### Requirement: Configuration getters

The system SHALL provide getter functions for all configurable parameters.

#### Scenario: Fan curve parameters
- **WHEN** fan_curve.cpp calls config_manager::get_fan_temp_low()
- **THEN** returns current low temperature threshold (float, °C)
- **WHEN** fan_curve.cpp calls config_manager::get_fan_pwm_min_percent()
- **THEN** returns current minimum fan speed percentage (float, %)

#### Scenario: Temperature protection parameters
- **WHEN** fault_guard.cpp calls config_manager::get_over_temp_c()
- **THEN** returns current over-temperature threshold (float, °C)
- **WHEN** fault_guard.cpp calls config_manager::get_fan_hysteresis()
- **THEN** returns current hysteresis band (float, °C)

#### Scenario: Display parameters
- **WHEN** display module calls config_manager::get_brightness_percent()
- **THEN** returns current brightness percentage (int, 0-100)

#### Scenario: Power parameters
- **WHEN** data_bridge.cpp calls config_manager::get_design_power_w()
- **THEN** returns current design power in watts (int, 350/450/550/750)

#### Scenario: Sensor calibration parameters
- **WHEN** ntc.cpp calls config_manager::get_ntc_temp_offset()
- **THEN** returns current NTC temperature offset (float, °C)

### Requirement: Configuration setters

The system SHALL provide setter functions that update runtime values and trigger NVS persistence.

#### Scenario: Set fan curve parameter
- **WHEN** settings UI calls config_manager::set_fan_temp_low(25.0f)
- **THEN** updates runtime value to 25.0°C
- **THEN** saves to NVS
- **WHEN** fan_curve.cpp next reads get_fan_temp_low()
- **THEN** returns 25.0°C

#### Scenario: Set design power
- **WHEN** settings UI calls config_manager::set_design_power_w(450)
- **THEN** updates runtime value to 450W
- **THEN** saves to NVS
- **WHEN** data_bridge.cpp next reads get_design_power_w()
- **THEN** returns 450

### Requirement: Configuration value bounds

The system SHALL enforce valid ranges for all configuration values.

#### Scenario: Value clamping
- **WHEN** setter receives value below minimum
- **THEN** value is clamped to minimum
- **WHEN** setter receives value above maximum
- **THEN** value is clamped to maximum

#### Scenario: Valid design power values
- **WHEN** set_design_power_w() receives value
- **THEN** value MUST be one of: 350, 450, 550, 750
- **WHEN** value is not valid
- **THEN** value is rejected and previous value is retained

### Requirement: Configuration reset

The system SHALL provide a function to reset all configuration to defaults.

#### Scenario: Reset to defaults
- **WHEN** config_manager::reset_to_defaults() is called
- **THEN** all configuration values are reset to app_config.h defaults
- **THEN** NVS storage is cleared
- **WHEN** getters are called after reset
- **THEN** return default values
