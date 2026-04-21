## ADDED Requirements

### Requirement: 按键硬件配置

The system SHALL 将 K1=GPIO32、K2=GPIO33、K3=GPIO26 配置为 `INPUT_PULLUP`，按下时为低电平。

#### Scenario: 默认状态
- **WHEN** 按键未按下
- **THEN** 对应 GPIO 读数为 1（高）

### Requirement: 软件去抖

The system SHALL 对每个按键以 5ms 周期采样并执行至少 3 次连续相同电平判定，过滤抖动后输出稳定状态。

#### Scenario: 抖动过滤
- **WHEN** 按键产生 < 10ms 的抖动脉冲
- **THEN** 不会产生按键事件

### Requirement: 短按与长按事件

The system SHALL 区分短按（按下到松开 < 800ms）与长按（按下持续 ≥ 800ms 触发一次），并通过事件队列分发给上层应用。

#### Scenario: 短按 K1
- **WHEN** K1 被按下并在 500ms 内松开
- **THEN** 产生 `Key1ShortPress` 事件

#### Scenario: 长按 K1
- **WHEN** K1 被按下并保持 1 秒
- **THEN** 在 800ms 处产生一次 `Key1LongPress` 事件，松开时不再产生短按事件
