## 1. 基础设施

- [ ] 1.1 在 `platformio.ini` 中添加 `lib_deps`：`me-no-dev/ESPAsyncWebServer` + `me-no-dev/AsyncTCP`
- [ ] 1.2 在 `include/app_config.h` 中添加编译期常量：`WEB_AP_SSID`、`WEB_AP_PASSWORD`、`WEB_SERVER_PORT`、`OTA_MAX_SIZE`、`TASK_STACK_WEB`
- [ ] 1.3 在 `config_manager.h` 中新增 `WifiConfig` 结构体和 getter/setter 声明
- [ ] 1.4 在 `config_manager.cpp` 中实现 WifiConfig 的 NVS 读写（key `wifi_cfg`）
- [ ] 1.5 在 `platformio.ini` 的 `build_src_filter` 中引入新源文件（`src/wifi/`、`src/ota/`、`src/web/`）
- [ ] 1.6 构建验证：`pio run -e esp32s3` 确认依赖安装成功

## 2. WiFi 管理模块

- [ ] 2.1 创建 `src/wifi/wifi_manager.h`：定义 `wifi_mgr::init()`、`start_ap()`、`stop_ap()`、`start_sta()`、`stop_all()`、状态枚举
- [ ] 2.2 创建 `src/wifi/wifi_manager.cpp`：实现 AP 模式创建（SSID=`FanCtrl-OTA`，密码=`WEB_AP_PASSWORD`）
- [ ] 2.3 实现 STA 模式：从 NVS 读取凭据，连接路由器，连接失败则静默停止
- [ ] 2.4 实现模式切换逻辑：AP ↔ STA ↔ OFF 状态机，支持从 Web 页面和设置开关两处触发
- [ ] 2.5 编译验证

## 3. Web 前端页面

- [ ] 3.1 创建 `ui/web/` 目录，编写单页 `index.html`（暗色主题，移动端适配）
- [ ] 3.2 固件更新区：拖拽上传 + 点击选择 + 进度条 + 状态文字
- [ ] 3.3 WiFi 配置区：SSID 输入 + 密码输入 + 保存按钮 + 已保存状态回显
- [ ] 3.4 将 HTML 文件 Gzip 压缩并转换为 C 数组常量 `html_page_gz[]`，存入 `src/web/html_page.h`
- [ ] 3.5 编译验证 HTML 数组能正确链接

## 4. Web 服务器

- [ ] 4.1 创建 `src/web/web_server.h`：声明 `web_server::start()`、`stop()`、路由注册
- [ ] 4.2 创建 `src/web/web_server.cpp`：初始化 ESPAsyncWebServer，绑定 AP IP（192.168.4.1:80）
- [ ] 4.3 实现 `GET /` 路由：返回 Gzip 压缩的 HTML 页面（Content-Encoding: gzip）
- [ ] 4.4 实现 `POST /api/wifi/config`：接收 JSON `{ssid, password}`，写入 NVS
- [ ] 4.5 实现 `GET /api/wifi/status`：返回已保存的 SSID 和状态（密码掩码为 `***`）
- [ ] 4.6 编译验证

## 5. OTA 逻辑

- [ ] 5.1 创建 `src/ota/ota_handler.h`：声明 `ota_handler::handle_upload()`、`get_progress()`
- [ ] 5.2 创建 `src/ota/ota_handler.cpp`：实现 `POST /update` 路由处理器，校验魔数 0xE9 + 大小限制
- [ ] 5.3 实现逐块写入 `Update.write()`，每块更新 `app_state::ota_progress` 原子变量
- [ ] 5.4 在 `app_state.h/cpp` 中新增 `ota_progress`（0-100）原子变量
- [ ] 5.5 实现刷写完成后 3 秒延时 → `ESP.restart()`
- [ ] 5.6 编译验证

## 6. TFT OTA 进度显示

- [ ] 6.1 在 `src/ui_bridge/data_bridge.cpp` 的定时器中读取 `app_state::ota_progress`
- [ ] 6.2 当进度 > 0 时，在 TFT 上显示覆盖文字 "OTA N%"（使用已有字体）
- [ ] 6.3 刷写完成或失败后清除覆盖文字
- [ ] 6.4 编译验证

## 7. 设置页 SYSTEM 页

- [ ] 7.1 在 `settings_ui.cpp` 中新增 `PAGE_SYSTEM = 5` 枚举值，`PAGE_COUNT` 改为 6
- [ ] 7.2 新增 `SYSTEM_ITEMS[]` 数组，包含"Web 管理"布尔开关项
- [ ] 7.3 新增 `SettingsItemType::BOOL` 类型支持（或复用 UINT8 0/1）
- [ ] 7.4 实现开关切换时调用 `wifi_mgr::start_ap()` / `wifi_mgr::stop_ap()`（通过 `config_manager` setter 触发）
- [ ] 7.5 更新标题栏的页码显示（`1/5` → `1/6`）
- [ ] 7.6 编译验证

## 8. FreeRTOS 任务集成

- [ ] 8.1 在 `tasks.h` 中声明 `web_task()`
- [ ] 8.2 在 `tasks.cpp` 中实现 `web_task()`：初始化 WiFi + 启动 Web 服务器，循环监控开关状态
- [ ] 8.3 在 `tasks.cpp::start_all()` 中创建 `web_task`（Core 0，优先级 2，栈 5120）
- [ ] 8.4 编译验证

## 9. 端到端验证

- [ ] 9.1 编译：`pio run -e esp32s3` 全量编译通过
- [ ] 9.2 静态分析：`pio check -e esp32s3 --skip-packages` 无新增警告
- [ ] 9.3 固件大小检查：确认总固件不超出 OTA 分区（3.25MB）
