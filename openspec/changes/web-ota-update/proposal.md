## Why

当前项目为纯离线固件，固件更新只能通过 USB 串口烧录。设备安装后物理访问不便，每次更新需要拆机接线，维护成本高。项目分区表已预留双 OTA 分区（app0/app1），具备 OTA 硬件基础。引入 Web OTA 功能可让用户通过 WiFi 无线更新固件，显著降低维护门槛，同时不影响现有离线运行模式。

## What Changes

- 新增 WiFi 管理模块：AP 热点模式（固定密码）+ STA 客户端模式，SSID/密码通过 Web 页面配置
- 新增 Web 服务器：提供固件上传页面 + SSID/密码配置表单
- 固件设置页新增第 6 页 `PAGE_SYSTEM`，仅包含"Web 管理"开关（开=热点+Web，关=自动连接路由器或关闭WiFi）
- 新增 NVS 持久化：保存 SSID、密码、Web 管理启停状态
- TFT 屏幕同步显示 OTA 刷写进度

## Capabilities

### New Capabilities

- `wifi-management`: WiFi AP/STA 模式管理，SSID/密码配置，热点启停控制
- `web-ota-server`: 嵌入式 Web 服务器，固件上传 API，OTA 刷写流程，进度展示
- `web-ota-ui`: 固件设置页 SYSTEM 页中的"Web 管理"开关；Web 前端页面（固件上传 + WiFi 配置）

### Modified Capabilities

<!-- 无现有 spec 需要修改 -->

## Impact

- **新增源码模块**：`src/wifi/`（WiFi 管理）、`src/ota/`（OTA 逻辑+TFT进度）、`src/web/`（Web 服务器）
- **新增 UI 静态资源**：`ui/web/`（HTML 页面，嵌入 PROGMEM）
- **修改现有文件**：
  - `src/ui_bridge/settings_ui.cpp`：新增第 6 页 `PAGE_SYSTEM`，含"Web 管理"开关
  - `src/app/config_manager.h/cpp`：新增 WiFi 凭据 + Web 管理开关读写接口
  - `include/app_config.h`：新增 OTA/WiFi 编译期常量
  - `src/app/tasks.h/cpp`：新增 `web_task` FreeRTOS 任务
  - `platformio.ini`：新增 `lib_deps`（AsyncTCP、ESPAsyncWebServer）、新源文件引入
- **依赖引入**：Arduino WiFi 库（板级自带）+ ESPAsyncWebServer + AsyncTCP
- **Flash 占用**：新增 Web 前端（~15KB gzip 后）+ 库代码（~80KB .text），需确认不超出 8MB 分区
