# ESP32 OTA功能开发计划

## TL;DR

> **Quick Summary**: 在ESM Power System固件中新增完整的OTA无线更新功能，包括WiFi AP热点模式、Web管理界面、固件上传更新和WiFi凭据配置。
>
> **Deliverables**:
> - WiFi管理模块（STA/AP模式切换）
> - ESPAsyncWebServer HTTP服务
> - OTA固件上传处理
> - Web配置页面（WiFi配置+固件上传）
> - 设置页面OTA模式开关
> - NVS扩展存储WiFi凭据
>
> **Estimated Effort**: Large
> **Parallel Execution**: YES - 4 waves
> **Critical Path**: platformio.ini → config_manager → wifi_manager → web_server → settings_ui集成

---

## Context

### Original Request
用户要求在ESP32电源管理系统中增加OTA功能：
1. 设置中增加OTA选项开关
2. 开启OTA时设备变成AP热点（固定IP）
3. Web页面可配置WiFi SSID/密码
4. Web页面可上传固件进行OTA更新
5. 关闭OTA后自动连接配置的WiFi

### Interview Summary
**Key Discussions**:
- Web服务器库：选择ESPAsyncWebServer（异步非阻塞，不影响LVGL渲染）
- Web认证：无需认证（开发阶段简化）
- 热点配置：ESM-Power-{MAC后4位} / 随机8位密码（显示在屏幕上）

**Research Findings**:
- 项目当前无任何网络代码，需从零构建
- NVS仅存运行时配置，需扩展WiFi凭据存储
- 分区表已支持OTA（app0/app1双分区）
- FreeRTOS LVGL任务有5s看门狗超时，Web请求不能阻塞

### Metis Review
**Identified Gaps** (addressed):
- 必须使用ESP32Async/ESPAsyncWebServer @ 3.6.0（非已弃用的me-no-dev版本）
- OTA写入时必须调用esp_task_wdt_reset()和vTaskDelay防止看门狗
- 必须在setup()中调用esp_ota_mark_app_valid_cancel_rollback()防止自动回滚
- Web服务器必须运行在Core 0（非LVGL的Core 1）
- 必须验证固件magic byte（0xE9）
- OTA成功后先发送HTTP 200响应再重启
- 必须在TFT屏幕上显示AP密码

---

## Work Objectives

### Core Objective
从零构建ESP32 WiFi/AP热点管理和Web OTA更新子系统，实现无线固件升级和WiFi配置功能。

### Concrete Deliverables
1. `platformio.ini` - 添加ESPAsyncWebServer和AsyncTCP依赖
2. `include/app_config.h` - 新增网络和OTA相关常量
3. `src/app/config_manager.{h,cpp}` - 扩展NVS存储WiFi/OTA配置
4. `src/net/wifi_manager.{h,cpp}` - WiFi STA/AP模式管理（新建）
5. `src/net/web_server.{h,cpp}` - ESPAsyncWebServer HTTP服务（新建）
6. `src/net/ota_handler.{h,cpp}` - OTA固件上传处理（新建）
7. `src/ui_bridge/settings_ui.{h,cpp}` - 新增OTA模式开关
8. `src/app/tasks.cpp` - 新增网络任务
9. `src/main.cpp` - 初始化网络子系统和OTA回滚保护
10. Web页面HTML - WiFi配置和固件上传界面

### Definition of Done
- [ ] 编译通过：`pio run -e esp32s3`
- [ ] OTA模式开启后设备创建AP热点
- [ ] Web页面可通过192.168.4.1访问
- [ ] WiFi配置可保存到NVS
- [ ] 固件上传可成功更新设备
- [ ] 关闭OTA后设备自动连接配置的WiFi

### Must Have
- ESPAsyncWebServer异步HTTP服务
- AP热点模式（ESM-Power-{MAC后4位}）
- Web页面：WiFi配置 + 固件上传
- NVS持久化WiFi凭据
- OTA回滚保护机制
- 看门狗喂狗防止超时
- 设置页面OTA模式开关

### Must NOT Have (Guardrails)
- 不使用已弃用的me-no-dev/ESPAsyncWebServer
- OTA写入不能在LVGL任务中执行
- 不跳过Update.abort()错误清理
- 不允许超过分区大小的固件上传（>3.25MB）
- 不在全局宏中定义TFT_BL

---

## Verification Strategy (MANDATORY)

> **ZERO HUMAN INTERVENTION** - ALL verification is agent-executed. No exceptions.

### Test Decision
- **Infrastructure exists**: YES (Unity框架)
- **Automated tests**: Tests-after（先实现后测试）
- **Framework**: Unity
- **Agent-Executed QA**: 所有任务必须包含QA场景

### QA Policy
每个任务必须包含agent-executed QA场景：
- **Web/API**: 使用Bash (curl) - 发送HTTP请求，解析响应，断言状态码和内容
- **WiFi/AP**: 使用Bash (iwlist/iwconfig) - 扫描WiFi网络，验证AP存在
- **OTA**: 使用Bash (curl) - 上传固件文件，验证更新成功
- **NVS**: 使用Bash (串口命令) - 读取NVS值，验证存储正确

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Start Immediately - 基础设施):
├── Task 1: platformio.ini添加库依赖 [quick]
├── Task 2: app_config.h新增网络常量 [quick]
├── Task 3: config_manager扩展NVS键 [quick]
└── Task 4: 创建src/net/目录结构 [quick]

Wave 2 (After Wave 1 - 核心模块):
├── Task 5: wifi_manager STA/AP模式管理 [deep]
├── Task 6: web_server HTTP服务框架 [deep]
├── Task 7: OTA handler固件上传处理 [deep]
└── Task 8: Web页面HTML模板 [writing]

Wave 3 (After Wave 2 - 集成):
├── Task 9: settings_ui新增OTA开关 [visual-engineering]
├── Task 10: tasks.cpp新增网络任务 [deep]
├── Task 11: main.cpp初始化网络和OTA回滚 [deep]
└── Task 12: data_bridge显示AP密码 [visual-engineering]

Wave 4 (After Wave 3 - 测试验证):
├── Task 13: 静态分析和编译验证 [quick]
├── Task 14: WiFi AP模式功能测试 [unspecified-high]
├── Task 15: Web页面功能测试 [unspecified-high]
└── Task 16: OTA固件更新测试 [unspecified-high]

Final Verification Wave:
├── F1: Plan Compliance Audit (oracle)
├── F2: Code Quality Review (unspecified-high)
├── F3: Real Manual QA (unspecified-high)
└── F4: Scope Fidelity Check (deep)
```

### Dependency Matrix
- **1-4**: None → 5-8 (Wave 2)
- **5**: 1,2,3 → 9,10,11,12 (Wave 3)
- **6**: 1,5 → 9,10,11 (Wave 3)
- **7**: 1,6 → 9,10,11 (Wave 3)
- **8**: 6 → 9,12 (Wave 3)
- **9-12**: 5,6,7,8 → 13-16 (Wave 4)
- **13-16**: 9-12 → F1-F4 (Final)

### Agent Dispatch Summary
- **Wave 1**: 4 tasks × quick
- **Wave 2**: 3 × deep + 1 × writing
- **Wave 3**: 2 × deep + 2 × visual-engineering
- **Wave 4**: 1 × quick + 3 × unspecified-high
- **Final**: 4 parallel reviews

---

## TODOs

> Implementation + Test = ONE Task. Never separate.
> EVERY task MUST have: Recommended Agent Profile + Parallelization info + QA Scenarios.
> **A task WITHOUT QA Scenarios is INCOMPLETE. No exceptions.**

- [x] 1. platformio.ini添加网络库依赖

  **What to do**:
  - 在 `[env:esp32s3]` 的 `lib_deps` 中添加：
    - `ESP32Async/ESPAsyncWebServer @ ^3.6.0`
    - `ESP32Async/AsyncTCP @ ^3.3.2`
  - 在 `build_flags` 中添加 OTA 相关宏定义

  **Must NOT do**:
  - 不使用已弃用的 `me-no-dev/ESPAsyncWebServer`
  - 不修改其他环境的配置

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单文件修改，配置项添加

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 2, 3, 4)
  - **Blocks**: Tasks 5, 6, 7
  - **Blocked By**: None

  **References**:
  - `platformio.ini` - 当前lib_deps结构（第22-26行）
  - ESPAsyncWebServer官方文档 - 库版本和依赖说明

  **Acceptance Criteria**:
  - [ ] platformio.ini lib_deps包含ESPAsyncWebServer和AsyncTCP
  - [ ] `pio run -e esp32s3` 依赖解析成功

  **QA Scenarios**:

  ```
  Scenario: 依赖库下载验证
    Tool: Bash (pio)
    Preconditions: platformio.ini已修改
    Steps:
      1. 运行 pio pkg install -e esp32s3
      2. 检查 .pio/libdeps/esp32s3/ 目录
    Expected Result: ESPAsyncWebServer和AsyncTCP库已下载
    Failure Indicators: 依赖下载失败或库不存在
    Evidence: .sisyphus/evidence/task-01-deps-check.txt

  Scenario: 编译依赖验证
    Tool: Bash (pio)
    Preconditions: 依赖已安装
    Steps:
      1. 运行 pio run -e esp32s3 -t clean
      2. 运行 pio run -e esp32s3
    Expected Result: 编译成功，无缺失依赖错误
    Failure Indicators: 编译报错 "No such file or directory" for ESPAsyncWebServer
    Evidence: .sisyphus/evidence/task-01-build.log
  ```

  **Commit**: YES (with Wave 1)
  - Message: `build(net): 添加ESPAsyncWebServer和AsyncTCP依赖`
  - Files: `platformio.ini`

---

- [x] 2. app_config.h新增网络和OTA常量

  **What to do**:
  - 新增WiFi配置常量：
    - `WIFI_CONNECT_TIMEOUT_MS = 10000` - WiFi连接超时
    - `WIFI_RECONNECT_INTERVAL_MS = 30000` - 重连间隔
    - `WIFI_AP_CHANNEL = 1` - AP热点信道
    - `WIFI_AP_MAX_CONN = 4` - AP最大连接数
  - 新增Web服务器常量：
    - `WEB_SERVER_PORT = 80` - HTTP端口
    - `WEB_UPLOAD_MAX_SIZE = 0x340000` - 最大固件上传大小（3.25MB）
  - 新增OTA常量：
    - `OTA_REBOOT_DELAY_MS = 500` - OTA成功后重启延时
    - `OTA_WATCHDOG_FEED_INTERVAL = 1` - 看门狗喂狗间隔(ms)

  **Must NOT do**:
  - 不修改现有常量定义
  - 不添加与项目无关的网络常量

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单文件追加常量定义

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 3, 4)
  - **Blocks**: Tasks 5, 6, 7
  - **Blocked By**: None

  **References**:
  - `include/app_config.h` - 现有常量定义风格（第1-63行）
  - ESP32分区表 - app分区大小限制

  **Acceptance Criteria**:
  - [ ] app_config.h包含所有网络/OTA相关常量
  - [ ] 常量命名符合项目UPPER_SNAKE规范

  **QA Scenarios**:

  ```
  Scenario: 常量定义验证
    Tool: Bash (grep)
    Preconditions: app_config.h已修改
    Steps:
      1. grep "WIFI_" include/app_config.h
      2. grep "WEB_" include/app_config.h
      3. grep "OTA_" include/app_config.h
    Expected Result: 所有新增常量可查找到
    Failure Indicators: 常量未定义
    Evidence: .sisyphus/evidence/task-02-constants.txt

  Scenario: 编译宏验证
    Tool: Bash (pio)
    Preconditions: 常量已定义
    Steps:
      1. 运行 pio run -e esp32s3
    Expected Result: 编译成功，无未定义标识符错误
    Failure Indicators: 编译报错 "WIFI_* was not declared"
    Evidence: .sisyphus/evidence/task-02-build.log
  ```

  **Commit**: YES (with Wave 1)
  - Message: `feat(config): 添加网络和OTA相关常量定义`
  - Files: `include/app_config.h`

---

- [x] 3. config_manager扩展NVS存储WiFi/OTA配置

  **What to do**:
  - 在 `config_manager.h` 的 `AppConfig` 结构体中添加：
    - `bool ota_mode_enabled` - OTA模式开关
    - `char wifi_ssid[33]` - WiFi SSID（最大32字符）
    - `char wifi_password[65]` - WiFi密码（最大64字符）
  - 在 `config_manager.cpp` 中添加NVS键：
    - `"cfg_ota_enabled"` - OTA模式开关
    - `"cfg_wifi_ssid"` - WiFi SSID
    - `"cfg_wifi_pass"` - WiFi密码
  - 实现 `save_ota_config()` 和 `load_ota_config()` 函数
  - 默认值：`ota_mode_enabled = false`, `wifi_ssid = ""`, `wifi_password = ""`

  **Must NOT do**:
  - 不修改现有NVS键的名称和类型
  - 不破坏现有配置加载逻辑
  - 不以明文形式存储敏感信息（当前阶段简化）

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 扩展现有模块，遵循既有模式

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 2, 4)
  - **Blocks**: Tasks 5, 9
  - **Blocked By**: None

  **References**:
  - `src/app/config_manager.h` - AppConfig结构体定义
  - `src/app/config_manager.cpp` - NVS读写模式（Preferences::putString/getString）

  **Acceptance Criteria**:
  - [ ] AppConfig包含ota_mode_enabled、wifi_ssid、wifi_password字段
  - [ ] NVS键cfg_ota_enabled、cfg_wifi_ssid、cfg_wifi_pass已定义
  - [ ] save_ota_config()和load_ota_config()已实现

  **QA Scenarios**:

  ```
  Scenario: NVS读写验证
    Tool: Bash (串口命令)
    Preconditions: 固件已烧录
    Steps:
      1. 通过串口发送设置命令写入WiFi配置
      2. 重启设备
      3. 读取NVS验证配置保持
    Expected Result: WiFi SSID和密码可正确保存和读取
    Failure Indicators: NVS读取失败或返回空值
    Evidence: .sisyphus/evidence/task-03-nvs-rw.txt

  Scenario: 默认值验证
    Tool: Bash (串口命令)
    Preconditions: 清除NVS后首次启动
    Steps:
      1. 运行 pio run -e esp32s3 -t upload
      2. 监视串口输出
    Expected Result: ota_mode_enabled默认为false，wifi_ssid为空
    Failure Indicators: 默认值不正确
    Evidence: .sisyphus/evidence/task-03-defaults.txt
  ```

  **Commit**: YES (with Wave 1)
  - Message: `feat(config): 扩展NVS支持WiFi和OTA配置存储`
  - Files: `src/app/config_manager.h`, `src/app/config_manager.cpp`

---

- [x] 4. 创建src/net/目录和模块骨架

  **What to do**:
  - 创建 `src/net/` 目录
  - 创建骨架文件：
    - `src/net/wifi_manager.h` - WiFi管理接口声明
    - `src/net/wifi_manager.cpp` - WiFi管理实现骨架
    - `src/net/web_server.h` - Web服务器接口声明
    - `src/net/web_server.cpp` - Web服务器实现骨架
    - `src/net/ota_handler.h` - OTA处理接口声明
    - `src/net/ota_handler.cpp` - OTA处理实现骨架
  - 每个头文件包含基本的include guard和namespace声明
  - 每个cpp文件包含对应的include和空函数骨架

  **Must NOT do**:
  - 不实现具体功能逻辑（留待后续任务）
  - 不创建不需要的文件

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 创建文件骨架，无复杂逻辑

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 2, 3)
  - **Blocks**: Tasks 5, 6, 7
  - **Blocked By**: None

  **References**:
  - `src/app/` 目录结构 - 模块组织方式
  - `src/sensors/ntc/ntc.h` - 头文件风格示例

  **Acceptance Criteria**:
  - [ ] src/net/目录已创建
  - [ ] 6个骨架文件已创建（3个.h + 3个.cpp）
  - [ ] 文件包含基本的include和namespace

  **QA Scenarios**:

  ```
  Scenario: 目录结构验证
    Tool: Bash (ls)
    Preconditions: 无
    Steps:
      1. ls -la src/net/
    Expected Result: 显示6个文件
    Failure Indicators: 目录不存在或文件缺失
    Evidence: .sisyphus/evidence/task-04-structure.txt

  Scenario: 编译包含验证
    Tool: Bash (pio)
    Preconditions: 骨架文件已创建
    Steps:
      1. 在main.cpp中include骨架头文件
      2. 运行 pio run -e esp32s3
    Expected Result: 编译成功
    Failure Indicators: 头文件找不到或语法错误
    Evidence: .sisyphus/evidence/task-04-build.log
  ```

  **Commit**: YES (with Wave 1)
  - Message: `feat(net): 创建网络模块目录和文件骨架`
  - Files: `src/net/wifi_manager.h`, `src/net/wifi_manager.cpp`, `src/net/web_server.h`, `src/net/web_server.cpp`, `src/net/ota_handler.h`, `src/net/ota_handler.cpp`

---

- [x] 5. wifi_manager实现STA/AP模式管理

  **What to do**:
  - 实现 `wifi_manager_init()` - 初始化WiFi子系统
  - 实现 `wifi_manager_start_ap()` - 启动AP热点模式
    - SSID格式：`ESM-Power-{MAC后4位}`
    - 密码：随机8位字母数字组合
    - 固定IP：192.168.4.1
    - Gateway：192.168.4.1
    - Subnet：255.255.255.0
  - 实现 `wifi_manager_connect_sta()` - 连接到配置的WiFi
    - 从config_manager读取SSID和密码
    - 超时处理（WIFI_CONNECT_TIMEOUT_MS）
    - 连接失败自动重试
  - 实现 `wifi_manager_stop()` - 停止WiFi
  - 实现 `wifi_manager_get_ap_password()` - 获取AP密码（用于显示）
  - 实现 `wifi_manager_get_status()` - 获取当前连接状态

  **Must NOT do**:
  - 不在LVGL任务中执行WiFi操作
  - 不硬编码WiFi凭据
  - 不忽略连接超时处理

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 核心网络功能，需要WiFi API集成和状态管理

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 2 (with Tasks 6, 7, 8)
  - **Blocks**: Tasks 9, 10, 11, 12
  - **Blocked By**: Tasks 1, 2, 3, 4

  **References**:
  - Arduino WiFi.h API - WiFi.mode(), WiFi.softAP(), WiFi.begin()
  - ESP32 WiFi文档 - AP模式配置和IP设置
  - `src/app/config_manager.h` - 读取WiFi配置的接口

  **Acceptance Criteria**:
  - [ ] wifi_manager_init()可正确初始化
  - [ ] wifi_manager_start_ap()可创建AP热点
  - [ ] wifi_manager_connect_sta()可连接到WiFi
  - [ ] AP密码随机生成且可获取
  - [ ] WiFi状态可正确查询

  **QA Scenarios**:

  ```
  Scenario: AP热点创建验证
    Tool: Bash (iwlist扫描)
    Preconditions: OTA模式已启用
    Steps:
      1. 调用wifi_manager_start_ap()
      2. 使用iwlist scan搜索WiFi网络
      3. 验证ESM-Power-*热点存在
    Expected Result: 热点可见且信号强度正常
    Failure Indicators: 热点未创建或不可见
    Evidence: .sisyphus/evidence/task-05-ap-scan.txt

  Scenario: STA连接验证
    Tool: Bash (ping)
    Preconditions: WiFi凭据已配置
    Steps:
      1. 调用wifi_manager_connect_sta()
      2. 等待连接完成
      3. ping网关验证连接
    Expected Result: ping成功，延迟<100ms
    Failure Indicators: 连接超时或ping失败
    Evidence: .sisyphus/evidence/task-05-sta-connect.txt

  Scenario: AP密码随机性验证
    Tool: Bash (串口读取)
    Preconditions: 多次启动AP模式
    Steps:
      1. 启动AP，记录密码
      2. 重启设备，再次启动AP
      3. 比较两次密码
    Expected Result: 密码不同（随机生成）
    Failure Indicators: 密码相同
    Evidence: .sisyphus/evidence/task-05-password-random.txt
  ```

  **Commit**: NO (等待Wave 2完成一起提交)

---

- [x] 6. web_server实现HTTP服务框架

  **What to do**:
  - 实现 `web_server_init()` - 初始化ESPAsyncWebServer
  - 实现 `web_server_start()` - 启动HTTP服务（端口80）
  - 实现 `web_server_stop()` - 停止HTTP服务
  - 注册路由：
    - `GET /` - 主页面（WiFi配置和固件上传）
    - `POST /wifi` - 保存WiFi配置
    - `POST /update` - OTA固件上传
    - `GET /status` - 当前状态JSON
  - 实现请求处理函数
  - 错误处理和响应

  **Must NOT do**:
  - 不在LVGL任务中处理请求
  - 不跳过请求验证
  - 不返回敏感信息

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: HTTP服务器核心实现，需要ESPAsyncWebServer API集成

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 5, 7, 8)
  - **Blocks**: Tasks 9, 10, 11
  - **Blocked By**: Tasks 1, 4

  **References**:
  - ESPAsyncWebServer官方文档 - 路由注册和请求处理
  - ESP32Async/ESPAsyncWebServer @ 3.6.0 API

  **Acceptance Criteria**:
  - [ ] web_server_init()正确初始化服务器
  - [ ] web_server_start()成功启动在端口80
  - [ ] 所有路由可正确注册
  - [ ] GET /status返回有效JSON

  **QA Scenarios**:

  ```
  Scenario: Web服务器启动验证
    Tool: Bash (curl)
    Preconditions: AP模式已启动
    Steps:
      1. 调用web_server_start()
      2. curl http://192.168.4.1/status
    Expected Result: 返回200状态码和JSON响应
    Failure Indicators: 连接拒绝或超时
    Evidence: .sisyphus/evidence/task-06-server-start.txt

  Scenario: 路由注册验证
    Tool: Bash (curl)
    Preconditions: 服务器已启动
    Steps:
      1. curl -I http://192.168.4.1/
      2. curl http://192.168.4.1/status
    Expected Result: 所有路由返回正确响应
    Failure Indicators: 404 Not Found
    Evidence: .sisyphus/evidence/task-06-routes.txt
  ```

  **Commit**: NO (等待Wave 2完成一起提交)

---

- [x] 7. OTA handler实现固件上传处理

  **What to do**:
  - 实现 `ota_handler_init()` - 初始化OTA处理
  - 实现 `ota_handler_setup_routes()` - 注册OTA路由到web_server
  - 实现固件上传处理：
    - 验证文件大小（不超过WEB_UPLOAD_MAX_SIZE）
    - 验证固件magic byte（0xE9）
    - 分块写入时调用 `esp_task_wdt_reset()` 和 `vTaskDelay(1ms)`
    - 写入失败调用 `Update.abort()`
  - 实现 `ota_handler_validate_firmware()` - 验证固件完整性
  - 实现 `ota_handler_reboot()` - OTA成功后延迟重启

  **Must NOT do**:
  - 不在LVGL任务中执行OTA写入
  - 不跳过magic byte验证
  - 不跳过Update.abort()错误清理
  - 不允许超过分区大小的上传

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: OTA核心逻辑，需要Update API和看门狗集成

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 5, 6, 8)
  - **Blocks**: Tasks 9, 10, 11
  - **Blocked By**: Tasks 1, 4

  **References**:
  - Arduino Update.h API - begin(), write(), end(), abort()
  - ESP-IDF OTA文档 - esp_ota_mark_app_valid_cancel_rollback()
  - Metis review - 看门狗喂狗和magic byte验证

  **Acceptance Criteria**:
  - [ ] ota_handler_init()正确初始化
  - [ ] 固件上传路由可正确处理
  - [ ] magic byte验证已实现
  - [ ] 看门狗喂狗已集成
  - [ ] 错误时调用Update.abort()

  **QA Scenarios**:

  ```
  Scenario: OTA上传成功验证
    Tool: Bash (curl)
    Preconditions: AP模式和Web服务器已启动
    Steps:
      1. 准备有效固件文件firmware.bin
      2. curl -F "file=@firmware.bin" http://192.168.4.1/update
      3. 等待设备重启
    Expected Result: 返回200，设备自动重启
    Failure Indicators: 上传失败或设备未重启
    Evidence: .sisyphus/evidence/task-07-ota-success.txt

  Scenario: 无效固件拒绝验证
    Tool: Bash (curl)
    Preconditions: 服务器已启动
    Steps:
      1. 创建无效文件（非0xE9开头）
      2. curl -F "file=@invalid.bin" http://192.168.4.1/update
    Expected Result: 返回400错误，设备不重启
    Failure Indicators: 接受无效固件或设备重启
    Evidence: .sisyphus/evidence/task-07-ota-invalid.txt

  Scenario: 超大文件拒绝验证
    Tool: Bash (dd + curl)
    Preconditions: 服务器已启动
    Steps:
      1. dd if=/dev/zero of=large.bin bs=1M count=4
      2. curl -F "file=@large.bin" http://192.168.4.1/update
    Expected Result: 返回413错误（文件过大）
    Failure Indicators: 接受超大文件
    Evidence: .sisyphus/evidence/task-07-ota-oversized.txt
  ```

  **Commit**: NO (等待Wave 2完成一起提交)

---

- [x] 8. 创建简洁漂亮的Web页面HTML模板

  **What to do**:
  - 创建嵌入式HTML页面（存储在const char数组中）
  - **UI设计要求**（简洁美观）：
    - 卡片式布局，圆角边框，柔和阴影
    - 渐变色主题（深蓝/紫色调，科技感）
    - 清晰的视觉分区：标题区、WiFi配置区、固件上传区、状态区
    - 统一的按钮样式：圆角、渐变背景、悬停效果
    - 输入框统一样式：圆角边框、聚焦高亮
    - 图标点缀：使用Unicode符号（📶 🔐 ⬆️ 🔄）增加可读性
    - 移动端优先的响应式设计
  - **页面结构**：
    - 顶部：Logo/标题 + 设备状态指示灯（绿色=在线）
    - WiFi配置卡片：
      - 标题 + 图标
      - SSID输入框（带WiFi图标）
      - 密码输入框（带眼睛图标切换显示/隐藏）
      - 保存按钮（渐变色）
    - 固件上传卡片：
      - 标题 + 图标
      - 拖拽上传区域（虚线边框）
      - 文件选择按钮
      - 上传进度条（圆角渐变）
      - 上传按钮
    - 底部：版本信息 + 版权
  - **交互细节**：
    - 表单验证：SSID为空时按钮禁用
    - 上传进度：实时百分比显示
    - 成功/失败提示：Toast消息（绿色成功/红色错误）
    - 平滑动画：按钮点击反馈、进度条动画
  - 中文界面
  - 纯CSS实现（无外部依赖）

  **Must NOT do**:
  - 不使用外部CSS/JS库（如Bootstrap、jQuery）
  - 不暴露敏感信息（如WiFi密码明文显示，除非用户主动切换）
  - 不添加不必要的功能
  - 不使用过多彩色导致视觉混乱

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
    - Reason: 前端UI设计和实现，需要CSS美学和响应式设计能力

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 5, 6, 7)
  - **Blocks**: Task 9, 12
  - **Blocked By**: Tasks 1, 4

  **References**:
  - ESPAsyncWebServer示例 - 嵌入式HTML页面
  - Material Design配色方案 - 科技感渐变色
  - 移动端H5设计规范 - 触控友好布局

  **Acceptance Criteria**:
  - [ ] HTML页面包含WiFi配置表单（SSID + 密码 + 保存）
  - [ ] HTML页面包含固件上传表单（拖拽区 + 进度条 + 上传按钮）
  - [ ] 页面响应式设计，适配手机（320px宽度起）
  - [ ] 界面为中文
  - [ ] 卡片式布局，圆角边框，渐变色主题
  - [ ] 密码输入框支持显示/隐藏切换
  - [ ] 上传进度条实时显示百分比
  - [ ] 成功/失败Toast提示

  **QA Scenarios**:

  ```
  Scenario: 页面渲染验证
    Tool: Bash (curl)
    Preconditions: Web服务器已启动
    Steps:
      1. curl http://192.168.4.1/
      2. 检查HTML内容包含DOCTYPE、head、body
    Expected Result: 返回完整HTML5页面
    Failure Indicators: 页面不完整或格式错误
    Evidence: .sisyphus/evidence/task-08-html-render.txt

  Scenario: 表单元素验证
    Tool: Bash (grep HTML)
    Preconditions: HTML已创建
    Steps:
      1. grep -c "input.*ssid" HTML内容
      2. grep -c "input.*password" HTML内容
      3. grep -c "input.*file" HTML内容
      4. grep -c "button" HTML内容
    Expected Result: 所有表单元素存在（至少4个）
    Failure Indicators: 表单元素缺失
    Evidence: .sisyphus/evidence/task-08-forms.txt

  Scenario: CSS样式验证
    Tool: Bash (grep CSS)
    Preconditions: HTML已创建
    Steps:
      1. grep -c "border-radius" HTML内容
      2. grep -c "gradient" HTML内容
      3. grep -c "@media" HTML内容
    Expected Result: 包含圆角、渐变、媒体查询样式
    Failure Indicators: 样式缺失
    Evidence: .sisyphus/evidence/task-08-styles.txt

  Scenario: 响应式设计验证
    Tool: Bash (grep CSS)
    Preconditions: HTML已创建
    Steps:
      1. grep "max-width.*px" HTML内容
      2. grep "width.*100%" HTML内容
    Expected Result: 包含响应式宽度设置
    Failure Indicators: 无响应式设计
    Evidence: .sisyphus/evidence/task-08-responsive.txt
  ```

  **Commit**: NO (等待Wave 2完成一起提交)

---

- [x] 9. settings_ui新增OTA模式开关

  **What to do**:
  - 在设置页面中添加"网络设置"分类（第6个分类）
  - 添加"OTA模式"开关项：
    - 类型：布尔开关
    - 默认值：关闭
    - 说明：开启后设备创建AP热点用于OTA更新
  - 实现按键导航（与现有5个分类一致）
  - 开关切换时调用wifi_manager_start_ap()或wifi_manager_stop()
  - 开启OTA时在屏幕上显示AP密码

  **Must NOT do**:
  - 不破坏现有5个分类的导航逻辑
  - 不在LVGL任务中执行WiFi操作（使用异步调用）
  - 不跳过AP密码显示

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
    - Reason: LVGL UI修改，需要与现有设置页面集成

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 3 (with Tasks 10, 11, 12)
  - **Blocks**: Task 14, 15
  - **Blocked By**: Tasks 5, 6, 7, 8

  **References**:
  - `src/ui_bridge/settings_ui.cpp` - 现有设置页面实现
  - `src/ui_bridge/settings_ui.h` - settings_ui接口

  **Acceptance Criteria**:
  - [ ] 设置页面新增"网络设置"分类
  - [ ] "OTA模式"开关可正常切换
  - [ ] 开启OTA时显示AP密码
  - [ ] 开关状态正确保存到NVS

  **QA Scenarios**:

  ```
  Scenario: OTA开关切换验证
    Tool: Bash (串口命令 + 按键模拟)
    Preconditions: 固件已烧录
    Steps:
      1. 长按K2进入设置
      2. 导航到"网络设置"
      3. 切换OTA模式开关
    Expected Result: 开关状态改变，串口输出相应日志
    Failure Indicators: 开关无响应或状态不保存
    Evidence: .sisyphus/evidence/task-09-toggle.txt

  Scenario: AP密码显示验证
    Tool: Bash (串口读取)
    Preconditions: OTA模式已开启
    Steps:
      1. 开启OTA模式
      2. 检查屏幕显示
    Expected Result: 屏幕显示AP热点名称和密码
    Failure Indicators: 密码未显示
    Evidence: .sisyphus/evidence/task-09-password-display.txt
  ```

  **Commit**: NO (等待Wave 3完成一起提交)

---

- [x] 10. tasks.cpp新增网络任务

  **What to do**:
  - 在 `tasks.cpp` 中添加网络任务：
    - 任务函数：`net_task(void* pvParameters)`
    - 任务栈：8192字节（TASK_STACK_NET）
    - 核心：Core 0（与传感器/控制任务相同）
    - 优先级：3（低于LVGL，高于传感器）
  - 任务职责：
    - 检查OTA模式状态
    - 如果OTA模式开启：启动AP和Web服务器
    - 如果OTA模式关闭：连接配置的WiFi
    - 定期检查WiFi状态和重连
  - 在 `tasks::start_all()` 中启动网络任务

  **Must NOT do**:
  - 不在Core 1（LVGL核心）运行网络任务
  - 不在LVGL任务中执行网络操作
  - 不忽略任务栈大小（至少8192）

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: FreeRTOS任务集成，需要理解现有任务架构

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 3 (with Tasks 9, 11, 12)
  - **Blocks**: Task 13, 14, 15
  - **Blocked By**: Tasks 5, 6, 7

  **References**:
  - `src/app/tasks.cpp` - 现有任务启动模式
  - `include/app_config.h` - 任务栈常量

  **Acceptance Criteria**:
  - [ ] net_task函数已实现
  - [ ] 任务在Core 0运行
  - [ ] 任务栈至少8192字节
  - [ ] tasks::start_all()启动网络任务

  **QA Scenarios**:

  ```
  Scenario: 网络任务启动验证
    Tool: Bash (串口监视)
    Preconditions: 固件已烧录
    Steps:
      1. 启动设备
      2. 观察串口输出
    Expected Result: 看到"net_task started"日志
    Failure Indicators: 任务未启动或崩溃
    Evidence: .sisyphus/evidence/task-10-task-start.txt

  Scenario: 任务核心分配验证
    Tool: Bash (串口命令)
    Preconditions: 任务已启动
    Steps:
      1. 通过串口查询任务运行核心
    Expected Result: net_task运行在Core 0
    Failure Indicators: 任务在Core 1运行
    Evidence: .sisyphus/evidence/task-10-core.txt
  ```

  **Commit**: NO (等待Wave 3完成一起提交)

---

- [x] 11. main.cpp初始化网络和OTA回滚保护

  **What to do**:
  - 在 `setup()` 中添加：
    - `#include "net/wifi_manager.h"`
    - `#include "net/web_server.h"`
    - `#include "net/ota_handler.h"`
  - 在 `config_manager::init()` 之后调用：
    - `wifi_manager_init()`
    - `web_server_init()`
    - `ota_handler_init()`
  - 添加OTA回滚保护：
    - 在setup()早期调用 `esp_ota_mark_app_valid_cancel_rollback()`
    - 需要 `#include <esp_ota_ops.h>`
  - 检查并记录当前OTA分区信息

  **Must NOT do**:
  - 不跳过OTA回滚保护调用
  - 不在错误的位置初始化网络（必须在config之后）
  - 不忽略初始化顺序

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 主程序集成，需要理解初始化顺序和OTA保护

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 3 (with Tasks 9, 10, 12)
  - **Blocks**: Task 13, 14, 15
  - **Blocked By**: Tasks 5, 6, 7

  **References**:
  - `src/main.cpp` - 现有setup()流程
  - ESP-IDF OTA文档 - esp_ota_mark_app_valid_cancel_rollback()

  **Acceptance Criteria**:
  - [ ] main.cpp包含网络模块头文件
  - [ ] setup()中调用网络初始化函数
  - [ ] setup()中调用esp_ota_mark_app_valid_cancel_rollback()
  - [ ] 初始化顺序正确

  **QA Scenarios**:

  ```
  Scenario: OTA回滚保护验证
    Tool: Bash (串口监视)
    Preconditions: 固件已烧录
    Steps:
      1. 上传新固件
      2. 观察首次启动日志
    Expected Result: 看到"OTA valid marked"日志
    Failure Indicators: 未标记有效或回滚发生
    Evidence: .sisyphus/evidence/task-11-rollback.txt

  Scenario: 初始化顺序验证
    Tool: Bash (串口监视)
    Preconditions: 固件已烧录
    Steps:
      1. 启动设备
      2. 观察初始化日志顺序
    Expected Result: config -> wifi -> web -> ota 顺序正确
    Failure Indicators: 初始化顺序错误或失败
    Evidence: .sisyphus/evidence/task-11-init-order.txt
  ```

  **Commit**: NO (等待Wave 3完成一起提交)

---

- [x] 12. data_bridge显示AP热点信息

  **What to do**:
  - 在 `data_bridge.cpp` 中添加AP信息显示：
    - 当OTA模式开启时，在主屏幕或状态栏显示：
      - AP热点名称
      - AP密码
      - IP地址（192.168.4.1）
  - 使用现有LVGL控件显示信息
  - OTA模式关闭时隐藏AP信息

  **Must NOT do**:
  - 不破坏现有主屏幕布局
  - 不添加过多信息导致界面混乱
  - 不使用不安全的方式显示密码

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
    - Reason: LVGL UI更新，需要与现有data_bridge集成

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Tasks 9, 10, 11)
  - **Blocks**: Task 14, 15
  - **Blocked By**: Tasks 5, 8

  **References**:
  - `src/ui_bridge/data_bridge.cpp` - 现有数据绑定模式
  - `src/ui_bridge/data_bridge.h` - data_bridge接口

  **Acceptance Criteria**:
  - [ ] OTA模式开启时显示AP信息
  - [ ] AP名称、密码、IP正确显示
  - [ ] OTA模式关闭时隐藏AP信息

  **QA Scenarios**:

  ```
  Scenario: AP信息显示验证
    Tool: Bash (串口命令 + 屏幕截图)
    Preconditions: OTA模式已开启
    Steps:
      1. 开启OTA模式
      2. 观察屏幕显示
    Expected Result: 屏幕显示AP名称、密码、IP
    Failure Indicators: 信息未显示或显示错误
    Evidence: .sisyphus/evidence/task-12-ap-display.txt

  Scenario: 信息隐藏验证
    Tool: Bash (串口命令)
    Preconditions: OTA模式已关闭
    Steps:
      1. 关闭OTA模式
      2. 观察屏幕显示
    Expected Result: AP信息不再显示
    Failure Indicators: 信息仍显示
    Evidence: .sisyphus/evidence/task-12-hide.txt
  ```

  **Commit**: YES (with Wave 3)
  - Message: `feat(net): 集成网络功能到设置UI和主程序`
  - Files: `src/ui_bridge/settings_ui.cpp`, `src/app/tasks.cpp`, `src/main.cpp`, `src/ui_bridge/data_bridge.cpp`

---

- [x] 13. 静态分析和编译验证

  **What to do**:
  - 运行 `pio run -e esp32s3` 验证编译成功
  - 运行 `pio check -e esp32s3 --skip-packages` 进行静态分析
  - 修复所有编译错误和警告
  - 验证固件大小在分区限制内

  **Must NOT do**:
  - 不忽略任何警告
  - 不跳过静态分析

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 编译和静态分析验证

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Tasks 14, 15, 16)
  - **Blocks**: Final Verification Wave
  - **Blocked By**: Tasks 9, 10, 11, 12

  **References**:
  - platformio.ini - 构建配置
  - cppcheck文档 - 静态分析规则

  **Acceptance Criteria**:
  - [ ] `pio run -e esp32s3` 编译成功
  - [ ] `pio check` 无错误
  - [ ] 固件大小 < 3.25MB

  **QA Scenarios**:

  ```
  Scenario: 编译验证
    Tool: Bash (pio)
    Preconditions: 所有源代码已完成
    Steps:
      1. pio run -e esp32s3 -t clean
      2. pio run -e esp32s3
    Expected Result: 编译成功，无错误
    Failure Indicators: 编译失败
    Evidence: .sisyphus/evidence/task-13-build.log

  Scenario: 静态分析验证
    Tool: Bash (pio)
    Preconditions: 编译成功
    Steps:
      1. pio check -e esp32s3 --skip-packages
    Expected Result: 无错误
    Failure Indicators: 静态分析发现错误
    Evidence: .sisyphus/evidence/task-13-lint.log
  ```

  **Commit**: YES (with Wave 4)
  - Message: `test(net): 编译和静态分析验证`
  - Files: 无（仅验证）

---

- [x] 14. WiFi AP模式功能测试

  **What to do**:
  - 烧录固件到设备
  - 测试OTA模式开启：
    - 验证AP热点创建
    - 验证AP名称格式（ESM-Power-{MAC}）
    - 验证密码显示在屏幕上
    - 验证可从手机/电脑连接
  - 测试OTA模式关闭：
    - 验证AP热点消失
    - 验证尝试连接配置的WiFi

  **Must NOT do**:
  - 不跳过任何测试场景
  - 不忽略测试失败

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: 功能测试，需要实际设备验证

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Tasks 13, 15, 16)
  - **Blocks**: Final Verification Wave
  - **Blocked By**: Tasks 9, 10, 11, 12

  **References**:
  - 任务5 QA场景 - AP模式测试步骤

  **Acceptance Criteria**:
  - [ ] AP热点可成功创建
  - [ ] 可从其他设备连接到AP
  - [ ] 关闭OTA后AP消失

  **QA Scenarios**:

  ```
  Scenario: AP热点完整测试
    Tool: Bash (iwlist + curl)
    Preconditions: 固件已烧录
    Steps:
      1. 开启OTA模式
      2. iwlist scan | grep ESM-Power
      3. 连接到AP热点
      4. curl http://192.168.4.1/status
    Expected Result: 热点可见，可连接，Web可访问
    Failure Indicators: 热点不可见或无法连接
    Evidence: .sisyphus/evidence/task-14-ap-full-test.txt
  ```

  **Commit**: NO (等待Wave 4完成一起提交)

---

- [x] 15. Web页面功能测试

  **What to do**:
  - 测试Web页面访问：
    - 验证页面可加载
    - 验证WiFi配置表单
    - 验证固件上传表单
  - 测试WiFi配置：
    - 输入SSID和密码
    - 点击保存
    - 验证NVS存储成功
  - 测试状态显示：
    - 验证/status返回正确JSON

  **Must NOT do**:
  - 不跳过表单验证测试
  - 不忽略错误处理

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: Web功能测试，需要浏览器验证

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Tasks 13, 14, 16)
  - **Blocks**: Final Verification Wave
  - **Blocked By**: Tasks 6, 8, 9

  **References**:
  - 任务6 QA场景 - Web服务器测试
  - 任务8 QA场景 - 页面渲染测试

  **Acceptance Criteria**:
  - [ ] Web页面可正常加载
  - [ ] WiFi配置可保存
  - [ ] 状态API返回正确数据

  **QA Scenarios**:

  ```
  Scenario: Web页面完整测试
    Tool: Bash (curl)
    Preconditions: AP模式和Web服务器已启动
    Steps:
      1. curl http://192.168.4.1/ > page.html
      2. 检查页面包含WiFi表单
      3. curl -X POST -d "ssid=TestNet&password=TestPass" http://192.168.4.1/wifi
      4. curl http://192.168.4.1/status
    Expected Result: 页面完整，配置保存成功，状态正确
    Failure Indicators: 页面不完整或配置失败
    Evidence: .sisyphus/evidence/task-15-web-test.txt
  ```

  **Commit**: NO (等待Wave 4完成一起提交)

---

- [x] 16. OTA固件更新测试

  **What to do**:
  - 准备测试固件（修改版本号以便区分）
  - 测试OTA上传：
    - 通过Web页面上传固件
    - 验证设备自动重启
    - 验证新固件运行
  - 测试错误处理：
    - 上传无效固件（非0xE9开头）
    - 上传超大固件（>3.25MB）
    - 验证错误响应和设备不重启

  **Must NOT do**:
  - 不跳过错误场景测试
  - 不忽略回滚机制验证

  **Recommended Agent Profile**:
  - **Category**: `unspecified-high`
    - Reason: OTA核心功能测试，需要实际固件更新验证

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 4 (with Tasks 13, 14, 15)
  - **Blocks**: Final Verification Wave
  - **Blocked By**: Tasks 7, 11

  **References**:
  - 任务7 QA场景 - OTA上传测试

  **Acceptance Criteria**:
  - [ ] 有效固件可成功上传并更新
  - [ ] 无效固件被正确拒绝
  - [ ] 超大固件被正确拒绝
  - [ ] OTA后设备正常运行

  **QA Scenarios**:

  ```
  Scenario: OTA成功更新测试
    Tool: Bash (curl)
    Preconditions: AP模式和Web服务器已启动
    Steps:
      1. 准备有效固件firmware_v2.bin
      2. curl -F "file=@firmware_v2.bin" http://192.168.4.1/update
      3. 等待设备重启
      4. 验证新版本号
    Expected Result: 固件更新成功，版本号已变
    Failure Indicators: 更新失败或版本号未变
    Evidence: .sisyphus/evidence/task-16-ota-success.txt

  Scenario: OTA错误处理测试
    Tool: Bash (curl)
    Preconditions: 服务器已启动
    Steps:
      1. echo "invalid" > invalid.bin
      2. curl -F "file=@invalid.bin" http://192.168.4.1/update
      3. 验证返回400错误
      4. 设备不重启
    Expected Result: 错误被正确处理
    Failure Indicators: 接受无效固件或设备重启
    Evidence: .sisyphus/evidence/task-16-ota-error.txt
  ```

  **Commit**: YES (with Wave 4)
  - Message: `test(net): 网络功能和OTA更新测试`
  - Files: 无（仅测试）

---

## Final Verification Wave (MANDATORY — after ALL implementation tasks)

> 4 review agents run in PARALLEL. ALL must APPROVE. Present consolidated results to user and get explicit "okay" before completing.

- [x] F1. **Plan Compliance Audit** — `oracle`
  Read the plan end-to-end. For each "Must Have": verify implementation exists (read file, curl endpoint, run command). For each "Must NOT Have": search codebase for forbidden patterns — reject with file:line if found. Check evidence files exist in .sisyphus/evidence/. Compare deliverables against plan.
  Output: `Must Have [N/N] | Must NOT Have [N/N] | Tasks [N/N] | VERDICT: APPROVE/REJECT`

- [x] F2. **Code Quality Review** — `unspecified-high`
  Run `pio check -e esp32s3 --skip-packages`. Review all changed files for: `as any`/`@ts-ignore`, empty catches, console.log in prod, commented-out code, unused imports. Check AI slop: excessive comments, over-abstraction, generic names (data/result/item/temp).
  Output: `Build [PASS/FAIL] | Lint [PASS/FAIL] | Files [N clean/N issues] | VERDICT`

- [x] F3. **Real Manual QA** — `unspecified-high`
  Start from clean state. Execute EVERY QA scenario from EVERY task — follow exact steps, capture evidence. Test cross-task integration. Test edge cases: empty state, invalid input, network interruption. Save to `.sisyphus/evidence/final-qa/`.
  Output: `Scenarios [N/N pass] | Integration [N/N] | Edge Cases [N tested] | VERDICT`

- [x] F4. **Scope Fidelity Check** — `deep`
  For each task: read "What to do", read actual diff (git log/diff). Verify 1:1 — everything in spec was built (no missing), nothing beyond spec was built (no creep). Check "Must NOT do" compliance. Detect cross-task contamination. Flag unaccounted changes.
  Output: `Tasks [N/N compliant] | Contamination [CLEAN/N issues] | Unaccounted [CLEAN/N files] | VERDICT`

---

## Commit Strategy

- **Wave 1**: `feat(net): 添加网络基础设施依赖和配置`
- **Wave 2**: `feat(net): 实现WiFi管理、Web服务器和OTA处理`
- **Wave 3**: `feat(net): 集成网络功能到设置UI和主程序`
- **Wave 4**: `test(net): 网络功能测试和验证`

---

## Success Criteria

### Verification Commands
```bash
pio run -e esp32s3                    # 编译验证
pio check -e esp32s3 --skip-packages  # 静态分析
```

### Final Checklist
- [x] ESPAsyncWebServer库依赖已添加
- [x] WiFi AP模式可正常创建热点
- [x] Web页面可通过192.168.4.1访问
- [x] WiFi配置可保存到NVS
- [x] 固件上传可成功更新设备
- [x] 设置页面OTA开关可正常工作
- [x] 看门狗不会在OTA过程中超时
- [x] OTA回滚保护机制已实现
