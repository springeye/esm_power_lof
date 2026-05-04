## ADDED Requirements

### Requirement: Default font replacement
The system SHALL use HarmonyOS Sans (hos) as the default font instead of Montserrat.

#### Scenario: Default font configuration
- **WHEN** LVGL initializes
- **THEN** LV_FONT_DEFAULT SHALL point to hos_14 (HarmonyOS Sans SC Regular 12px)

#### Scenario: Font rendering quality
- **WHEN** any text is rendered on the 240x280 screen
- **THEN** the text SHALL use anti-aliased rendering (bpp=8) for clear display

### Requirement: Montserrat font cleanup
The system SHALL disable unused Montserrat font macros to reduce firmware size.

#### Scenario: Unused font removal
- **WHEN** the firmware is compiled
- **THEN** LV_FONT_MONTSERRAT_14, LV_FONT_MONTSERRAT_16, LV_FONT_MONTSERRAT_20, and LV_FONT_MONTSERRAT_28 macros SHALL be set to 0

#### Scenario: Firmware size reduction
- **WHEN** Montserrat fonts are disabled
- **THEN** the firmware size SHALL decrease compared to the baseline

### Requirement: UI layout preservation
The system SHALL maintain existing UI layout and text sizing after font replacement.

#### Scenario: Screen layout consistency
- **WHEN** the UI is displayed on the 240x280 screen
- **THEN** all text elements SHALL maintain their current positions and sizes

#### Scenario: Font size compatibility
- **WHEN** XML files reference hos_14, hos_regular, or hos_bold_big fonts
- **THEN** the text SHALL display without overflow or truncation

### Requirement: FontAwesome icon preservation
The system SHALL keep FontAwesome icon fonts unchanged.

#### Scenario: Icon rendering
- **WHEN** UI elements display FontAwesome icons
- **THEN** the icons SHALL render correctly using font_awesome_14 or font_awesome_48

### Requirement: Build system compatibility
The system SHALL compile successfully with the new font configuration.

#### Scenario: Native build
- **WHEN** `pio run -e native` is executed
- **THEN** the build SHALL complete without errors

#### Scenario: ESP32-S3 build
- **WHEN** `pio run -e esp32s3` is executed
- **THEN** the build SHALL complete without errors

#### Scenario: Unit tests
- **WHEN** `pio test -e native` is executed
- **THEN** all tests SHALL pass
