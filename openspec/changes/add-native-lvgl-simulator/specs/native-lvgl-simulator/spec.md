## ADDED Requirements

### Requirement: Native LVGL initialization
The system SHALL initialize LVGL 9.x on PC without Arduino dependencies when `BUILD_NATIVE` is defined.

#### Scenario: LVGL init on PC
- **WHEN** native executable starts
- **THEN** `lv_init()` completes successfully without Arduino `millis()` dependency

### Requirement: SDL2 display output
The system SHALL render LVGL UI to an SDL2 window at 240×280 resolution.

#### Scenario: Window displays UI
- **WHEN** native executable runs
- **THEN** an SDL2 window opens showing the LVGL UI at 240×280 pixels

#### Scenario: Window closes cleanly
- **WHEN** user closes the SDL2 window
- **THEN** the process exits cleanly without memory leaks

### Requirement: LVGL tick advancement
The system SHALL advance LVGL tick using `std::chrono` instead of Arduino `millis()`.

#### Scenario: Tick advances at 1ms
- **WHEN** native executable runs for 1 second
- **THEN** LVGL tick advances by approximately 1000ms

### Requirement: UI source code reuse
The system SHALL compile LVGL Editor generated UI sources (`ui/*.c`) without modification.

#### Scenario: Generated UI compiles on PC
- **WHEN** `pio run -e native` executes
- **THEN** all UI sources (`lof_power_system_gen.c`, `home_gen.c`, `splash_gen.c`, font files) compile without errors

### Requirement: Keyboard input mapping
The system SHALL map keyboard keys to LVGL input events for K1/K2/K3 buttons.

#### Scenario: Key press triggers LVGL event
- **WHEN** user presses '1' key (mapped to K1)
- **THEN** LVGL receives a `LV_EVENT_PRESSED` event on the mapped input device

### Requirement: Build command compatibility
The system SHALL support building via `pio run -e native` without additional manual steps.

#### Scenario: Build succeeds
- **WHEN** user runs `pio run -e native`
- **THEN** build completes with exit code 0

### Requirement: lv_conf.h conditional compilation
The system SHALL use `BUILD_NATIVE` macro to conditionally configure LVGL for PC builds.

#### Scenario: ESP32 build unaffected
- **WHEN** `pio run -e esp32s3` executes
- **THEN** `lv_conf.h` uses Arduino `millis()` for tick (existing behavior)

#### Scenario: Native build uses std::chrono
- **WHEN** `pio run -e native` executes
- **THEN** `lv_conf.h` disables `LV_TICK_CUSTOM` (tick handled by native code)
