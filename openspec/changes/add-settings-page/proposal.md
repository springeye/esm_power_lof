## Why

当前ESP32风扇控制器固件的所有配置参数（风扇曲线、温度阈值、亮度等）均为编译期常量，用户无法在运行时调整。这导致：
- 不同使用场景（静音/性能）需要重新编译固件
- 硬件差异（如NTC传感器偏差）无法现场校准
- 用户无法根据实际电源功率（350W-750W）计算真实的负载百分比

需要一个设置页面，让用户通过3个物理按键（上/中/下）在设备上直接调整常用参数，并通过NVS持久化保存。

## What Changes

- **新增设置页面UI**：5个分类页面（风扇设置、温度保护、显示设置、功率配置、传感器校准）
- **新增配置管理模块**：`config_manager` 管理运行时配置，替代直接引用 `app_config.h` 常量
- **新增NVS持久化**：使用ESP32 Preferences保存用户设置，重启后保留
- **修改风扇控制**：`fan_curve.cpp` 读取运行时配置而非编译期常量
- **修改故障保护**：`fault_guard.cpp` 读取运行时配置
- **修改数据桥**：`data_bridge.cpp` 使用配置的功率值计算负载百分比

## Capabilities

### New Capabilities

- `settings-ui`: 设置页面的LVGL UI实现，包括5个分类页面、按键导航、数值编辑交互
- `config-management`: 运行时配置管理模块，提供getter/setter接口，管理配置生命周期
- `nvs-persistence`: NVS持久化层，保存/加载用户配置，处理首次启动默认值

### Modified Capabilities

（无现有spec需要修改）

## Impact

- **代码变更**：
  - 新增：`src/app/config_manager.{h,cpp}`，`src/ui_bridge/settings_bridge.cpp`
  - 新增：`ui/screens/settings_*.xml` → 生成 `*_gen.c/h`
  - 修改：`src/fan/fan_curve.cpp`，`src/app/fault_guard.cpp`，`src/ui_bridge/data_bridge.cpp`
  - 修改：`src/main.cpp`（初始化config_manager）
  - 修改：`ui/lof_power_system.c`（注册设置页面）

- **依赖**：
  - ESP32 Preferences库（NVS封装）
  - LVGL（已存在）

- **硬件约束**：
  - 3个物理按键（K1=上，K2=确认，K3=下）
  - 240x280 ST7789 TFT（非触摸）
  - ESP32-S3（已有NVS分区）
