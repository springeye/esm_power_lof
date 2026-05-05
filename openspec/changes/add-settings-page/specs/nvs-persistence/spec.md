## ADDED Requirements

### Requirement: NVS initialization

The system SHALL initialize NVS storage on startup and load saved configuration values.

#### Scenario: First boot (no saved data)
- **WHEN** device boots for the first time
- **THEN** NVS partition is initialized
- **THEN** config_manager loads default values from app_config.h
- **THEN** default values are saved to NVS for future boots

#### Scenario: Normal boot (with saved data)
- **WHEN** device boots with existing NVS data
- **THEN** config_manager loads values from NVS
- **WHEN** NVS read fails for any key
- **THEN** that key uses default value from app_config.h

### Requirement: NVS write operations

The system SHALL persist configuration changes to NVS when user confirms edits.

#### Scenario: Save on user confirmation
- **WHEN** user confirms a settings change in UI
- **THEN** config_manager::set_xxx() is called
- **THEN** value is written to NVS immediately
- **THEN** NVS write completes before returning to caller

#### Scenario: NVS write failure handling
- **WHEN** NVS write operation fails
- **THEN** error is logged to serial output
- **THEN** runtime value is still updated (UI shows new value)
- **WHEN** device reboots after write failure
- **THEN** previous successfully saved value is loaded

### Requirement: NVS key naming

The system SHALL use consistent key names for NVS storage.

#### Scenario: Key naming convention
- **WHEN** storing configuration values
- **THEN** keys use format: "cfg_<parameter_name>"
- **THEN** examples: "cfg_fan_temp_low", "cfg_fan_temp_mid", "cfg_design_power_w"

### Requirement: NVS data types

The system SHALL use appropriate NVS data types for each parameter.

#### Scenario: Float parameters
- **WHEN** storing temperature thresholds (fan_temp_low, fan_temp_mid, etc.)
- **THEN** use NVS type: float (4 bytes)
- **WHEN** storing NTC offset
- **THEN** use NVS type: float (4 bytes)

#### Scenario: Integer parameters
- **WHEN** storing design power
- **THEN** use NVS type: int32_t (4 bytes)
- **WHEN** storing brightness percentage
- **THEN** use NVS type: int32_t (4 bytes)

### Requirement: NVS partition usage

The system SHALL use the existing NVS partition without modifying the partition table.

#### Scenario: Partition table compatibility
- **WHEN** firmware is flashed to device
- **THEN** uses existing "nvs" partition from partitions/default_8MB.csv
- **THEN** no partition table changes required
- **THEN** NVS namespace: "esm_power_lof"
