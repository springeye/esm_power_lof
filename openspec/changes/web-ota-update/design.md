## Context

当前项目是 ESP32-S3 纯离线智能风扇控制器。所有逻辑以 FreeRTOS 任务驱动，LVGL 9.x 渲染 TFT 屏幕，设置页面通过 `settings_ui.cpp` 管理 5 个配置页（FAN/TEMP/DISPLAY/POWER/SENSOR），运行时配置通过 `config_manager` 持久化到 NVS。

分区表已预留双 OTA（app0/app1 + otadata），但项目设计层面明确排除了 WiFi/BLE/OTA。本次变更打破此约束，为目标场景（设备已安装、不便物理烧录）引入无线固件更新能力。

ESP32-S3 内置 2.4GHz WiFi 和蓝牙，无需额外硬件。当前 GPIO 分配已满（见 `pins.h` GPIO 使用汇总），但 WiFi 为片上外设，不占用 GPIO。唯一的限制是 ADC2 在 WiFi 启用时不可用——本项目 NTC 温度传感器使用 ADC1_CH0（GPIO1），不受影响。

## Goals / Non-Goals

**Goals:**
- 在设置页面新增"Web 管理"开关，用户可随时启用/关闭
- 启用后创建 WiFi 热点（AP 模式），提供 Web 页面供固件上传
- Web 页面支持上传 .bin 固件并执行 OTA 刷写，显示进度
- Web 页面提供 SSID / 密码表单，填写后持久化到 NVS
- 关闭 Web 管理时，若用户已配置 WiFi 凭据，自动切换到 STA 客户端模式连接路由器
- Web 前端简洁美观，适配移动端浏览器

**Non-Goals:**
- 不支持蓝牙 OTA
- 不支持通过 STA 模式提供 Web 管理（仅在 AP 热点模式下）
- 不支持 BLE/WiFi 用于传感器数据传输或远程监控
- 不支持 OTA 失败自动回滚（Raft 或类似机制）——依赖 ESP32 Arduino 内置的安全回滚
- 不实现 mDNS 服务发现

## Decisions

### D1: 异步 Web 服务器 → ESPAsyncWebServer

**选择**：使用 `me-no-dev/ESPAsyncWebServer` + `me-no-dev/AsyncTCP`。

**理由**：
- 异步 I/O 不阻塞调用线程，Web 请求在后台处理，不影响 LVGL 渲染（5ms 周期，不能被长时间阻塞导致 WDT 复位）
- 社区成熟度极高，ESP32 Arduino 生态事实标准
- 支持文件上传（multipart/form-data）的异步处理，OTA 场景必需

**替代方案**：
- `WiFiServer`（Arduino 内置）：同步阻塞，每个请求会阻塞 LVGL 任务，导致 WDT 复位 ❌
- `WebServer.h`（ESP32 标准库）：同步，同样有阻塞问题 ❌
- `mongoose`: 功能更强但体积大，Flash 消耗过高 ⚠️

### D2: AP 模式为 Web OTA 服务入口

**选择**：Web 管理启用时强制创建 AP 热点，不同时在 STA 和 AP 之间切换服务端。

**理由**：
- 简化状态管理：AP 模式下 ESP32 作为 DHCP 服务器，客户端可直接通过固定 IP（192.168.4.1）访问
- 安全边界清晰：热点是临时性的，关闭后 WiFi 资源释放或切换至 STA
- 用户工作流明确：启用 → 手机连热点 → 打开网页 → 上传固件 → 关闭

**AP 参数**：
- SSID：`FanCtrl-OTA`
- 密码：固定预设密码 `12345678`（编译期常量 `WEB_AP_PASSWORD`，可在 `app_config.h` 修改）
- IP：`192.168.4.1`
- 子网：`255.255.255.0`

### D3: STA 模式为"关 Web 管理"时的自动行为

**选择**：用户关闭 Web 管理时，自动检查 NVS 中是否有已保存的 SSID 和密码。如果有，切换到 STA 模式尝试连接路由器；如果没有或连接失败，直接关闭 WiFi。无需用户确认。

**状态流转**：
```
OFF ──[启用Web管理]──> AP模式（热点 + Web服务）
AP ──[关闭Web管理，有凭据]──> STA模式（连接路由器）
AP ──[关闭Web管理，无凭据]──> OFF
STA ──[启用Web管理]──> AP模式
```

### D4: 固件设置页集成 — 新增 PAGE_SYSTEM

**选择**：新增第 6 个设置页 `PAGE_SYSTEM`，仅包含一个"Web 管理"布尔开关。

**理由**：
- 用户明确要求新增独立页面，方便未来扩展系统级设置（固件版本信息等）
- "Web 管理"开关控制热点启停，K2 键进入编辑模式切换 ON/OFF
- 无需在固件设置页显示 SSID/密码（在 Web 页面配置）

### D5: NVS 存储方案

**选择**：在 `config_manager` 的 `AppConfig` 中新增 `WifiConfig` 结构体，与现有配置统一管理。

```cpp
struct WifiConfig {
    char ssid[33];        // 最多 32 字节 + null
    char password[65];    // 最多 64 字节 + null
    bool web_mgmt_enabled; // Web 管理开关状态
};
```

NVS key: `"wifi_cfg"`，blob 存储。

**理由**：延续现有 `config_manager` 的设计模式（FanConfig、DisplayConfig 等），避免新增分散的 NVS 读写逻辑。

### D6: FreeRTOS 任务划分

**选择**：新增 `web_task`（Core 0，优先级 2，栈 5120），不污染现有 5 个任务。

**理由**：
- Core 0 已有较多任务（sensor/ctrl/input/power），WiFi + Web 服务不需要高实时性，优先级设为 2（最低）
- 单独任务便于管理 WiFi 生命周期（启停、模式切换），不影响 LVGL 渲染（Core 1 独占）
- 5120 字节栈可容纳 ESPAsyncWebServer + WiFi 协议栈需求

### D7: Web 前端实现

**选择**：单 HTML 文件，内嵌 CSS/JS，Gzip 压缩后以 `PROGMEM` 常量数组形式编译进固件。

**理由**：
- 无需 SPIFFS/LittleFS 文件系统，避免分区表变更
- 页面约 15KB（压缩后 4-5KB），Flash 压力极小（8MB 总容量）
- 简化部署：无外部资源依赖

**功能**：
- 固件上传区：拖拽/点击选择 .bin 文件 + 进度条
- WiFi 配置区：SSID 输入 + 密码输入 + 保存按钮
- 连接状态指示：显示当前热点名称和已连接设备数
- 适配移动端：viewport 设置 + 响应式布局

**样式**：暗色主题，与设备 TFT 屏幕的 `0x0a0e14` 深蓝色调一致，简洁专业。

### D8: OTA 刷写流程（含 TFT 进度）

**选择**：使用 Arduino `Update` 库的标准 API（`Update.begin()`, `Update.write()`, `Update.end()`）。

**流程**：
1. 前端上传 .bin 文件（`POST /update`，`multipart/form-data`）
2. 服务端逐块写入 `Update.write()`
3. 每写入一块同步更新：Web 页面进度条 + TFT 屏幕进度（通过 `app_state` 原子变量）
4. 全部写入后调用 `Update.end()` 验证
5. 返回成功响应，前端显示"即将重启"，3 秒后 `ESP.restart()`

**TFT 进度显示**：在 OTA 刷写期间，TFT 显示覆盖层（简单文字进度，如 "OTA 35%..."），不阻塞渲染。通过 `app_state::set_ota_progress()` 推送进度，`data_bridge` 定时器读取并更新 LVGL 标签。

**安全措施**：
- 校验文件头魔数（0xE9 ESP32 固件标识）
- 校验固件大小不超过分区容量（app0/app1 各 3.25MB）
- ESP32 Arduino 内置安全回滚（`esp_ota_set_boot_partition`）

## Risks / Trade-offs

| 风险 | 缓解措施 |
|------|---------|
| WiFi 启用增加功耗，可能影响散热性能 | 仅在用户主动启用时开启，默认关闭；关闭后彻底释放 WiFi 资源 |
| ESPAsyncWebServer 库增加 Flash 占用（~80KB .text） | 验证总固件大小未超出分区限制；如必要可开启 `-Os` 优化 |
| ADC2 不可用（WiFi 占用） | NTC 使用 ADC1_CH0（GPIO1），不受影响 ✅ |
| 用户配置错误 SSID/密码导致 STA 连接失败 | 失败后静默关闭 WiFi，不阻塞设备正常运行；LED 状态指示或 TFT 提示 |
| Web 热点为固定密码，密码硬编码在固件中 | 密码编译期常量，用户可通过 `app_config.h` 修改预设值 |
| ESPAsyncWebServer 在 Arduino 3.x 兼容性 | pin to me-no-dev 仓库的特定 commit 或 fork；PlatformIO 中可指定版本 |

## Open Questions

1. **STA 连接是否影响原有任务调度？** 需实测 WiFi 协议栈对 Core 0 其他任务的影响。
2. **固定密码是否需要通过 Web 页面修改？** 当前仅通过 `app_config.h` 编译期修改，后续可加入 Web 页面配置功能。
