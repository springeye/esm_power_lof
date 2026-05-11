# web-ota-server

嵌入式 Web 服务器，固件上传 API，OTA 刷写流程，进度推送（Web + TFT 双通道）。

## ADDED Requirements

### Requirement: 固件上传 API

系统 SHALL 提供 `POST /update` 端点，接收 multipart/form-data 格式的 .bin 固件文件并执行 OTA 刷写。

#### Scenario: 成功上传并刷写固件
- **WHEN** 用户通过 Web 页面上传一个有效的 .bin 固件文件
- **THEN** 系统校验文件头（0xE9 魔数），通过后逐块写入并返回 HTTP 200 + 成功消息

#### Scenario: 上传无效文件
- **WHEN** 用户上传非固件文件（魔数校验失败）
- **THEN** 系统返回 HTTP 400 + 错误消息，不执行刷写

#### Scenario: 固件大小超限
- **WHEN** 用户上传的固件大小超过单 OTA 分区容量（3.25MB）
- **THEN** 系统返回 HTTP 413 + 错误消息，不执行刷写

### Requirement: OTA 进度推送

系统 SHALL 在 OTA 刷写过程中实时推送进度百分比，供 Web 页面和 TFT 屏幕同步展示。

#### Scenario: Web 页面显示进度
- **WHEN** OTA 刷写正在进行中，已写入 N%
- **THEN** Web 页面进度条显示 N%，并更新状态文字

#### Scenario: TFT 屏幕显示进度
- **WHEN** OTA 刷写正在进行中，已写入 N%
- **THEN** TFT 屏幕显示覆盖文字 "OTA N%"，持续刷新直至刷写完成

### Requirement: 刷写完成自动重启

系统 SHALL 在 OTA 刷写成功且获得前端确认后，延时 3 秒自动重启进入新固件。

#### Scenario: 正常重启
- **WHEN** `Update.end()` 返回 true 且前端确认消息已送达
- **THEN** 系统等待 3 秒后调用 `ESP.restart()`

#### Scenario: 刷写失败不重启
- **WHEN** `Update.end()` 返回 false 或写入过程中发生错误
- **THEN** 系统返回错误消息，不执行重启

### Requirement: WiFi 凭据 API

系统 SHALL 提供 REST API 供 Web 页面读写 WiFi 凭据。

#### Scenario: 保存凭据
- **WHEN** Web 页面发送 `POST /api/wifi/config`，body 为 `{"ssid":"MyWiFi","password":"mypass"}`
- **THEN** 系统保存到 NVS，返回 `{"ok":true}`

#### Scenario: 读取凭据状态
- **WHEN** Web 页面发送 `GET /api/wifi/status`
- **THEN** 系统返回 `{"ssid":"MyWiFi","password":"***","saved":true}` 或 `{"saved":false}`
