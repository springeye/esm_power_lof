## ADDED Requirements

### Requirement: Home 屏幕顶部状态栏布局

Home 屏幕 SHALL 在顶部显示一行状态栏，包含：
- 左侧：状态指示点（Unicode `◉` 字符）和状态文本（绑定 `system_state`）
- 右侧：运行时间（绑定 `uptime`）和温度（绑定 `device_temp`）

#### Scenario: 运行中状态显示
- **WHEN** PS_ON 打开，设备处于运行状态
- **THEN** 状态指示点显示绿色，状态文本显示"运行中"，温度和运行时间正常显示

#### Scenario: 停止状态显示
- **WHEN** PS_ON 关闭，设备处于停止状态
- **THEN** 状态指示点显示红色（静态不闪烁），状态文本显示"停止"

### Requirement: 功率主区显示当前功率和设计功率

功率主区 SHALL 居中显示：
- 顶行："当前功率" 标签 + 设计功率徽标（`⚡` + 额定功率值）
- 主行：当前功率大字号数字（绑定 `device_current_power`），使用 `hos_bold_big` 字体

#### Scenario: 运行中功率显示
- **WHEN** 设备运行中且功率 > 0
- **THEN** 功率数字以大字号居中显示当前功率值（如 "65.0W"）

#### Scenario: 停止状态功率显示
- **WHEN** 设备停止，PS_ON 关闭
- **THEN** 功率数字显示 "0.0W"

### Requirement: 功率占比进度条

Home 屏幕 SHALL 在功率主区下方显示功率占比进度条区域：
- 左侧标签"功率占比"
- 右侧百分比数值（绑定 `device_power_percent_txt`）
- 下方 `lv_bar` 进度条（绑定 `device_power_percent`）

#### Scenario: 进度条显示功率百分比
- **WHEN** 设备运行中，功率为设计功率的 54%
- **THEN** 进度条填充至 54%，右侧显示"54%"

### Requirement: 三列统计行

统计行 SHALL 显示三列等宽数据：
- 运行时间（绑定 `uptime`）
- 总电能（绑定 `wh`）
- 效率百分比（绑定 `device_power_percent_txt`，后续可能需要专用绑定）

#### Scenario: 运行中统计数据
- **WHEN** 设备运行中
- **THEN** 三列分别显示运行时间（如"02:34h"）、总电能（如"156Wh"）、效率（如"91%"）

#### Scenario: 停止状态统计数据
- **WHEN** 设备停止
- **THEN** 运行时间和效率列显示"--"，总电能列显示累计值

### Requirement: 三列输出通道卡片

通道区域 SHALL 使用 flex row 布局显示三个等宽通道卡片（CH1/CH2/CH3），每列包含：
- 顶部标签头：通道号（CH1/CH2/CH3），带对应颜色底边框（CH1 绿、CH2 橙、CH3 蓝）
- 电压行：左标签"电压"，右值（绑定 `ch1_voltage` / `ch2_voltage` / `ch3_voltage`）
- 电流行：左标签"电流"，右值（绑定 `ch1_current` / `ch2_current` / `ch3_current`）
- 功率大字：绑定 `ch1_pwer` / `ch2_pwer` / `ch3_pwer`

通道区域 SHALL 使用 `style_flex_grow="1"` 填满剩余屏幕高度。

#### Scenario: 通道数据正常显示
- **WHEN** 设备运行中
- **THEN** CH1 显示 12.0V / 2.50A / 30.0W，CH2 显示 5.00V / 4.00A / 20.0W，CH3 显示 3.30V / 4.55A / 15.0W

#### Scenario: 通道数据停止状态
- **WHEN** 设备停止（PS_ON 关闭）
- **THEN** 所有通道数据归零（0.00V / 0.00A / 0.0W）

### Requirement: CH3 绑定修复

CH3 通道的电压、电流、功率绑定 SHALL 使用 `ch3_voltage`、`ch3_current`、`ch3_pwer`，而非现有的 `ch2_*` 错误绑定。

#### Scenario: CH3 数据独立性
- **WHEN** CH3 通道读取数据
- **THEN** 绑定 `ch3_voltage`、`ch3_current`、`ch3_pwer` 独立于 CH2 通道数据