## ADDED Requirements

### Requirement: Auto-discover UI source files from file_list_gen.cmake
The system SHALL parse `ui/file_list_gen.cmake` to extract all `LV_EDITOR_PROJECT_SOURCES` entries and convert them to `build_src_filter` format (`+<../ui/<relative-path>>`) for the current PlatformIO environment.

#### Scenario: Normal UI file list parsing
- **WHEN** `ui/file_list_gen.cmake` contains a list of font, screen, and entry source files
- **THEN** the script extracts all file paths and generates corresponding `+<../ui/...>` SRC_FILTER entries
- **AND** each generated path uses forward slashes regardless of host OS

#### Scenario: file_list_gen.cmake is missing or empty
- **WHEN** `ui/file_list_gen.cmake` does not exist or contains no `LV_EDITOR_PROJECT_SOURCES` entries
- **THEN** the script logs a WARNING and falls back to scanning `ui/**/*.c` for a best-effort file list

#### Scenario: New LVGL Editor export adds a screen
- **WHEN** developer runs LVGL Editor to export a new screen (e.g., `settings.xml` → `screens/settings_gen.c`)
- **THEN** the next `pio run` automatically includes `+<../ui/screens/settings_gen.c>` without any manual edit to `platformio.ini`

### Requirement: Auto-discover business logic source files for native platform
For the `[env:native]` environment, the system SHALL auto-discover all portable `.cpp` and `.c` files under `src/` using a glob-based approach, excluding hardware-specific directories and files.

#### Scenario: New module added under src/
- **WHEN** developer creates `src/new_module/module.cpp` and `src/new_module/module.h`
- **THEN** the next `pio run -e native` automatically compiles `module.cpp`
- **AND** no manual edit to `platformio.ini` is required

#### Scenario: Hardware-specific files excluded from native
- **WHEN** `src/hal/` contains hardware abstraction files
- **THEN** those files SHALL NOT be included in the native build
- **AND** files in `src/display/tft_driver.cpp` and `src/display/lvgl_port.cpp` SHALL NOT be included (native variants are used instead)

#### Scenario: Native-specific stubs auto-detected
- **WHEN** files matching `*_native.cpp` or `*_native.c` exist in any `src/` subdirectory
- **THEN** those files SHALL be included in the native build
- **AND** their non-native counterparts (same name without `_native`) SHALL NOT be included

### Requirement: Platform-specific file convention
The system SHALL define and enforce the following conventions for automatic platform assignment:

| Convention | Target Platform |
|------------|----------------|
| `src/hal/**/*.cpp` | esp32s3 only |
| `src/display/tft_driver.cpp` | esp32s3 only (has `tft_driver_native.cpp` counterpart) |
| `src/display/lvgl_port.cpp` | esp32s3 only (has `lvgl_port_native.cpp` counterpart) |
| `src/**/*_native.cpp`, `src/**/*_native.c` | native only |
| `src/native/**/*.cpp` | native only |
| All other `src/**/*.cpp`, `src/**/*.c` | both platforms |
| `ui/**/*.c`, `ui/**/*.h` | both platforms |

#### Scenario: Developer creates a native stub correctly
- **WHEN** developer creates `src/display/new_driver_native.cpp` for a new display driver
- **THEN** the file is automatically included in native builds
- **AND** `src/display/new_driver.cpp` (if it exists) is excluded from native builds

#### Scenario: Developer violates naming convention
- **WHEN** developer creates `src/hal/portable_logic.cpp` (a file that should compile on both platforms but is placed in `hal/`)
- **THEN** the script logs a WARNING message indicating the file will NOT be compiled for native
- **AND** the build proceeds without including the file

### Requirement: Legacy UI exclusion preserved
The system SHALL automatically exclude `ui/_legacy/` and `src/ui/_legacy/` directories from all builds, matching the existing manual `-<ui/_legacy/>` filter behavior.

#### Scenario: Legacy files remain excluded
- **WHEN** `src/ui/_legacy/` contains old UI implementation files
- **THEN** those files SHALL NOT be compiled in any environment
- **AND** the exclusion is automatic and does not need to be manually specified in `platformio.ini`
