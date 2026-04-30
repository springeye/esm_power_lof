## ADDED Requirements

### Requirement: PSON 输出控制

The system SHALL 通过 GPIO27 以开漏方式输出 PSON 信号（外部上拉到 +5VSB，低有效），上电默认高电平（电源关闭）。

#### Scenario: 上电默认
- **WHEN** ESP32 复位完成
- **THEN** GPIO27 处于高阻或高电平，电源处于关闭状态

#### Scenario: 开机指令
- **WHEN** 上层调用 `power::turnOn()`
- **THEN** GPIO27 拉低，电源进入开启时序

### Requirement: PWOK 输入监测

The system SHALL 通过 GPIO34（仅输入）经电平转换后采样 PWOK 信号，并以 ≥ 100Hz 频率轮询。

#### Scenario: 上电检测
- **WHEN** PSON 拉低 1 秒内 PWOK 升高
- **THEN** 电源状态变更为 On

#### Scenario: 上电超时
- **WHEN** PSON 拉低 1 秒后 PWOK 仍为低
- **THEN** 电源状态变更为 Fault，PSON 自动恢复高电平

### Requirement: 上电时序状态机

The system SHALL 维护 `Off → Standby → Starting → On → Stopping → Off` 的电源状态机，并支持 `Fault` 异常分支；状态变化通过 UI 实时反映。

#### Scenario: 正常开机
- **WHEN** 用户在 Standby 状态下短按 K1
- **THEN** 进入 Starting，PSON 拉低，等待 PWOK；PWOK 高后进入 On

#### Scenario: 正常关机
- **WHEN** 用户在 On 状态下长按 K1 ≥ 2 秒
- **THEN** 进入 Stopping，PSON 拉高，等待 PWOK 失稳后进入 Off

#### Scenario: 运行中故障
- **WHEN** 处于 On 状态时 PWOK 持续低于 100ms
- **THEN** 进入 Fault，PSON 自动拉高并通过 UI 告警

### Requirement: 故障保护

The system SHALL 在以下任一条件成立时强制关闭电源（PSON 拉高）：温度 ≥ 80°C、风扇 Fault 持续 5s、任一 INA226 通道电流超量程。

#### Scenario: 过温保护
- **WHEN** 温度 ≥ 80°C
- **THEN** 电源被强制关闭，UI 显示过温保护原因
