## ADDED Requirements

### Requirement: 主仪表盘界面

The system SHALL 提供一个主仪表盘屏幕，集中显示当前温度、风扇占空比/转速、三路电源（电压/电流/功率）、电源状态（Standby/On/Fault）。

#### Scenario: 默认进入仪表盘
- **WHEN** 设备完成启动
- **THEN** 屏幕首先显示主仪表盘，包含温度、风扇、Rail1/Rail2/Rail3、电源状态共 5 个区块

#### Scenario: 数据周期刷新
- **WHEN** 传感器任务更新数据
- **THEN** UI 在不超过 500ms 内反映最新数值

### Requirement: 状态颜色与告警

The system SHALL 根据数值阈值改变指示颜色：温度 < 50°C 绿、50–70°C 黄、≥70°C 红；任一 INA226 过流或电源 Fault 状态显示红色高亮。

#### Scenario: 温度告警颜色
- **WHEN** 温度从 45°C 升到 75°C
- **THEN** 温度数值文字依次变绿→黄→红

#### Scenario: 电源故障高亮
- **WHEN** 电源状态为 Fault
- **THEN** 电源状态区块背景或边框变红，文字显示 "FAULT"

### Requirement: 按键交互提示

The system SHALL 在屏幕底部固定显示当前可用按键的功能提示（如 "K1:Power  K2:Page  K3:Mode"），随上下文变化。

#### Scenario: 提示与功能一致
- **WHEN** 当前页支持 K2 翻页
- **THEN** 底部提示包含 "K2:Page" 字样

### Requirement: 字体与布局规范

The system SHALL 使用 Montserrat 24 显示主数值、Montserrat 16 显示标签与单位，全部为英文/数字，禁止任何中文字符。

#### Scenario: 字体使用
- **WHEN** 截屏检查
- **THEN** 主数值字号大于标签字号，且画面中不出现中文字符
