## ADDED Requirements

### Requirement: 按键以 K1/K2/K3 物理标识命名
系统 SHALL 使用 `KEY_K1`、`KEY_K2`、`KEY_K3` 宏标识三个物理按键的 GPIO 引脚，与 PCB 丝印对齐，其中 K1=GPIO32，K2=GPIO33，K3=GPIO26。

#### Scenario: 引脚宏与 GPIO 对应
- **WHEN** inputTask 读取 `KEY_K1` 引脚
- **THEN** 访问 GPIO 32

#### Scenario: 按键数组使用新宏
- **WHEN** inputTask 初始化 `key_pins[3]`
- **THEN** 数组内容为 `{KEY_K1, KEY_K2, KEY_K3}`

### Requirement: 旧按键方向宏不再存在
系统 SHALL NOT 在任何源文件中保留 `KEY_UP`、`KEY_ENTER`、`KEY_DOWN` 引脚宏定义及其引用。

#### Scenario: 编译无旧按键宏引用
- **WHEN** 执行 `pio run -e native` 和 `pio run -e esp32dev`
- **THEN** 编译成功，无未定义符号错误
