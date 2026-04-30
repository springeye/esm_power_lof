## ADDED Requirements

### Requirement: INA226 通道以 CH1/CH2/CH3 标识
系统 SHALL 使用 `INA_CH1`、`INA_CH2`、`INA_CH3` 枚举值标识三路 XT60 输出口的 INA226 监控通道，其中 CH1 对应 I2C 地址 0x40，CH2 对应 0x41，CH3 对应 0x44。

#### Scenario: 枚举值与 I2C 地址对应
- **WHEN** 代码调用 `ina226_read(INA_CH1, &d)`
- **THEN** 驱动访问 I2C 地址 0x40 的 INA226 芯片

#### Scenario: app_state 以 CH 命名存储电流
- **WHEN** sensorTask 读取 CH2 数据
- **THEN** 结果存入 `app_state::ch2_ma`，可通过 `app_state::get_ch2_a()` 获取安培值

#### Scenario: 配置数组以 CH 命名
- **WHEN** 初始化代码访问 `INA226_ADDR_CH[0]`
- **THEN** 返回值为 0x40（CH1 地址）

### Requirement: 旧符号不再存在
系统 SHALL NOT 在任何源文件中保留 `INA_RAIL_LOAD`、`INA_RAIL_12V`、`INA_RAIL_5V`、`INA_LOAD_ADDR`、`INA_12V_ADDR`、`INA_5V_ADDR`、`INA226_ADDR_RAIL`、`load_ma`（INA 语义）、`v12_ma`、`v5_ma` 等旧命名符号。

#### Scenario: 编译无旧符号引用
- **WHEN** 执行 `pio run -e native` 和 `pio run -e esp32dev`
- **THEN** 编译成功，无未定义符号错误
