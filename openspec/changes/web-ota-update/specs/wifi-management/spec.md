# wifi-management

WiFi AP/STA 模式管理，SSID/密码配置（通过 Web 页面），热点启停控制（通过 SYSTEM 设置页开关），凭据 NVS 持久化。

## ADDED Requirements

### Requirement: Web 管理开关控制热点启停

系统 SHALL 在用户通过 SYSTEM 设置页打开"Web 管理"开关时，启动 AP 热点模式；关闭开关时，停止 AP 模式并根据是否有保存的 WiFi 凭据决定是否切换到 STA 客户端模式。

#### Scenario: 打开 Web 管理开关
- **WHEN** 用户在 SYSTEM 设置页将"Web 管理"开关设为 ON
- **THEN** ESP32 创建 AP 热点（SSID=`FanCtrl-OTA`，密码=`WEB_AP_PASSWORD`，IP=`192.168.4.1`）

#### Scenario: 关闭 Web 管理开关（有已保存凭据）
- **WHEN** 用户在 SYSTEM 设置页将"Web 管理"开关设为 OFF，且 NVS 中存在有效的 WiFi SSID
- **THEN** ESP32 停止 AP 热点，切换到 STA 模式自动连接已配置的路由器

#### Scenario: 关闭 Web 管理开关（无已保存凭据）
- **WHEN** 用户在 SYSTEM 设置页将"Web 管理"开关设为 OFF，且 NVS 中不存在有效的 WiFi SSID
- **THEN** ESP32 停止 AP 热点，彻底关闭 WiFi 模块

### Requirement: Web 页面配置 WiFi 凭据

系统 SHALL 通过 Web 页面接收用户输入的 SSID 和密码，并持久化到 NVS。

#### Scenario: 保存 WiFi 凭据
- **WHEN** 用户在 Web 页面输入 SSID 和密码并点击保存
- **THEN** 系统将凭据保存到 NVS（key `wifi_cfg`），Web 页面显示保存成功

#### Scenario: 回显已保存凭据
- **WHEN** 用户访问 Web 页面的 WiFi 配置区
- **THEN** 系统从 NVS 读取已保存的 SSID 并显示（密码用 `***` 掩码回显）

### Requirement: STA 连接自动重试

系统 SHALL 在 STA 模式下尝试连接路由器，连接失败后静默停止 WiFi 而不影响设备正常运行。

#### Scenario: STA 连接成功
- **WHEN** ESP32 切换到 STA 模式且有有效凭据
- **THEN** ESP32 成功连接路由器并获取 IP 地址

#### Scenario: STA 连接失败
- **WHEN** ESP32 切换到 STA 模式但凭据错误或路由器不可达
- **THEN** ESP32 在超时后静默停止 WiFi，不触发任何故障报警

### Requirement: 热点固定密码

系统 SHALL 使用编译期常量 `WEB_AP_PASSWORD` 作为 AP 热点密码，默认值 `12345678`。

#### Scenario: 连接 AP 热点
- **WHEN** 用户在手机 WiFi 列表中选择 `FanCtrl-OTA` 热点
- **THEN** 用户需输入预设密码 `12345678` 方可连接
