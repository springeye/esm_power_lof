## Context

项目为 ESP32 风扇控制器，使用三个 INA226 监控三路 XT60 输出口电流，三个物理按键控制 PSU 开关机和 UI 导航。当前代码以电压轨道（LOAD/12V/5V）和方向（UP/ENTER/DOWN）命名，与 PCB 丝印（CH1/CH2/CH3、K1/K2/K3）不一致，增加了硬件调试和代码维护的认知负担。

## Goals / Non-Goals

**Goals:**
- 将所有 INA226 相关符号统一为 CH1/CH2/CH3 命名体系
- 将所有按键引脚宏统一为 K1/K2/K3 命名体系
- 保持 I2C 地址映射（0x40→CH1, 0x41→CH2, 0x44→CH3）不变
- 保持按键引脚号（GPIO 32/33/26）不变
- 所有调用点同步更新，编译无警告

**Non-Goals:**
- 不修改 INA226 驱动逻辑或采样算法
- 不修改按键去抖逻辑
- 不调整 UI 显示文字（UI 层已用 "CH1/CH2/CH3" 字符串，无需改动）

## Decisions

**D1：枚举值命名 `INA_CH1/CH2/CH3`**
选择 `INA_CH1` 而非 `INA_RAIL_CH1`，去掉 `RAIL` 中间词，因为 CH 本身已隐含通道含义，更简洁。

**D2：`app_state` 字段命名 `ch1_ma/ch2_ma/ch3_ma`**
与枚举保持一致的 CH 前缀，getter/setter 同步改为 `get_ch1_a()` / `set_ch1_ma()` 等，保持现有 `_ma`（毫安）单位后缀不变。

**D3：按键宏命名 `KEY_K1/KEY_K2/KEY_K3`**
保留 `KEY_` 前缀（与 `KeyEvent` 枚举的 `KEY_SHORT/KEY_LONG` 命名空间一致），追加 `K1/K2/K3` 物理标识。

**D4：`INA226_ADDR_RAIL` 数组改为 `INA226_ADDR_CH`**
数组下标 0/1/2 对应 CH1/CH2/CH3，与枚举值对齐。

**D5：`ui_events.cpp` 中的按键 ID 参数**
`handle_key(k, ...)` 中 `k` 为数组下标（0/1/2），对应 K1/K2/K3，内部逻辑用 `KEY_K1/K2/K3` 宏比较，无需改变函数签名。

## Risks / Trade-offs

- [风险] 调用点遗漏 → 缓解：用 `rg` 全库搜索旧符号，编译报错兜底
- [风险] `native_main.cpp` 中硬编码的枚举值 → 缓解：同步更新仿真入口
- [Trade-off] 破坏性重命名导致旧分支合并冲突 → 可接受，项目单分支开发
