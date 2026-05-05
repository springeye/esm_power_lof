## ADDED Requirements

### Requirement: Settings page navigation

The system SHALL provide a settings page accessible from the home screen, with 5 category pages navigable using 3 physical buttons (K1=up, K2=confirm, K3=down).

#### Scenario: Enter settings from home
- **WHEN** user long-presses K2 on the home screen
- **THEN** system loads the settings main page showing the first category (Fan Settings)

#### Scenario: Navigate between settings items
- **WHEN** user is on any settings page
- **THEN** K1 moves focus to previous item, K3 moves focus to next item
- **WHEN** focus is on the first item and user presses K1
- **THEN** focus wraps to the last item
- **WHEN** focus is on the last item and user presses K3
- **THEN** focus wraps to the first item

#### Scenario: Navigate between category pages
- **WHEN** user is on any settings page
- **THEN** long-press K1 switches to previous category page
- **THEN** long-press K3 switches to next category page
- **WHEN** on first page and long-press K1
- **THEN** wraps to last page (page 5)
- **WHEN** on last page and long-press K3
- **THEN** wraps to first page (page 1)

#### Scenario: Return from settings
- **WHEN** user is on settings main page and long-presses K2
- **THEN** system returns to home screen
- **WHEN** user is on a sub-category page and long-presses K2
- **THEN** system returns to settings main page

### Requirement: Settings page visual design

The settings page SHALL display category name, page indicator, and settings items with current values.

#### Scenario: Page header display
- **WHEN** any settings page is displayed
- **THEN** header shows category name and page indicator (e.g., "Fan Settings 1/5")

#### Scenario: Settings item display
- **WHEN** a settings item is not focused
- **THEN** item shows label on left and current value on right

#### Scenario: Focused item display
- **WHEN** a settings item is focused
- **THEN** item background highlights with accent color
- **WHEN** item is focused and not in edit mode
- **THEN** K2 press enters edit mode

### Requirement: Value editing interaction

The system SHALL allow editing numeric values using K1/K3 to adjust and K2 to confirm.

#### Scenario: Enter edit mode
- **WHEN** user presses K2 on a focused settings item
- **THEN** system enters edit mode for that item
- **THEN** value display starts blinking (500ms interval)
- **THEN** up/down arrow icons appear near the value

#### Scenario: Adjust value in edit mode
- **WHEN** in edit mode and user presses K1
- **THEN** value increases by one step
- **WHEN** in edit mode and user presses K3
- **THEN** value decreases by one step
- **WHEN** value reaches minimum and user presses K3
- **THEN** value wraps to maximum
- **WHEN** value reaches maximum and user presses K1
- **THEN** value wraps to minimum

#### Scenario: Confirm value edit
- **WHEN** in edit mode and user presses K2
- **THEN** system saves the new value
- **THEN** system exits edit mode
- **THEN** value stops blinking
- **THEN** arrow icons disappear

#### Scenario: Cancel value edit
- **WHEN** in edit mode and user long-presses K2
- **THEN** system cancels the edit
- **THEN** value reverts to previous value
- **THEN** system exits edit mode

### Requirement: Fan settings page

The system SHALL provide a fan settings page with temperature threshold and speed percentage controls.

#### Scenario: Fan settings items
- **WHEN** user navigates to page 1 (Fan Settings)
- **THEN** page displays:
  - Low temp threshold (20-40°C, step 0.5°C, default 30°C)
  - Mid temp threshold (40-60°C, step 0.5°C, default 50°C)
  - High temp threshold (60-80°C, step 0.5°C, default 70°C)
  - Force temp threshold (70-90°C, step 0.5°C, default 75°C)
  - Min fan speed (10-50%, step 1%, default 20%)
  - Mid fan speed (40-80%, step 1%, default 60%)

### Requirement: Temperature protection page

The system SHALL provide a temperature protection page with warning and shutdown thresholds.

#### Scenario: Temperature protection items
- **WHEN** user navigates to page 2 (Temp Protection)
- **THEN** page displays:
  - Warning threshold (60-85°C, step 0.5°C, default 70°C)
  - Shutdown threshold (70-95°C, step 0.5°C, default 80°C)
  - Hysteresis band (0.5-5°C, step 0.5°C, default 2°C)

### Requirement: Display settings page

The system SHALL provide a display settings page with brightness control.

#### Scenario: Display settings items
- **WHEN** user navigates to page 3 (Display)
- **THEN** page displays:
  - Brightness (0-100%, step 5%, default 80%)

### Requirement: Power configuration page

The system SHALL provide a power configuration page for selecting device design power.

#### Scenario: Power configuration items
- **WHEN** user navigates to page 4 (Power Config)
- **THEN** page displays:
  - Design power (350W/450W/550W/750W, selectable, default 750W)
- **WHEN** user enters edit mode for design power
- **THEN** K1/K3 cycles through options: 350W → 450W → 550W → 750W → 350W

### Requirement: Sensor calibration page

The system SHALL provide a sensor calibration page for NTC temperature offset.

#### Scenario: Sensor calibration items
- **WHEN** user navigates to page 5 (Sensor Cal)
- **THEN** page displays:
  - NTC temp offset (-10.0 to +10.0°C, step 0.1°C, default 0.0°C)
