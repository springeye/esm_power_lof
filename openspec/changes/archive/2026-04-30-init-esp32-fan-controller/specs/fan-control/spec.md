## ADDED Requirements

### Requirement: 25kHz PWM 输出

The system SHALL 在 GPIO25 上输出 25kHz、10-bit 分辨率的 PWM 信号驱动 4 线风扇 PWM 引脚，占空比范围 0–100%。

#### Scenario: 频率与分辨率
- **WHEN** 风扇模块初始化完成
- **THEN** LEDC 通道频率为 25000Hz，分辨率为 10-bit

#### Scenario: 占空比设置
- **WHEN** 调用 `fan::setDuty(50)`
- **THEN** PWM 占空比稳定在 50% ± 1%

### Requirement: 转速反馈测量

The system SHALL 通过 PCNT 模块在 GPIO35 上对 SPEED 信号进行计数，并以每秒一次的周期换算为 RPM（公式：`RPM = pulses/2 * 60`）。

#### Scenario: 正常转速读取
- **WHEN** 风扇以 100% 占空比运行
- **THEN** `fan::getRpm()` 返回的数值与铭牌额定转速误差 ≤ 10%

#### Scenario: 计数复位
- **WHEN** 完成一次 RPM 计算
- **THEN** PCNT 计数器被清零，开始下一秒的计数

### Requirement: 温控闭环

The system SHALL 根据 NTC 温度按以下分段曲线自动调整 PWM 占空比，并带 2°C 回滞：T<30°C 为 20%、30–50°C 线性 20→60%、50–70°C 线性 60→100%、T≥75°C 强制 100%。

#### Scenario: 低温区
- **WHEN** 温度稳定于 25°C
- **THEN** 占空比稳定在 20%

#### Scenario: 高温保护
- **WHEN** 温度上升到 ≥75°C
- **THEN** 占空比强制为 100%，UI 显示高温告警标志

#### Scenario: 回滞防抖
- **WHEN** 温度在 50°C 上下小幅波动 ±1°C
- **THEN** 占空比不发生抖动跳变

### Requirement: 风扇堵转告警

The system SHALL 当 PWM 占空比 >30% 持续 3 秒而 RPM 仍为 0 时，将风扇状态置为 Fault 并通过 UI 告警。

#### Scenario: 堵转检测
- **WHEN** 占空比 50% 持续 3 秒且 RPM 始终为 0
- **THEN** 风扇状态变为 Fault，UI 显示风扇告警
