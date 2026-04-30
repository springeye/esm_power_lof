## ADDED Requirements

### Requirement: PlatformIO 工程结构

The system SHALL 提供一个完整的 PlatformIO + Arduino 工程结构，包含 `platformio.ini`、`src/`、`include/`、`lib/`、`partitions/` 与 `.gitignore`，使开发者克隆仓库后无需额外手工步骤即可执行构建。

#### Scenario: 克隆后首次构建
- **WHEN** 开发者克隆仓库并执行 `pio run`
- **THEN** PlatformIO 自动安装所有依赖并完成编译，退出码为 0

#### Scenario: 工程基础文件齐全
- **WHEN** 检查仓库根目录
- **THEN** 必须存在 `platformio.ini`、`src/main.cpp`、`include/pins.h`、`include/lv_conf.h`、`include/app_config.h`、`partitions/default_8MB.csv`、`.gitignore`

### Requirement: 目标硬件与构建配置

The system SHALL 在 `platformio.ini` 中将构建目标固定为 ESP32-WROOM-32E-N8（8MB flash），framework 为 arduino，monitor 波特率为 115200。

#### Scenario: 构建目标参数
- **WHEN** 解析 `platformio.ini` 中的 `[env:esp32dev]` 段
- **THEN** `platform` 为 `espressif32@^6.7.0`，`board` 为 `esp32dev`，`framework` 为 `arduino`，`board_build.partitions` 为 `partitions/default_8MB.csv`，`board_upload.flash_size` 为 `8MB`，`monitor_speed` 为 `115200`

#### Scenario: 依赖锁定
- **WHEN** 解析 `lib_deps`
- **THEN** 必须显式包含 `bodmer/TFT_eSPI@^2.5.43`、`lvgl/lvgl@^9.2.2`、`robtillaart/INA226@^0.6.0`，并锁定主版本范围

### Requirement: 模块化目录分层

The system SHALL 按 HAL / Driver / Service / App 分层组织源码，每个外设有独立子目录，禁止把多模块逻辑塞进 `main.cpp`。

#### Scenario: 目录结构
- **WHEN** 检查 `src/` 子目录
- **THEN** 必须存在 `hal/`、`display/`、`ui/`、`sensors/ntc/`、`sensors/ina226/`、`fan/`、`power/`、`input/`、`app/` 子目录

#### Scenario: main.cpp 职责
- **WHEN** 阅读 `src/main.cpp`
- **THEN** 仅包含 `setup()` / `loop()` 框架与各模块 `init()` / 任务创建调用，不包含具体业务逻辑

### Requirement: 引脚分配单点定义

The system SHALL 将所有外设 GPIO 编号集中定义在 `include/pins.h`，业务代码不得使用裸数字。

#### Scenario: 修改引脚不影响业务代码
- **WHEN** 仅修改 `include/pins.h` 中某外设的 GPIO 常量
- **THEN** 重新编译即可切换引脚，无需改动其它源文件
