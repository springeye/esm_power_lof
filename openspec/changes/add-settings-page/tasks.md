## 1. Config Manager Module

- [ ] 1.1 Create `src/app/config_manager.h` with class definition
  - Define ConfigManager class with private storage for all configurable values
  - Declare getter/setter functions for each parameter
  - Add init(), reset_to_defaults(), load_from_nvs(), save_to_nvs() methods

- [ ] 1.2 Create `src/app/config_manager.cpp` with implementation
  - Implement default value initialization from app_config.h
  - Implement NVS read/write using Preferences library
  - Implement value bounds clamping
  - Implement thread-safe access (std::atomic or mutex)

- [ ] 1.3 Add config_manager to build system
  - Add `src/app/config_manager.cpp` to platformio.ini build_src_filter
  - Verify compilation in esp32s3 environment

## 2. NVS Persistence Layer

- [ ] 2.1 Initialize NVS in main.cpp
  - Add Preferences library initialization in setup()
  - Add config_manager::init() call after NVS init
  - Handle first-boot default value saving

- [ ] 2.2 Implement NVS key management
  - Define key naming convention: "cfg_<parameter_name>"
  - Implement namespace: "esm_power_lof"
  - Handle NVS read/write errors gracefully

## 3. Modify Fan Curve Module

- [ ] 3.1 Update fan_curve.cpp to use config_manager
  - Replace FAN_TEMP_LOW/MID/HIGH/FORCE with config_manager getters
  - Replace FAN_PWM_MIN/MAX with config_manager getters
  - Add mid_duty to config_manager (currently hardcoded 614)

- [ ] 3.2 Update fan_curve.h with new dependencies
  - Add #include "app/config_manager.h"
  - Update function signatures if needed

## 4. Modify Fault Guard Module

- [ ] 4.1 Update fault_guard.cpp to use config_manager
  - Replace OVER_TEMP_C (hardcoded 80°C) with config_manager getter
  - Replace STALL_RPM_THRESH (hardcoded 100) with config_manager getter
  - Replace FAN_STALL_DUTY_THRESH/FAN_STALL_TIMEOUT_MS with config_manager getters

## 5. Modify Data Bridge

- [ ] 5.1 Update data_bridge.cpp to use config_manager
  - Replace DEVICE_DESIGN_POWER_W (hardcoded 750) with config_manager getter
  - Update load percentage calculation to use dynamic power value

## 6. Settings UI Implementation

- [x] 6.1 Create LVGL Editor XML template
  - Created `ui/screens/settings.xml` with container structure
  - Header bar: back icon + category title + page indicator (1/5)
  - Content area: scrollable container for dynamic settings items
  - Bottom bar: operation hints (K1/K3: Move, K2: Edit, Long K2: Back)
  - Styles defined: normal, focused, editing states for items

- [ ] 6.2 User exports settings_gen.c/h from LVGL Editor
  - Open settings.xml in LVGL Editor
  - Export to ui/screens/settings_gen.c and settings_gen.h
  - Verify generated code compiles

- [ ] 6.3 Create settings_ui.h header file
  - Define SettingsUI class or namespace
  - Declare init(), show(), hide(), handle_key() functions
  - Define settings page enum (FAN, TEMP, DISPLAY, POWER, SENSOR)
  - Define settings item struct (label, value pointer, type, min, max, step)

- [ ] 6.4 Create settings_ui.cpp with LVGL implementation
  - Implement dynamic settings item creation in content_area
  - Implement focus management with visual feedback
  - Implement value display with formatting (temperature, percentage, etc.)
  - Implement edit mode with blinking animation (500ms interval)
  - Implement arrow icons (up/down) during edit mode

- [ ] 6.5 Implement keyboard navigation
  - K1/K3: Move focus between items (wrap around)
  - K2 short press: Enter/confirm edit mode
  - K2 long press: Return to previous level
  - Long press K1/K3: Switch between category pages

- [ ] 6.6 Implement value editing
  - Implement numeric value adjustment with K1/K3
  - Implement value bounds enforcement (min/max clamping)
  - Implement design power selection (350/450/550/750W cycle)
  - Implement visual feedback (blinking value, arrow icons)

- [ ] 6.7 Create 5 category page data definitions
  - Page 1: Fan Settings (6 items: temp_low/mid/high/force + pwm_min/mid_percent)
  - Page 2: Temp Protection (3 items: warning/shutdown thresholds + hysteresis)
  - Page 3: Display Settings (1 item: brightness 0-100%)
  - Page 4: Power Config (1 item: design power 350/450/550/750W)
  - Page 5: Sensor Cal (1 item: NTC offset ±10°C)

## 7. Screen Manager Integration

- [ ] 7.1 Update screen_manager.cpp to support settings page
  - Add settings page creation in init()
  - Add settings page load/unload functions
  - Handle transitions: home → settings → home

- [ ] 7.2 Update input_bridge.cpp for settings navigation
  - Add long-press K2 detection for entering settings
  - Route key events to settings_ui when in settings mode
  - Handle return from settings to home

## 8. Build System Updates

- [ ] 8.1 Update platformio.ini
  - Add Preferences library dependency
  - Add new source files to build_src_filter
  - Verify no build conflicts

- [ ] 8.2 Update include paths
  - Ensure config_manager.h is accessible from all modules
  - Verify no circular dependencies

## 9. Testing

- [ ] 9.1 Create unit tests for config_manager
  - Test default value initialization
  - Test getter/setter functions
  - Test value bounds clamping
  - Test reset_to_defaults()

- [ ] 9.2 Create integration tests for NVS persistence
  - Test first-boot initialization
  - Test save/load cycle
  - Test NVS error handling

- [ ] 9.3 Create UI interaction tests
  - Test keyboard navigation
  - Test value editing flow
  - Test page switching

## 10. Documentation

- [ ] 10.1 Update AGENTS.md
  - Add config_manager module description
  - Add settings UI architecture notes
  - Update code map with new files

- [ ] 10.2 Update README.md
  - Add settings page feature description
  - Add configuration parameters list
