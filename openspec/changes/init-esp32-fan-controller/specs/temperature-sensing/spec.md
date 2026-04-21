## ADDED Requirements

### Requirement: NTC 温度采样

The system SHALL 通过 ADC1（GPIO36）周期采集 NTC 分压电压，使用 12-bit 分辨率与多次中值平均（≥16 次），并按 β 公式换算为摄氏度。

#### Scenario: 采样周期
- **WHEN** 系统正常运行
- **THEN** NTC 温度数值每 200ms ± 50ms 更新一次

#### Scenario: 多采样降噪
- **WHEN** 采集单次温度
- **THEN** 至少进行 16 次原始 ADC 采样并取中值/均值后再换算

### Requirement: 温度换算与配置

The system SHALL 在 `include/app_config.h` 中以常量形式公开 NTC 的 R25、B 值与上拉电阻，并支持仅修改这些常量即可适配不同热敏元件。

#### Scenario: 默认参数
- **WHEN** 检查 `app_config.h`
- **THEN** 存在 `NTC_R25_OHM`、`NTC_B_VALUE`、`NTC_PULLUP_OHM`，默认值分别为 10000、3950、10000

### Requirement: 温度合理性与故障判定

The system SHALL 在 NTC 短路（温度计算 > 150°C）或开路（温度计算 < -40°C）时，将传感器状态标记为 Fault，UI 显示 "--" 并触发风扇 100% 保护。

#### Scenario: 短路保护
- **WHEN** ADC 读数持续接近 0（NTC 短路）
- **THEN** 温度状态为 Fault，风扇被强制设为 100%

#### Scenario: 开路保护
- **WHEN** ADC 读数持续接近满量程（NTC 开路）
- **THEN** 温度状态为 Fault，UI 显示 "--"，风扇被强制设为 100%
