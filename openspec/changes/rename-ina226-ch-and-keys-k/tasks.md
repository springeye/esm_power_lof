## 1. INA226 枚举与地址宏重命名

- [x] 1.1 修改 `include/pins.h`：将 `INA_LOAD_ADDR`/`INA_12V_ADDR`/`INA_5V_ADDR` 重命名为 `INA_CH1_ADDR`/`INA_CH2_ADDR`/`INA_CH3_ADDR`
- [x] 1.2 修改 `include/app_config.h`：将 `INA226_ADDR_RAIL[3]` 重命名为 `INA226_ADDR_CH[3]`
- [x] 1.3 修改 `src/sensors/ina226/ina226.h`：将 `Ina226Rail` 枚举值 `INA_RAIL_LOAD`/`INA_RAIL_12V`/`INA_RAIL_5V` 重命名为 `INA_CH1`/`INA_CH2`/`INA_CH3`，更新注释中的地址说明
- [x] 1.4 修改 `src/sensors/ina226/ina226.cpp`：更新 `s_addrs[]` 初始化中的枚举引用，更新边界检查 `rail > INA_RAIL_5V` 为 `rail > INA_CH3`，更新 `INA226_ADDR_RAIL` 为 `INA226_ADDR_CH`

## 2. app_state 字段与接口重命名

- [x] 2.1 修改 `src/app/app_state.h`：将 `load_ma`/`v12_ma`/`v5_ma` atomic 字段重命名为 `ch1_ma`/`ch2_ma`/`ch3_ma`
- [x] 2.2 修改 `src/app/app_state.h`：将 getter `get_load_a()`/`get_v12_a()`/`get_v5_a()` 重命名为 `get_ch1_a()`/`get_ch2_a()`/`get_ch3_a()`
- [x] 2.3 修改 `src/app/app_state.h`：将 setter `set_load_ma()`/`set_v12_ma()`/`set_v5_ma()` 重命名为 `set_ch1_ma()`/`set_ch2_ma()`/`set_ch3_ma()`

## 3. 按键引脚宏重命名

- [x] 3.1 修改 `include/pins.h`：将 `KEY_UP`/`KEY_ENTER`/`KEY_DOWN` 重命名为 `KEY_K1`/`KEY_K2`/`KEY_K3`，保持 GPIO 号不变（32/33/26）

## 4. 调用点更新

- [x] 4.1 修改 `src/app/tasks.cpp`：更新 `ina226_read()` 调用中的枚举参数（`INA_RAIL_LOAD/12V/5V` → `INA_CH1/CH2/CH3`），更新 `set_load_ma/v12_ma/v5_ma` 为 `set_ch1_ma/ch2_ma/ch3_ma`，更新 `key_pins[3]` 数组为 `{KEY_K1, KEY_K2, KEY_K3}`
- [x] 4.2 修改 `src/app/fault_guard.cpp`：更新 `INA226_MAX_CURRENT_A` 相关注释中的通道描述（如有旧命名）
- [x] 4.3 修改 `src/ui/ui_events.cpp`：更新按键 ID 比较逻辑，将 `KEY_UP`/`KEY_ENTER`/`KEY_DOWN` 替换为 `KEY_K1`/`KEY_K2`/`KEY_K3`
- [x] 4.4 修改 `src/ui/ui_events.h`：更新函数注释中的按键 ID 说明
- [x] 4.5 修改 `src/native_main.cpp`：更新仿真入口中的 `INA_RAIL_LOAD/12V/5V` 枚举引用为 `INA_CH1/CH2/CH3`

## 5. 验证

- [x] 5.1 执行 `pio run -e native`，确认编译链接成功
- [x] 5.2 执行 `pio test -e native`，确认 33/33 测试通过
- [x] 5.3 用 `rg` 全库搜索旧符号（`INA_RAIL_`、`INA_LOAD_ADDR`、`INA_12V_ADDR`、`INA_5V_ADDR`、`INA226_ADDR_RAIL`、`load_ma`、`v12_ma`、`v5_ma`、`KEY_UP`、`KEY_ENTER`、`KEY_DOWN`），确认无残留
