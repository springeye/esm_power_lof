# web-ota-ui

固件设置页 SYSTEM 页中的"Web 管理"开关，Web 前端页面（固件上传 + WiFi 配置）。

## ADDED Requirements

### Requirement: SYSTEM 设置页开关

系统 SHALL 在设置页面新增第 6 页 `PAGE_SYSTEM`，包含一个"Web 管理"布尔开关。

#### Scenario: 导航到 SYSTEM 页
- **WHEN** 用户在设置页面通过 K1/K3 键翻页至第 6 页
- **THEN** 屏幕显示 SYSTEM 页面标题和"Web 管理"开关项，显示当前状态（ON 或 OFF）

#### Scenario: 切换 Web 管理开关
- **WHEN** 用户在 SYSTEM 页选中"Web 管理"项并按下 K2 进入编辑模式，再按 K1/K3 切换值
- **THEN** 开关状态在 ON 和 OFF 之间切换，并实时调用 WiFi 管理模块启停热点

#### Scenario: 开关状态持久化
- **WHEN** 用户修改"Web 管理"开关值并退出编辑模式
- **THEN** 新值持久化到 NVS，设备重启后保持上次状态

### Requirement: Web 前端页面

系统 SHALL 在访问 AP 热点 IP（192.168.4.1）时提供一个单页 Web 应用，支持固件上传和 WiFi 配置。

#### Scenario: 访问首页
- **WHEN** 用户连接热点后在浏览器打开 `http://192.168.4.1`
- **THEN** 显示包含固件更新区和 WiFi 配置区的完整页面

#### Scenario: 固件更新区 - 选择文件
- **WHEN** 用户点击上传区域或拖拽 .bin 文件到上传区
- **THEN** 页面显示所选文件名和大小

#### Scenario: 固件更新区 - 进度展示
- **WHEN** OTA 刷写正在进行
- **THEN** 页面显示进度条、百分比数字和当前状态文字（如 "正在写入..."）

#### Scenario: WiFi 配置区 - 填写并保存
- **WHEN** 用户输入 SSID 和密码并点击保存按钮
- **THEN** 页面调用 `/api/wifi/config` 保存凭据，并显示保存成功提示

#### Scenario: WiFi 配置区 - 回显已保存凭据
- **WHEN** 页面加载 WiFi 配置区
- **THEN** 自动调用 `/api/wifi/status` 获取已保存信息，SSID 回显到输入框

### Requirement: Web 页面视觉设计

系统 SHALL 提供一个暗色主题、适配移动端的单页 Web 界面。

#### Scenario: 移动端适配
- **WHEN** 用户在手机浏览器访问页面
- **THEN** 页面布局响应式适配，字体和按钮大小适合触屏操作

#### Scenario: 视觉一致性
- **WHEN** 页面在任何浏览器中打开
- **THEN** 显示暗色背景（#0a0e14 色调）、清晰的层次结构、专业简洁的排版
