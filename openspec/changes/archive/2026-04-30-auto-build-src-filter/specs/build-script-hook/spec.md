## ADDED Requirements

### Requirement: PlatformIO extra_scripts integration
The system SHALL integrate as a PlatformIO `extra_scripts` hook, running automatically before each build without requiring any manual invocation.

#### Scenario: Script runs automatically on build
- **WHEN** developer runs `pio run -e native` or `pio run -e esp32s3`
- **THEN** the auto-discovery script executes before compilation begins
- **AND** no additional command-line flags or manual steps are needed

#### Scenario: Script uses only Python standard library
- **WHEN** the script is loaded by PlatformIO's SCons environment
- **THEN** all imports SHALL resolve to Python standard library modules (`os`, `re`, `glob`, `pathlib`)
- **AND** no external `pip install` is required

### Requirement: Non-invasive SRC_FILTER injection
The system SHALL append auto-discovered filter rules to the build environment's `SRC_FILTER`, rather than replacing or modifying `platformio.ini` content.

#### Scenario: Handwritten rules preserved
- **WHEN** `platformio.ini` contains handwritten `build_src_filter` entries
- **THEN** those entries remain active and are NOT removed by the script
- **AND** auto-discovered entries are added as additional rules

#### Scenario: Script failure does not block build
- **WHEN** the auto-discovery script encounters an error (e.g., missing `file_list_gen.cmake`)
- **THEN** the script logs an ERROR message but falls back to existing handwritten `build_src_filter` rules
- **AND** the build proceeds with the handwritten rules as baseline

### Requirement: Build output transparency
The system SHALL print a summary of auto-discovered source files at the start of each build, aiding debugging and verification.

#### Scenario: Verbose summary on build
- **WHEN** the script completes source discovery
- **THEN** it prints a categorized summary to the build console:
  - Number of UI files discovered
  - Number of business logic files discovered
  - Any platform-specific exclusions applied
- **AND** includes a `--verbose` mode (via `build_flags = -DAUTO_SRC_VERBOSE`) that lists every file path

### Requirement: Compatibility with existing scripts
The system SHALL coexist with `scripts/use_msys2_mingw.py` and any other existing `extra_scripts`.

#### Scenario: Multiple extra_scripts coexist
- **WHEN** `platformio.ini` has `extra_scripts = pre:scripts/use_msys2_mingw.py` AND `pre:scripts/auto_src_filter.py`
- **THEN** both scripts execute in declaration order
- **AND** the auto-discovery script's `SRC_FILTER` modifications are visible to the build
