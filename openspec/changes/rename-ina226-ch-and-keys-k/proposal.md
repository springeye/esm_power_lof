## Why

当前代码中三个 INA226 传感器以电压轨道（LOAD/12V/5V）命名，三个按键以方向（UP/ENTER/DOWN）命名，与硬件实际用途不符。硬件上三个 INA226 分别监控三个 XT60 输出口（CH1/CH2/CH3），三个按键对应物理标识 K1/K2/K3，统一命名可消除歧义并与 PCB 丝印对齐。

## What Changes

- 将 `INA_RAIL_LOAD / INA_RAIL_12V / INA_RAIL_5V` 枚举值重命名为 `INA_CH1 / INA_CH2 / INA_CH3`
- 将 `INA_LOAD_ADDR / INA_12V_ADDR / INA_5V_ADDR` 宏重命名为 `INA_CH1_ADDR / INA_CH2_ADDR / INA_CH3_ADDR`
- 将 `INA226_ADDR_RAIL[3]` 数组重命名为 `INA226_ADDR_CH[3]`
- 将 `app_state` 中的 `load_ma / v12_ma / v5_ma` 及对应 getter/setter 重命名为 `ch1_ma / ch2_ma / ch3_ma`
- 将 `KEY_UP / KEY_ENTER / KEY_DOWN` 引脚宏重命名为 `KEY_K1 / KEY_K2 / KEY_K3`
- 更新所有引用上述符号的调用点（tasks.cpp、fault_guard.cpp、ui_events.cpp、native_main.cpp 等）
- **BREAKING**：`Ina226Rail` 枚举公开 API 变更，`app_state` getter/setter 接口变更

## Capabilities

### New Capabilities

- `ina226-channel-naming`：以 CH1/CH2/CH3 标识三路 XT60 输出口的 INA226 监控通道
- `key-physical-naming`：以 K1/K2/K3 标识三个物理按键，与 PCB 丝印对齐

### Modified Capabilities

## Impact

- `include/pins.h`：INA 地址宏、KEY 引脚宏
- `include/app_config.h`：`INA226_ADDR_RAIL` 数组、`INA226_MAX_CURRENT_A` 注释
- `src/sensors/ina226/ina226.h`：`Ina226Rail` 枚举
- `src/sensors/ina226/ina226.cpp`：枚举引用、地址数组
- `src/app/app_state.h`：atomic 字段、getter/setter
- `src/app/tasks.cpp`：INA 读取调用、KEY 引脚数组
- `src/app/fault_guard.cpp`：过流检测调用
- `src/ui/ui_events.cpp`：按键事件处理
- `src/native_main.cpp`：仿真入口中的 INA 枚举引用
