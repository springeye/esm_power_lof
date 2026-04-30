## ADDED Requirements

### Requirement: I2C 总线与设备地址

The system SHALL 在 GPIO21(SDA)/GPIO22(SCL) 上以 400kHz 速率初始化共享 I2C 总线，并按地址 0x40、0x41、0x44 实例化 3 个 INA226 通道（CH1/CH2/CH3）。

#### Scenario: 设备探测
- **WHEN** 系统启动
- **THEN** 3 个 INA226 设备地址均被探测到并初始化成功，否则对应通道状态为 Fault

#### Scenario: 总线频率
- **WHEN** 检查 Wire 配置
- **THEN** I2C 时钟为 400000Hz

### Requirement: 测量配置

The system SHALL 对每路 INA226 调用 `setMaxCurrentShunt(40, 0.002)` 配置 2mΩ shunt、最大 40A 量程，并启用电压、电流、功率连续测量。

#### Scenario: 量程配置
- **WHEN** INA226 初始化完成
- **THEN** 当前 shunt 阻值寄存为 0.002Ω，最大电流量程为 40A

### Requirement: 错峰轮询采集

The system SHALL 以 250ms 周期错峰轮询 3 路 INA226，分别读取母线电压（V）、电流（A）、功率（W），并将数据通过线程安全方式发布给 UI 与控制任务。

#### Scenario: 数据更新周期
- **WHEN** 系统稳定运行
- **THEN** 每路 INA226 数据每 750ms 内至少更新一次

#### Scenario: 数据合理性
- **WHEN** 任意通道读到电压 < 0V 或 > 36V
- **THEN** 对应通道状态置为 Fault，UI 显示 "--"

### Requirement: I2C 错误恢复

The system SHALL 在 I2C 读写连续失败 ≥ 3 次时，对故障通道执行总线恢复（重新 begin 或时钟脉冲拉爆），并将其状态恢复为正常或保持 Fault。

#### Scenario: 单次错误
- **WHEN** I2C 读写偶发 1 次失败
- **THEN** 系统不告警，下个周期重试
