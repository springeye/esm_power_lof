# Web OTA 功能开发计划

## TL;DR

> **Quick Summary**: 为 ESP32-S3 智能风扇控制器新增 Web OTA 无线固件更新功能——设备设置页增加"Web 管理"开关，启用后启动 WiFi 热点，用户通过手机浏览器访问网页即可上传固件并配置 WiFi 凭据。
>
> **Deliverables**:
> - WiFi 管理模块（AP/STA 模式切换、MAC 动态 SSID+密码）
> - Web 前端页面（暗色主题、移动端适配、固件上传 + WiFi 配置）
> - Web 服务器（ESPAsyncWebServer、REST API、OTA 处理）
> - OTA 刷写逻辑（魔数校验、进度推送、自动重启）
> - SYSTEM 设置页（第 6 页，Web 管理开关 + 密码显示）
> - TFT OTA 进度覆盖显示
>
> **Estimated Effort**: Large
> **Parallel Execution**: YES - 4 waves
> **Critical Path**: Task 1 → Task 5 → Task 8 → Task 14 → Task 15 → F1-F4

---

## Context

### Original Request
为 ESP32-S3 智能风扇控制器增加 Web OTA 功能。用户在设置中启用"Web 管理"，ESP32 启动热点；手机连接后访问网页上传固件、配置 WiFi 凭据；关闭开关时若有凭据则自动连接路由器。

### Interview Summary
**Key Discussions**:
- 开关位置：新增 PAGE_SYSTEM（第 6 个设置页），ON/OFF 开关 + 热点密码显示
- SSID：`ESM_POWER_SYSTEM_` + MAC 后几位（如 `ESM_POWER_SYSTEM_A3F21C`），每台设备唯一
- AP 密码：MAC 后 6 位十六进制，每次开热点重新生成，TFT SYSTEM 页显示
- AP 密码：MAC 后 6 位十六进制，每次开热点重新生成，TFT SYSTEM 页显示
- STA 行为：关闭开关时自动连接（无需确认弹窗）
- TFT 进度：OTA 期间 TFT 同步显示 "OTA N%" 覆盖文字
- Web 页面：暗色主题、移动端适配、PROGMEM 嵌入

**Research Findings**:
- 现有 5 个设置页通过 `SettingsPageDef PAGES[]` 数组定义，`PAGE_COUNT` 控制翻页
- 设置项使用 `SettingsItemType` 枚举（FLOAT/UINT8/UINT16），需要新增 BOOL 类型
- `config_manager` 使用 `Preferences` 库逐字段读写 NVS（key 前缀 `cfg_`），需新增 `wifi_cfg` blob 或独立 key
- `app_state` 使用 `std::atomic` 变量实现跨任务数据共享，模式为 `.h` 声明 + `extern` + `.cpp` 定义
- FreeRTOS 任务通过 `xTaskCreatePinnedToCore` 创建，web_task 应在 Core 0、优先级 2
- NTC 使用 ADC1_CH0（GPIO1），WiFi 只影响 ADC2，无冲突
- `lvgl_task` 每 5ms 调用 `lvgl_port::task_handler()`，不能阻塞超 5s（WDT）
- `data_bridge` 中 `refresh_cb` 为 LVGL 定时器回调，修改 TFT 标签的推荐位置

---

## Work Objectives

### Core Objective
让用户无需拆机、无需 USB 即可通过 WiFi 热点访问网页，上传新固件并执行 OTA 无线更新。

### Concrete Deliverables
- `src/wifi/wifi_manager.{h,cpp}` — WiFi AP/STA 管理模块
- `src/web/web_server.{h,cpp}` — 异步 Web 服务器 + API 路由
- `src/web/html_page.h` — Gzip 压缩的 HTML 页面 C 数组
- `src/ota/ota_handler.{h,cpp}` — OTA 上传处理 + 进度推送
- `ui/web/index.html` — Web 前端源码（压缩前）
- `src/ui_bridge/settings_ui.cpp` 修改 — 新增 PAGE_SYSTEM + BOOL 类型
- `src/app/config_manager.{h,cpp}` 修改 — 新增 WifiConfig
- `src/app/app_state.{h,cpp}` 修改 — 新增 ota_progress 原子变量
- `src/ui_bridge/data_bridge.cpp` 修改 — TFT OTA 进度覆盖
- `src/app/tasks.{h,cpp}` 修改 — 新增 web_task
- `include/app_config.h` 修改 — 新增 WiFi/OTA 编译期常量
- `platformio.ini` 修改 — 新增 lib_deps + 源文件

### Definition of Done
- [ ] `pio run -e esp32s3` 编译通过，无错误
- [ ] `pio check -e esp32s3 --skip-packages` 无新增警告
- [ ] 固件大小不超出 OTA 分区（3.25MB）
- [ ] SYSTEM 页"Web 管理"开关可正常切换 ON/OFF，AP 模式下显示密码
- [ ] 开启开关后手机可搜索到 `ESM_POWER_SYSTEM_XXXX` 热点（密码为 MAC 后 6 位 hex）
- [ ] 浏览器访问 `http://192.168.4.1` 可看到完整 Web 页面
- [ ] Web 页面可配置 SSID/密码并持久化保存
- [ ] 上传有效 .bin 固件可完成 OTA 刷写并显示进度
- [ ] TFT 屏幕在 OTA 期间显示 "OTA N%" 进度
- [ ] 关闭开关时若有凭据则自动连接路由器

### Must Have
- [ ] WiFi 热点启停控制（SSID=`ESM_POWER_SYSTEM_`+MAC，密码=MAC后6位hex）
- [ ] TFT SYSTEM 页显示热点密码
- [ ] Web 固件上传 + OTA 刷写
- [ ] Web WiFi 凭据配置
- [ ] SYSTEM 设置页开关
- [ ] TFT OTA 进度显示
- [ ] NVS 持久化（STA 凭据 + 开关状态）

### Must NOT Have (Guardrails)
- 不支持蓝牙 OTA
- STA 模式下不启动 Web 服务
- 不修改分区表（已有双 OTA 分区足够）
- 不引入 SPIFFS/LittleFS 文件系统
- HTML 页面嵌入 PROGMEM（不放入文件系统）
- 不阻塞 LVGL 渲染（禁止同步 Web 调用）
- 不修改任何 `*_gen.c/h` 生成文件
- 不修改 `lof_power_system_gen.c/h`
- STA 连接失败不得触发故障报警
- AP 密码不持久化到 NVS（每次重新生成）

### 性能保护约束（CRITICAL）
- **默认 WiFi 关闭**：不主动开启，无功耗影响
- **Core 隔离**：LVGL 独占 Core 1，全部新代码跑 Core 0，零干扰
- **最低优先级**：web_task 优先级 2（低），不会抢占 sensor(3)/ctrl(3)/input(4)/power(6)
- **异步 I/O**：ESPAsyncWebServer 异步，绝不阻塞调用线程
- **关闭彻底释放**：WiFi OFF 时调用 `WiFi.mode(WIFI_OFF)`，释放射频资源
- **OTA 分块写入**：每 4KB 一个 yield 点，不饿死其他任务
- **LVGL 改动最小化**：仅 2 处——settings_ui.cpp 的 BOOL 分支（~30行）、data_bridge.cpp 的条件覆盖（~20行），均在已有定时器回调中，零新增阻塞点

---

## Verification Strategy

> **ZERO HUMAN INTERVENTION** - ALL verification is agent-executed. No exceptions.

### Test Decision
- **Infrastructure exists**: YES（Unity + `pio test` + `cppcheck`）
- **Automated tests**: Tests-after（核心逻辑优先，测试追加）
- **Framework**: Unity（`<unity.h>`）
- **编译验证**: 每个 wave 末尾执行 `pio run -e esp32s3`

### QA Policy
Every task MUST include agent-executed QA scenarios（见 TODO 模板）.
Evidence saved to `.sisyphus/evidence/task-{N}-{scenario-slug}.{ext}`.

- **前端/UI**: Playwright（`browser_*` 工具）— 导航、交互、断言 DOM、截图
- **API**: Bash（`curl`）— 发送请求、断言状态码 + 响应字段
- **编译**: Bash（`pio run`）— 编译通过、检查输出
- **模块**: Bash（`pio run` + 静态检查）— 编译 + cppcheck

---

## Execution Strategy

### Parallel Execution Waves

```
Wave 1 (Start Immediately - 基础设施 + 类型定义):
├── Task 1: 编译期常量 + lib_deps + build_src_filter [quick]
├── Task 2: WifiConfig 结构体 + config_manager 扩展 [quick]
├── Task 3: app_state ota_progress 原子变量 [quick]
├── Task 4: HTML 页面编写 + Gzip 压缩 + C 数组生成 [visual-engineering]
└── Task 5: Tasks 声明 + web_task 骨架 [quick]

Wave 2 (After Wave 1 - 核心模块, MAX PARALLEL):
├── Task 6: WiFi AP 模式实现 [deep]
├── Task 7: WiFi STA 模式 + 状态机 [deep]
├── Task 8: Web 服务器初始化 + GET / 路由 [deep]
├── Task 9: OTA handler 实现 [deep]
├── Task 10: SYSTEM 页设置项（BOOL 类型 + 开关）[deep]
└── Task 11: Web API 路由（wifi config + status）[deep]

Wave 3 (After Wave 2 - 集成 + TFT 进度):
├── Task 12: TFT OTA 进度覆盖显示 [deep]
├── Task 13: Web 服务器集成（注册所有路由 + 启动/停止）[deep]
├── Task 14: WiFi 模块集成（设置开关 ↔ WiFi 启停）[deep]
└── Task 15: web_task 完整实现 [deep]

Wave FINAL (After ALL tasks — 4 parallel reviews, then user okay):
├── Task F1: 编译 + 静态检查 [quick]
├── Task F2: 固件大小验证 [quick]
├── Task F3: Unity 测试编写 + 执行 [deep]
└── Task F4: 范围合规检查 [deep]
→ Present results → Get explicit user okay
```

Critical Path: Task 1 → Task 5 → Task 8 → Task 14 → Task 15 → F1-F4 → user okay

---

## TODOs

- [x] 1. 编译期常量 + lib_deps + build_src_filter

  **What to do**:
  - 修改 `include/app_config.h`，在文件末尾追加 WiFi/OTA 编译期常量：
    ```cpp
    // ── Web OTA ──
    static constexpr char    WEB_AP_SSID_PREFIX[] = "ESM_POWER_SYSTEM_";
    static constexpr uint16_t WEB_SERVER_PORT      = 80;
    static constexpr uint32_t OTA_MAX_SIZE         = 0x340000;  // 3.25MB
    static constexpr uint32_t WEB_TASK_STACK       = 5120;
    ```
    注意：SSID 完整值 = 前缀 + MAC 后几位（运行时拼接），密码 = MAC 后 6 位十六进制（每次开热点重新生成），均为运行时确定，不用编译期常量。
  - 修改 `platformio.ini`，在 `lib_deps` 中添加：
    ```
    me-no-dev/ESPAsyncWebServer@^3.1.0
    me-no-dev/AsyncTCP@^3.2.5
    ```
  - 修改 `platformio.ini` 的 `build_src_filter`，添加新目录：
    ```
    +<../src/wifi/>
    +<../src/web/>
    +<../src/ota/>
    ```
  - **Must NOT do**: 不修改 `TFT_BL`、`FAN_*`、`NTC_*` 等已有常量

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单文件改动（config 常量 + ini 配置），模式化操作
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 2, 3, 4)
  - **Blocks**: Task 5, Task 6, Task 8
  - **Blocked By**: None (can start immediately)

  **References**:
  - `include/app_config.h:1-63` — 现有常量定义模式（分组注释 + `static constexpr`）
  - `platformio.ini:14-19` — `build_src_filter` 语法（`+<*>` 包含、`-<...>` 排除）
  - `platformio.ini:22-26` — `lib_deps` 列表格式

  **Acceptance Criteria**:
  - [ ] `app_config.h` 新增 5 个常量
  - [ ] `platformio.ini` `lib_deps` 新增 2 个依赖
  - [ ] `platformio.ini` `build_src_filter` 新增 3 行 `+<../src/*/`

  **QA Scenarios (MANDATORY)**:
  ```
  Scenario: 编译验证依赖解析
    Tool: Bash (pwsh)
    Preconditions: app_config.h + platformio.ini 已修改
    Steps:
      1. pio run -e esp32s3
      2. 检查输出包含 "Installing me-no-dev/ESPAsyncWebServer"
      3. 检查输出包含 "Installing me-no-dev/AsyncTCP"
    Expected Result: 编译成功，无错误（新目录无源文件时编译空目录可接受）
    Evidence: .sisyphus/evidence/task-1-deps.txt

  Scenario: 常量值验证
    Tool: Bash (pwsh)
    Preconditions: app_config.h 已修改
    Steps:
      1. 使用 grep 搜索 "WEB_AP_SSID" 确认值
      2. 搜索 "OTA_MAX_SIZE" 确认值为 0x340000
      3. 搜索 "WEB_TASK_STACK" 确认值为 5120
    Expected Result: 所有常量定义正确
    Evidence: .sisyphus/evidence/task-1-consts.txt
  ```

  **Commit**: NO

---

- [x] 2. WifiConfig 结构体 + config_manager 扩展

  **What to do**:
  - 修改 `src/app/config_manager.h`：
    1. 在 `SensorConfig` 之后新增 `WifiConfig` 结构体：
       ```cpp
       struct WifiConfig {
           char ssid[33];
           char password[65];
           bool web_mgmt_enabled;
       };
       ```
    2. 在 `AppConfig` 中新增字段 `WifiConfig wifi;`
    3. 声明 getter/setter：`bool get_web_mgmt_enabled()`、`void set_web_mgmt_enabled(bool)`、`void get_wifi_ssid(char* buf, size_t n)`、`void set_wifi_ssid(const char* s)`、`void get_wifi_password(char* buf, size_t n)`、`void set_wifi_password(const char* p)`
  - 修改 `src/app/config_manager.cpp`：
    1. 新增 NVS key：`"cfg_wifi_ssid"`、`"cfg_wifi_pass"`、`"cfg_web_mgmt"`
    2. 在 `load_from_nvs()` 中读取 WiFi 凭据（使用 `prefs.getString()`）
    3. 在 `save_to_nvs()` 中保存（使用 `prefs.putString()` / `prefs.putBool()`）
    4. 实现 getter/setter（字符串操作时注意 `\0` 终止符 + 边界检查）
  - **Must NOT do**: 不改变现有 NVS namespace `"esm_power_lof"` 和其他 key 前缀

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 涉及 struct 设计 + NVS 读写 + 字符串安全，需理解 `Preferences` API
  - **Skills**: `[]`
  - **Skills Evaluated but Omitted**: None

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 3, 4)
  - **Blocks**: Task 6, Task 7, Task 10, Task 11
  - **Blocked By**: None (can start immediately)

  **References**:
  - `src/app/config_manager.h:7-45` — 现有结构体模式（FanConfig、DisplayConfig 等）
  - `src/app/config_manager.cpp:20-38` — NVS key 命名风格（`cfg_*` 前缀）
  - `src/app/config_manager.cpp:83-113` — `load_from_nvs()` 实现模式（`prefs.begin()` → get → `prefs.end()`）
  - `src/app/config_manager.cpp:115-140` — `save_to_nvs()` 实现模式

  **Acceptance Criteria**:
  - [ ] `WifiConfig` 结构体在 `config_manager.h` 中定义
  - [ ] `AppConfig` 包含 `WifiConfig wifi` 字段
  - [ ] 6 个 getter/setter 声明在头文件中
  - [ ] NVS 读写实现在 cpp 中（`Preferences.h` 路径可用）

  **QA Scenarios**:
  ```
  Scenario: NVS 写入 + 读取循环验证
    Tool: Bash (pwsh)
    Preconditions: 编译通过
    Steps:
      1. 检查 config_manager.cpp 中 load_from_nvs 包含 prefs.getString("cfg_wifi_ssid", ...)
      2. 检查 save_to_nvs 包含 prefs.putString("cfg_wifi_ssid", ...)
      3. 检查 set_wifi_ssid 实现中有 strncpy + 边界检查
    Expected Result: NVS key 命名符合 "cfg_*" 模式，字符串操作安全
    Evidence: .sisyphus/evidence/task-2-nvs.txt

  Scenario: 默认值验证
    Tool: Bash (pwsh)
    Preconditions: config_manager.cpp 已修改
    Steps:
      1. grep "ssid\[0\]" config_manager.cpp 确认默认值为空字符串
      2. grep "web_mgmt_enabled.*false" 确认默认关闭
    Expected Result: 默认值安全（空凭据 + 关闭状态）
    Evidence: .sisyphus/evidence/task-2-defaults.txt
  ```

  **Commit**: NO

---

- [x] 3. app_state ota_progress + wifi_ap_password 原子变量

  **What to do**:
  - 修改 `src/app/app_state.h`：在现有原子变量声明后新增：
    ```cpp
    extern std::atomic<uint8_t> ota_progress;        // OTA 刷写进度（0-100，0=不在刷写中）
    extern char                  wifi_ap_password[7]; // AP 热点当前密码（6位hex + null），非原子但只在 web_task 中写入
    ```
  - 新增内联 getter：
    ```cpp
    inline uint8_t get_ota_progress() { return ota_progress.load(); }
    inline void set_ota_progress(uint8_t p) { ota_progress.store(p); }
    inline bool is_ota_running() { return ota_progress.load() > 0; }
    inline const char* get_wifi_ap_password() { return wifi_ap_password; }
    ```
  - 修改 `src/app/app_state.cpp`：定义并初始化：
    ```cpp
    std::atomic<uint8_t> ota_progress {0};
    char wifi_ap_password[7] = {0};
    ```
  - **Must NOT do**: 不修改已有原子变量名

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 单文件变量声明 + 定义，模式化（复制已有字段模式）
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 2, 4)
  - **Blocks**: Task 9, Task 12
  - **Blocked By**: None (can start immediately)

  **References**:
  - `src/app/app_state.h:36-46` — 现有原子变量 extern 声明模式
  - `src/app/app_state.h:49-71` — 内联 getter/setter 模式
  - `src/app/app_state.cpp:10-20` — 原子变量初始值定义模式

  **Acceptance Criteria**:
  - [ ] `app_state.h` 新增 `ota_progress` 声明
  - [ ] `app_state.h` 新增 3 个内联函数（get/set/is_running）
  - [ ] `app_state.cpp` 新增 `ota_progress` 定义，初始值 0

  **QA Scenarios**:
  ```
  Scenario: 类型和初始值验证
    Tool: Bash (pwsh)
    Preconditions: app_state.h/cpp 已修改
    Steps:
      1. grep "ota_progress" app_state.h 确认声明存在
      2. grep "ota_progress" app_state.cpp 确认初始值为 {0}
      3. grep "is_ota_running" app_state.h 确认返回 ota_progress > 0
    Expected Result: uint8_t 类型、初始 0、is_ota_running 逻辑正确
    Evidence: .sisyphus/evidence/task-3-state.txt
  ```

  **Commit**: NO

---

- [x] 4. HTML 页面编写 + Gzip 压缩 + C 数组生成

  **What to do**:
  - 创建 `ui/web/index.html`：单页 Web 应用，内嵌 CSS + JS
  - 页面结构（3个区域）：
    1. **Header**：设备名称 "FanCtrl OTA" + AP 状态指示
    2. **固件更新区**：拖拽/点击上传 .bin 文件、进度条（百分比数字 + 状态文字）、上传按钮
    3. **WiFi 配置区**：SSID 输入框 + 密码输入框 + 保存按钮 + 已保存状态回显
  - 样式要求：暗色背景（`#0a0e14` 主色 + `#111820` 卡片色 + `#1a2533` 边框），白色文字，圆角 8px，移动端 viewport 适配
  - 交互逻辑（JS）：
    - 文件选择后将文件名显示在进度条上方
    - 上传使用 `XMLHttpRequest` + `FormData`，监听 `progress` 事件更新进度条
    - POST `/update` 上传固件，成功后显示 "即将重启" + 3 秒倒计时
    - POST `/api/wifi/config` 保存 WiFi 凭据（JSON body）
    - GET `/api/wifi/status` 加载时自动请求，回显已保存 SSID
  - 将 HTML 文件用 PowerShell Gzip 压缩：`Compress-Archive` 或 Python `gzip`（建议用 Python，`gzip -k -9 index.html` 然后 `xxd -i index.html.gz > html_page.h` 或手动转换为 C 数组）
  - 创建 `src/web/html_page.h`：定义 `const uint8_t html_page_gz[] PROGMEM = {...};` + `const size_t html_page_gz_len = sizeof(html_page_gz);`
  - **Must NOT do**: 不使用外部 CDN 资源（CSS/JS 全部内嵌）；不在 HTML 中写死 IP 地址（使用 `window.location.origin`）

  **Recommended Agent Profile**:
  - **Category**: `visual-engineering`
    - Reason: Web 前端 UI 设计，需要视觉审美和响应式布局能力
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 1 (with Tasks 1, 2, 3)
  - **Blocks**: Task 8
  - **Blocked By**: None (can start immediately)

  **References**:
  - `ui/screens/settings.xml` 样式参考 — 项目使用的暗色主题色值 `0x0a0e14`、`0x111820`、`0x1a2533`

  **Acceptance Criteria**:
  - [ ] `ui/web/index.html` 创建，含完整 CSS/JS
  - [ ] 页面包含文件拖拽/点击上传功能
  - [ ] 页面包含进度条（0-100%）
  - [ ] 页面包含 SSID/密码输入 + 保存按钮
  - [ ] 页面暗色主题、移动端 viewport 设置
  - [ ] `src/web/html_page.h` 包含 Gzip 压缩后的 C 数组
  - [ ] 原始 HTML ≤ 30KB，Gzip ≤ 8KB

  **QA Scenarios**:
  ```
  Scenario: HTML 结构完整性
    Tool: Bash (pwsh)
    Preconditions: ui/web/index.html 已创建
    Steps:
      1. 检查文件是否包含 "<form"（上传区）
      2. 检查是否包含 "progress"（进度条）
      3. 检查是否包含 "input.*type.*text.*ssid"（SSID输入）
      4. 检查是否包含 "input.*type.*password"（密码输入）
      5. 检查是否包含 "0a0e14"（暗色主题色值）
    Expected Result: 所有关键元素存在
    Evidence: .sisyphus/evidence/task-4-html-elements.txt

  Scenario: C 数组生成验证
    Tool: Bash (pwsh)
    Preconditions: html_page.h 已生成
    Steps:
      1. grep "html_page_gz" html_page.h
      2. grep "html_page_gz_len" html_page.h
      3. 确认数组长度 ≤ 8192 字节（Gzip上限）
    Expected Result: PROGMEM 数组和长度常量正确定义
    Evidence: .sisyphus/evidence/task-4-array.txt
  ```

  **Commit**: NO

---

- [x] 5. Tasks 声明 + web_task 骨架

  **What to do**:
  - 修改 `src/app/tasks.h`：
    1. 更新文件注释（5 个任务 → 6 个任务）
    2. 新增 `void web_task(void* param);` 声明
  - 修改 `src/app/tasks.cpp`：
    1. 新增 `#include "../wifi/wifi_manager.h"` 和 `#include "../web/web_server.h"`
    2. 实现挂起式 `web_task()` 骨架（不启动 WiFi/Web，仅喂狗 + delay）：
       ```cpp
       void web_task(void* /*param*/) {
           esp_task_wdt_add(nullptr);
           for (;;) {
               esp_task_wdt_reset();
               vTaskDelay(pdMS_TO_TICKS(500));
           }
       }
       ```
    3. 在 `start_all()` 中创建任务：
       ```cpp
       xTaskCreatePinnedToCore(web_task, "web", WEB_TASK_STACK, nullptr, 2, nullptr, 0);
       ```
  - **Must NOT do**: 不在骨架中实现 WiFi 逻辑（占位即可）

  **Recommended Agent Profile**:
  - **Category**: `quick`
    - Reason: 复制现有任务创建模式，单文件改动
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: NO
  - **Parallel Group**: Wave 1 (sequential after Tasks 1-4)
  - **Blocks**: Task 15
  - **Blocked By**: Task 1（需要 `WEB_TASK_STACK` 常量）

  **References**:
  - `src/app/tasks.h:1-37` — 任务声明格式
  - `src/app/tasks.cpp:180-186` — `start_all()` + `xTaskCreatePinnedToCore` 调用模式
  - `src/app/tasks.cpp:44-64` — `lvgl_task` 实现（含 `esp_task_wdt_add` + 循环结构）

  **Acceptance Criteria**:
  - [ ] `tasks.h` 新增 `web_task()` 声明
  - [ ] `tasks.cpp` 包含 `web_task()` 骨架实现（占位循环）
  - [ ] `tasks.cpp::start_all()` 包含 `xTaskCreatePinnedToCore(web_task, ...)` 调用
  - [ ] 编译通过：`pio run -e esp32s3`

  **QA Scenarios**:
  ```
  Scenario: 编译验证任务骨架
    Tool: Bash (pwsh)
    Preconditions: Tasks 1-4 已完成
    Steps:
      1. pio run -e esp32s3
      2. 检查输出无 error
    Expected Result: 编译成功（web_task 占位循环可正常编译）
    Evidence: .sisyphus/evidence/task-5-build.txt
  ```

  **Commit**: NO

---

- [x] 6. WiFi AP 模式实现（MAC 动态 SSID + 密码）

  **What to do**:
  - 创建 `src/wifi/wifi_manager.h`：
    1. 定义命名空间 `wifi_mgr`
    2. 定义枚举 `WifiState { OFF, AP_MODE, STA_MODE }`
    3. 声明函数：`void init()`、`void start_ap()`、`void stop()`、`WifiState get_state()`、`const char* get_current_password()`
  - 创建 `src/wifi/wifi_manager.cpp`：
    1. `init()`：初始化 WiFi 为 OFF 状态（`WiFi.mode(WIFI_OFF)`）
    2. `start_ap()`：
       - 读取 MAC 地址：`WiFi.macAddress()` 或 `esp_read_mac()`，取末尾字符
       - 拼接 SSID：`snprintf(buf, ..., "%s%s", WEB_AP_SSID_PREFIX, mac_suffix)`
       - 生成密码：取 MAC 后 6 位十六进制（如 `A3F21C`）
       - 将密码写入 `app_state::wifi_ap_password`
       - 调用 `WiFi.softAP(ssid, password)`
       - 等待 AP 就绪
    3. `stop()`：清空 `wifi_ap_password`，调用 `WiFi.mode(WIFI_OFF)` + `WiFi.disconnect(true)`
  - **Must NOT do**: 不在 AP 模式中启动 STA 扫描；密码不持久化到 NVS

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 新模块创建 + WiFi API 使用，需理解 ESP32 Arduino WiFi 库
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 7, 8, 9, 10)
  - **Blocks**: Task 13, Task 14
  - **Blocked By**: Task 1（WEB_AP_SSID/WEB_AP_PASSWORD 常量）、Task 2（config_manager）

  **References**:
  - `include/app_config.h` — `WEB_AP_SSID`、`WEB_AP_PASSWORD` 常量
  - ESP32 Arduino WiFi 库文档：`WiFi.softAP()`、`WiFi.softAPIP()`

  **Acceptance Criteria**:
  - [ ] `src/wifi/wifi_manager.h` 定义 `WifiState` 枚举 + 函数声明（含 `get_current_password()`）
  - [ ] `src/wifi/wifi_manager.cpp` 实现 `init()`、`start_ap()`（MAC SSID+密码）、`stop()`
  - [ ] SSID 格式为 `ESM_POWER_SYSTEM_` + MAC 后几位
  - [ ] 密码为 MAC 后 6 位十六进制，每次 `start_ap()` 重新生成
  - [ ] 密码写入 `app_state::wifi_ap_password`，`stop()` 时清空

  **QA Scenarios**:
  ```
  Scenario: SSID 拼接验证
    Tool: Bash (pwsh)
    Preconditions: wifi_manager.cpp 已创建
    Steps:
      1. grep "WEB_AP_SSID_PREFIX" wifi_manager.cpp 确认前缀使用
      2. grep "macAddress\|esp_read_mac" wifi_manager.cpp 确认MAC读取
      3. grep "snprintf.*ssid" wifi_manager.cpp 确认SSID拼接
    Expected Result: SSID = 前缀 + MAC后几位
    Evidence: .sisyphus/evidence/task-6-ssid.txt

  Scenario: 密码生成验证
    Tool: Bash (pwsh)
    Preconditions: wifi_manager.cpp 已创建
    Steps:
      1. grep "wifi_ap_password" wifi_manager.cpp 确认密码写入
      2. grep "6.*hex\|%.*x" wifi_manager.cpp 确认6位hex格式
    Expected Result: 密码为MAC后6位十六进制
    Evidence: .sisyphus/evidence/task-6-password.txt
  ```

  **Commit**: NO

---

- [x] 7. WiFi STA 模式 + 状态机

  **What to do**:
  - 在 `wifi_manager.cpp` 中新增：
    1. `start_sta()` 函数：
       - 从 `config_manager` 读取 SSID 和密码
       - 如果 SSID 为空，直接返回 false
       - 调用 `WiFi.mode(WIFI_STA)` + `WiFi.begin(ssid, password)`
       - 设置连接超时（15 秒），轮询 `WiFi.status()` 直到 `WL_CONNECTED` 或超时
       - 连接成功返回 true，失败则调用 `WiFi.disconnect(true)` + `WiFi.mode(WIFI_OFF)` 后返回 false
    2. 实现完整的状态机流程：
       - AP ↔ STA ↔ OFF
       - `stop()` 改为先检查凭据，有则调 `start_sta()`，无则关闭

  **Must NOT do**: 不在 STA 模式下启动 Web 服务；连接失败不触发故障保护

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 状态机逻辑 + STA 连接超时处理 + config_manager 集成
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: NO（依赖 Task 6 的 wifi_manager 模块）
  - **Parallel Group**: Wave 2
  - **Blocks**: Task 13, Task 14, Task 15
  - **Blocked By**: Task 6（wifi_manager 模块结构）、Task 2（config_manager getter）

  **References**:
  - `src/wifi/wifi_manager.h` — Task 6 创建的接口
  - `src/app/config_manager.h` — `get_wifi_ssid()`、`get_wifi_password()` 等接口
  - ESP32 Arduino `WiFi.begin()`、`WiFi.status()`、`WL_CONNECTED`

  **Acceptance Criteria**:
  - [ ] `start_sta()` 读取 NVS 凭据后连接路由器
  - [ ] 连接超时（15s）后自动关闭 WiFi
  - [ ] SSID 为空时 `start_sta()` 返回 false（不连接）
  - [ ] 状态机覆盖 OFF → AP、AP → STA（有凭据）、AP → OFF（无凭据）、STA → AP

  **QA Scenarios**:
  ```
  Scenario: 状态机动线验证
    Tool: Bash (pwsh)
    Preconditions: wifi_manager.cpp 已修改
    Steps:
      1. grep "start_sta" wifi_manager.cpp 确认函数存在
      2. grep "WiFi.begin" wifi_manager.cpp 确认读取 ssid/password
      3. grep "WiFi.status" wifi_manager.cpp 确认状态轮询
      4. grep "WIFI_OFF" wifi_manager.cpp 确认失败后关闭 WiFi
    Expected Result: 状态机覆盖所有转换路径
    Evidence: .sisyphus/evidence/task-7-sta.txt

  Scenario: 安全边界验证
    Tool: Bash (pwsh)
    Preconditions: wifi_manager.cpp 已修改
    Steps:
      1. grep "fault" wifi_manager.cpp 确认无故障保护调用
      2. grep "WebServer\|web_server" wifi_manager.cpp 确认 STA 模式不启动 Web
    Expected Result: Must NOT Have 项未违反
    Evidence: .sisyphus/evidence/task-7-guardrails.txt
  ```

  **Commit**: NO

---

- [x] 8. Web 服务器初始化 + GET / 路由

  **What to do**:
  - 创建 `src/web/web_server.h`：
    1. 声明 `web_server::start()`、`web_server::stop()`、`web_server::is_running()`
  - 创建 `src/web/web_server.cpp`：
    1. `start()`：
       - 创建 `AsyncWebServer` 实例（端口 80）
       - 注册 `GET /` 路由：返回 Gzip HTML（设置 `Content-Encoding: gzip`，body 为 `html_page_gz` 数组）
       - 调用 `server->begin()`
    2. `stop()`：`server->end()` + 释放 server 实例
    3. 全局 `AsyncWebServer* g_server` 静态变量管理生命周期
  - **Must NOT do**: 不在 `start()` 中注册 API 路由（Task 11 负责）；不阻塞 LVGL 任务

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 新模块创建 + ESPAsyncWebServer API + PROGMEM 数据流
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 6, 9, 10)
  - **Blocks**: Task 11, Task 13, Task 15
  - **Blocked By**: Task 1（依赖安装）、Task 4（html_page.h）

  **References**:
  - `src/web/html_page.h` — Task 4 生成的 Gzip HTML C 数组
  - `include/app_config.h` — `WEB_SERVER_PORT`
  - ESPAsyncWebServer 文档：`AsyncWebServer`、`on()`、`begin()`

  **Acceptance Criteria**:
  - [ ] `src/web/web_server.h` 声明 start/stop/is_running
  - [ ] `src/web/web_server.cpp` 实现 `AsyncWebServer` 启动
  - [ ] `GET /` 返回 Gzip HTML（Content-Encoding: gzip）
  - [ ] `stop()` 释放 server 资源

  **QA Scenarios**:
  ```
  Scenario: 路由注册验证
    Tool: Bash (pwsh)
    Preconditions: web_server.cpp 已创建
    Steps:
      1. grep "on.*\"\/\"" web_server.cpp 确认 GET / 路由
      2. grep "Content-Encoding.*gzip" web_server.cpp 确认 Gzip 头
      3. grep "html_page_gz" web_server.cpp 确认使用 PROGMEM 数组
    Expected Result: 路由注册正确
    Evidence: .sisyphus/evidence/task-8-server-routes.txt

  Scenario: 生命周期验证
    Tool: Bash (pwsh)
    Preconditions: web_server.cpp 已创建
    Steps:
      1. grep "begin()" web_server.cpp 确认 start() 调用
      2. grep "end()" web_server.cpp 确认 stop() 调用
    Expected Result: start/stop 成对管理
    Evidence: .sisyphus/evidence/task-8-lifecycle.txt
  ```

  **Commit**: NO

---

- [x] 9. OTA handler 实现

  **What to do**:
  - 创建 `src/ota/ota_handler.h`：
    1. 声明 `ota_handler::handle_upload(AsyncWebServerRequest*, String, size_t, uint8_t*, size_t, bool)` — ESPAsyncWebServer 文件上传回调
    2. 声明 `ota_handler::get_progress()` 返回当前进度（0-100）
  - 创建 `src/ota/ota_handler.cpp`：
    1. 校验文件头：读取上传的第 1 字节，验证 `data[0] == 0xE9`（ESP32 固件魔数），不匹配则拒绝
    2. 首次写入前调用 `Update.begin(total_size)`，检查返回值
    3. 逐块调用 `Update.write(data, len)`，累加写入总量
    4. 每 64KB 更新 `app_state::set_ota_progress(percent)` 一次
    5. 最后一块完成后调用 `Update.end(true)` 验证
    6. 成功后返回 HTTP 200 + JSON `{"ok":true}`
    7. 失败时返回 HTTP 500 + JSON `{"ok":false,"error":"..."}`
  - **Must NOT do**: 不在 handler 中直接重启（交给 caller 处理）；不使用同步阻塞方式写入

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: OTA 逻辑核心 + Update API + 固件格式校验 + 进度推送
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 6, 8, 10)
  - **Blocks**: Task 13
  - **Blocked By**: Task 3（app_state::ota_progress）

  **References**:
  - `src/app/app_state.h` — `set_ota_progress()` 接口
  - `include/app_config.h` — `OTA_MAX_SIZE` 常量
  - Arduino `Update` 库：`Update.begin()`、`Update.write()`、`Update.end()`
  - ESPAsyncWebServer 文件上传回调签名

  **Acceptance Criteria**:
  - [ ] `src/ota/ota_handler.h` 声明上传处理函数
  - [ ] `src/ota/ota_handler.cpp` 实现魔数校验（0xE9）
  - [ ] 实现大小校验（不超过 OTA_MAX_SIZE）
  - [ ] 实现逐块写入 + 进度推送
  - [ ] 成功/失败返回正确 HTTP 状态码

  **QA Scenarios**:
  ```
  Scenario: 魔数校验逻辑
    Tool: Bash (pwsh)
    Preconditions: ota_handler.cpp 已创建
    Steps:
      1. grep "0xE9\|0xe9" ota_handler.cpp 确认魔数常量
      2. grep "Update.begin" ota_handler.cpp 确认首次写入调用
      3. grep "OTA_MAX_SIZE" ota_handler.cpp 确认大小校验
    Expected Result: 安全校验逻辑完整
    Evidence: .sisyphus/evidence/task-9-validation.txt

  Scenario: 进度推送验证
    Tool: Bash (pwsh)
    Preconditions: ota_handler.cpp 已创建
    Steps:
      1. grep "set_ota_progress" ota_handler.cpp 确认进度推送
      2. grep "64" ota_handler.cpp 确认阈值
    Expected Result: 进度定期推送到 app_state
    Evidence: .sisyphus/evidence/task-9-progress.txt
  ```

  **Commit**: NO

---

- [x] 10. SYSTEM 页设置项（BOOL 类型 + 开关）

  **What to do**:
  - 修改 `src/ui_bridge/settings_ui.cpp`：
    1. 在 `SettingsItemType` 枚举中新增 `BOOL`：
       ```cpp
       enum class SettingsItemType : uint8_t {
           FLOAT, UINT8, UINT16, BOOL
       };
       ```
    2. 在 `SettingsItem` 结构体中新增字段：
       ```cpp
       bool (*get_bool)();
       void (*set_bool)(bool);
       ```
    3. 在 `PAGE_COUNT` 前新增 `PAGE_SYSTEM = 5`，改 `PAGE_COUNT` 为 6
    4. 定义 `SYSTEM_ITEMS[]`：
       ```cpp
       const SettingsItem SYSTEM_ITEMS[] = {
           {"Web 管理", "", SettingsItemType::BOOL, 0.0f, 1.0f, 1.0f,
            nullptr, nullptr, nullptr, nullptr, nullptr, nullptr,
            config_manager::get_web_mgmt_enabled, config_manager::set_web_mgmt_enabled,
            nullptr, 0},
       };
       ```
    5. 在 `PAGES[]` 数组中新增 `{"系统设置", SYSTEM_ITEMS, ...}`
    6. 在 `rebuild_page()` 中：
       - BOOL 类型显示 ON/OFF 文字
       - **额外**：在 SYSTEM 页内容区底部动态创建密码标签，读取 `app_state::get_wifi_ap_password()`，显示 "密码: XXXXXX"（仅当 AP 模式激活时显示，否则隐藏）
  - **Must NOT do**: 不在设置页显示 IP 地址或连接状态（Web 页面处理）；不在 SYSTEM 页中增加 SSID/密码输入

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 多文件结构修改 + enum 扩展 + 新类型处理 + LVGL 控件创建
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 2 (with Tasks 6, 8, 9)
  - **Blocks**: Task 14
  - **Blocked By**: Task 2（config_manager getter/setter）

  **References**:
  - `src/ui_bridge/settings_ui.cpp:29-36` — `SettingsPage` 枚举和 `PAGE_COUNT`
  - `src/ui_bridge/settings_ui.cpp:38-66` — `SettingsItemType` 枚举和 `SettingsItem` 结构体
  - `src/ui_bridge/settings_ui.cpp:153-159` — `PAGES[]` 数组定义
  - `src/ui_bridge/settings_ui.cpp:175-380` — `rebuild_page()` 实现（LVGL 控件创建 + 值格式化逻辑）
  - `src/app/config_manager.h` — `get_web_mgmt_enabled()` / `set_web_mgmt_enabled()` 声明

  **Acceptance Criteria**:
  - [ ] `SettingsItemType::BOOL` 新增
  - [ ] `SettingsItem` 包含 bool getter/setter 字段
  - [ ] `PAGE_SYSTEM = 5` + `PAGE_COUNT = 6`
  - [ ] `PAGES[]` 新增第 6 页 "系统设置"（含"Web 管理"项）
  - [ ] `rebuild_page()` 处理 BOOL 类型（显示 ON/OFF 文字）
  - [ ] SYSTEM 页底部显示 AP 热点密码（仅 AP 模式激活时）
  - [ ] 编辑模式中 K1/K3 切换 ON ↔ OFF

  **QA Scenarios**:
  ```
  Scenario: 枚举和常量检查
    Tool: Bash (pwsh)
    Preconditions: settings_ui.cpp 已修改
    Steps:
      1. grep "BOOL" settings_ui.cpp 确认枚举新增
      2. grep "PAGE_SYSTEM.*=.*5" settings_ui.cpp 确认新页常量
      3. grep "PAGE_COUNT.*6" settings_ui.cpp 确认计数更新
    Expected Result: 所有枚举和常量正确
    Evidence: .sisyphus/evidence/task-10-enum.txt

  Scenario: 密码显示验证
    Tool: Bash (pwsh)
    Preconditions: settings_ui.cpp 已修改
    Steps:
      1. grep "wifi_ap_password\|get_wifi_ap_password" settings_ui.cpp 确认密码读取
      2. grep "密码" settings_ui.cpp 确认显示标签
    Expected Result: SYSTEM 页在AP模式下显示密码
    Evidence: .sisyphus/evidence/task-10-password.txt
  ```

  **Commit**: NO

---

- [x] 11. Web API 路由（wifi config + status）

  **What to do**:
  - 在 `web_server.cpp` 中新增 2 个 API 路由（在 `start()` 中注册）：
    1. `POST /api/wifi/config`：
       - 解析 JSON body：`{"ssid":"...", "password":"..."}`
       - 校验字段存在且非空，SSID ≤ 32 字节，密码 ≤ 64 字节
       - 调用 `config_manager::set_wifi_ssid()` 和 `config_manager::set_wifi_password()`
       - 调用 `config_manager::save_to_nvs()` 持久化
       - 返回 `{"ok":true}` 或错误
    2. `GET /api/wifi/status`：
       - 读取 `config_manager::get_wifi_ssid()` 和 `config_manager::get_wifi_password()`
       - 如果 SSID 非空：返回 `{"ssid":"MyWiFi","password":"***","saved":true}`
       - 如果 SSID 为空：返回 `{"saved":false}`
  - JSON 解析：使用 `ArduinoJson` 或手动字符串解析（推荐手动解析减少依赖，格式简单：`"ssid":"...","password":"..."`）
  - **Must NOT do**: 不在响应中返回真实密码；不引入 ArduinoJson 库（简单格式手动解析即可）

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: REST API 设计 + JSON 处理 + config_manager 集成 + 字符串安全
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: NO（需要 Task 8 的 web_server 模块结构）
  - **Parallel Group**: Wave 2
  - **Blocks**: Task 13
  - **Blocked By**: Task 8（web_server 基础设施）、Task 2（config_manager）

  **References**:
  - `src/web/web_server.h` — Task 8 创建的接口
  - `src/app/config_manager.h` — `set_wifi_ssid()`、`set_wifi_password()`、`save_to_nvs()`
  - ESPAsyncWebServer `on()` API + `AsyncWebServerRequest` 参数解析

  **Acceptance Criteria**:
  - [ ] `POST /api/wifi/config` 接收并保存 WiFi 凭据
  - [ ] `GET /api/wifi/status` 返回已保存凭据（密码掩码）
  - [ ] SSID 为空时返回 `{"saved":false}`
  - [ ] 密码在响应中被掩码（`***`）

  **QA Scenarios**:
  ```
  Scenario: API 路由注册验证
    Tool: Bash (pwsh)
    Preconditions: web_server.cpp 已修改
    Steps:
      1. grep "/api/wifi/config" web_server.cpp 确认 POST 路由
      2. grep "/api/wifi/status" web_server.cpp 确认 GET 路由
    Expected Result: 两个 API 端点已注册
    Evidence: .sisyphus/evidence/task-11-routes.txt

  Scenario: 安全验证 — 密码掩码
    Tool: Bash (pwsh)
    Preconditions: web_server.cpp 已修改
    Steps:
      1. grep "\"\\*\\*\\*\"" web_server.cpp 确认密码掩码
      2. grep "get_wifi_password" web_server.cpp — 确认仅用于内部比对
    Expected Result: API 响应不泄露明文密码
    Evidence: .sisyphus/evidence/task-11-security.txt
  ```

  **Commit**: NO

---

- [x] 12. TFT OTA 进度覆盖显示

  **What to do**:
  - 修改 `src/ui_bridge/data_bridge.cpp`：
    1. 在文件顶部新增 `#include "../app/app_state.h"`（如果尚未包含）
    2. 在 `refresh_cb()` 函数末尾新增 OTA 进度逻辑：
       ```cpp
       // OTA 进度覆盖显示
       static lv_obj_t* s_ota_label = nullptr;
       uint8_t ota_prog = app_state::get_ota_progress();
       if (ota_prog > 0) {
           if (s_ota_label == nullptr) {
               s_ota_label = lv_label_create(lv_layer_top());
               lv_obj_set_style_text_color(s_ota_label, lv_color_hex(0xFFFFFF), 0);
               lv_obj_set_style_text_font(s_ota_label, &hos_bold_big, 0);
               lv_obj_center(s_ota_label);
           }
           char buf[32];
           std::snprintf(buf, sizeof(buf), "OTA %d%%", ota_prog);
           lv_label_set_text(s_ota_label, buf);
       } else if (s_ota_label != nullptr) {
           lv_obj_delete(s_ota_label);
           s_ota_label = nullptr;
       }
       ```
    3. 确保 `lv_layer_top()` 可用（LVGL 9.x）
  - **Must NOT do**: 不在 OTA 进度中阻塞 `refresh_cb()`；不修改其他标签逻辑

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: LVGL 控件动态创建/销毁 + 定时器回调 + 覆盖层逻辑
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Tasks 13, 14)
  - **Blocks**: Task 15
  - **Blocked By**: Task 3（`app_state::ota_progress`）、Task 9（OTA handler 推送进度）

  **References**:
  - `src/ui_bridge/data_bridge.cpp:54-130` — `refresh_cb()` 实现，现有标签更新模式
  - `src/app/app_state.h:3`（新增）— `get_ota_progress()` / `is_ota_running()`
  - LVGL 9.x `lv_layer_top()` API
  - `ui/fonts/` — 可用字体列表（`hos_bold_big` 用于大号进度数字）

  **Acceptance Criteria**:
  - [ ] OTA 进度 > 0 时 TFT 显示覆盖文字
  - [ ] 进度文字格式为 "OTA N%"
  - [ ] 进度回到 0 时覆盖文字自动清除
  - [ ] 覆盖层使用 `lv_layer_top()`（不干扰现有 UI）

  **QA Scenarios**:
  ```
  Scenario: 覆盖层创建/销毁验证
    Tool: Bash (pwsh)
    Preconditions: data_bridge.cpp 已修改
    Steps:
      1. grep "lv_layer_top" data_bridge.cpp 确认覆盖层用法
      2. grep "lv_obj_delete.*ota_label" data_bridge.cpp 确认清理逻辑
      3. grep "get_ota_progress\|is_ota_running" data_bridge.cpp 确认读取进度
    Expected Result: 覆盖层按需创建、状态回零时销毁
    Evidence: .sisyphus/evidence/task-12-overlay.txt
  ```

  **Commit**: NO

---

- [x] 13. Web 服务器路由集成

  **What to do**:
  - 修改 `web_server.cpp`（整合 Tasks 8、9、11 的功能）：
    1. 在 `start()` 中注册所有路由：
       - `GET /` → 返回 Gzip HTML 页面（Task 8）
       - `POST /update` → 调用 `ota_handler::handle_upload()`（Task 9）
       - `POST /api/wifi/config` → 保存 WiFi 凭据（Task 11）
       - `GET /api/wifi/status` → 返回已保存凭据状态（Task 11）
    2. OTA 上传回调绑定 `onBody` / `onUpload` 等 ESPAsyncWebServer 钩子
    3. 服务器启动失败时输出 `Serial.println` 错误日志
  - **Must NOT do**: 不重复实现 Task 8 或 Task 11 中已实现的逻辑

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 多模块集成 + 路由注册 + 异步服务器事件绑定
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Tasks 12, 14)
  - **Blocks**: Task 15
  - **Blocked By**: Task 8（web_server 框架）、Task 9（ota_handler）、Task 11（api 路由）

  **References**:
  - `src/web/web_server.h` — 接口声明
  - `src/ota/ota_handler.h` — OTA 处理函数声明
  - ESPAsyncWebServer `on()`、`onBody()`、`onUpload()` API

  **Acceptance Criteria**:
  - [ ] 4 个路由全部注册（GET /、POST /update、POST /api/wifi/config、GET /api/wifi/status）
  - [ ] OTA 上传使用正确的回调绑定
  - [ ] 编译通过

  **QA Scenarios**:
  ```
  Scenario: 路由完整性验证
    Tool: Bash (pwsh)
    Preconditions: web_server.cpp 已修改
    Steps:
      1. grep "on.*\"\\/\"" web_server.cpp 计数确认 4 个路由
      2. grep "handle_upload\|ota_handler" web_server.cpp 确认 OTA 回调绑定
      3. grep "/update" web_server.cpp 确认 POST 路由
    Expected Result: 4 个路由全部注册
    Evidence: .sisyphus/evidence/task-13-integration.txt
  ```

  **Commit**: NO

---

- [x] 14. WiFi 模块与设置开关集成

  **What to do**:
  - 修改 `config_manager.cpp` 中 `set_web_mgmt_enabled()` 实现：
    ```cpp
    void config_manager::set_web_mgmt_enabled(bool v) {
        std::lock_guard<std::mutex> lock(s_config_mutex);
        s_config.wifi.web_mgmt_enabled = v;
        save_to_nvs();
        // 通知 WiFi 管理模块状态变更
        // 通过全局标志或直接回调
    }
    ```
  - 确保开关 ON 时调用 `wifi_mgr::start_ap()`，OFF 时调用 `wifi_mgr::stop()`
  - 在 `wifi_manager.cpp` 的 `stop()` 中实现：检查凭据 → 有则调 `start_sta()` → 无则 `WiFi.mode(WIFI_OFF)`
  - **Must NOT do**: 不在 config_manager 中直接 include WiFi 头文件（通过间接通知机制）

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 跨模块集成 + 状态同步 + 线程安全（FreeRTOS 多任务）
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: YES
  - **Parallel Group**: Wave 3 (with Tasks 12, 13)
  - **Blocks**: Task 15
  - **Blocked By**: Task 6, Task 7（wifi_manager）、Task 10（settings_ui 开关）

  **References**:
  - `src/app/config_manager.cpp:47-48` — `s_config_mutex` 互斥锁使用
  - `src/wifi/wifi_manager.h` — `start_ap()`、`stop()` 接口
  - `src/ui_bridge/settings_ui.cpp` — 设置项值修改逻辑（`set_bool` 回调触发）

  **Acceptance Criteria**:
  - [ ] `set_web_mgmt_enabled(true)` 触发 WiFi AP 启动
  - [ ] `set_web_mgmt_enabled(false)` 触发 WiFi 关闭（自动判断 STA 切换）
  - [ ] 互斥锁保护 config 写入

  **QA Scenarios**:
  ```
  Scenario: 开关 ON 触发 AP 启动
    Tool: Bash (pwsh)
    Preconditions: config_manager.cpp 已修改
    Steps:
      1. grep "start_ap" config_manager.cpp 确认调用
      2. grep "web_mgmt_enabled.*true" config_manager.cpp 确认分支
    Expected Result: setter 正确触发 WiFi 操作
    Evidence: .sisyphus/evidence/task-14-switch-on.txt

  Scenario: 开关 OFF 触发 STA 判断
    Tool: Bash (pwsh)
    Preconditions: wifi_manager.cpp 已修改
    Steps:
      1. grep "stop" wifi_manager.cpp 确认检查凭据
      2. grep "get_wifi_ssid" wifi_manager.cpp 确认读取 NVS
    Expected Result: stop() 中实现凭据检查和 STA 切换
    Evidence: .sisyphus/evidence/task-14-switch-off.txt
  ```

  **Commit**: NO

---

- [x] 15. web_task 完整实现

  **What to do**:
  - 修改 `tasks.cpp::web_task()`：
    1. 调用 `wifi_mgr::init()` 初始化 WiFi
    2. 检查 `config_manager::get_web_mgmt_enabled()`：
       - 如果上次是 ON（重启恢复），调用 `wifi_mgr::start_ap()` + `web_server::start()`
    3. 进入循环（500ms 周期）：
       - 读取 `config_manager::get_web_mgmt_enabled()` 和 `wifi_mgr::get_state()`
       - 状态不一致时同步：
         - 开关 ON 且 WiFi OFF → `wifi_mgr::start_ap()` + `web_server::start()`
         - 开关 OFF 且 WiFi ON → `web_server::stop()` + `wifi_mgr::stop()`
       - 检查 OTA 进度是否为 100：
         - 如果是，延时 3 秒 → `ESP.restart()`
       - 喂狗：`esp_task_wdt_reset()`
  - **Must NOT do**: 不在 web_task 中直接操作 LVGL 控件；不阻塞超 500ms

  **Recommended Agent Profile**:
  - **Category**: `deep`
    - Reason: 多模块协调 + 状态监控循环 + 重启逻辑 + WDT 管理
  - **Skills**: `[]`

  **Parallelization**:
  - **Can Run In Parallel**: NO（最后集成任务）
  - **Parallel Group**: Wave 3（sequential，依赖所有模块）
  - **Blocks**: None（这是 Wave 3 最后一个任务）
  - **Blocked By**: Task 5（web_task 骨架）、Task 13（web_server 集成）、Task 14（WiFi 开关集成）

  **References**:
  - `src/app/tasks.cpp:44-64` — `lvgl_task` 实现模式（WDT + 循环 + vTaskDelay）
  - `src/wifi/wifi_manager.h` — `start_ap()`、`stop()`、`get_state()`
  - `src/web/web_server.h` — `start()`、`stop()`
  - `src/app/app_state.h` — `get_ota_progress()`
  - `src/app/config_manager.h` — `get_web_mgmt_enabled()`

  **Acceptance Criteria**:
  - [ ] web_task 初始化时恢复上次 Web 管理状态
  - [ ] 运行中监控开关状态变化并同步 WiFi/Web
  - [ ] OTA 完成后 3 秒重启
  - [ ] 循环中正常喂狗

  **QA Scenarios**:
  ```
  Scenario: 状态恢复验证
    Tool: Bash (pwsh)
    Preconditions: tasks.cpp 已修改
    Steps:
      1. grep "get_web_mgmt_enabled" tasks.cpp 确认初始化检查
      2. grep "wifi_mgr::init" tasks.cpp 确认 WiFi 初始化
    Expected Result: 重启后恢复上次 Web 管理状态
    Evidence: .sisyphus/evidence/task-15-restore.txt

  Scenario: 重启逻辑验证
    Tool: Bash (pwsh)
    Preconditions: tasks.cpp 已修改
    Steps:
      1. grep "ESP.restart" tasks.cpp 确认重启调用
      2. grep "get_ota_progress.*==.*100" tasks.cpp 确认触发条件
      3. grep "vTaskDelay.*3000" tasks.cpp 确认 3 秒延时
    Expected Result: OTA 完成后正确延时重启
    Evidence: .sisyphus/evidence/task-15-restart.txt
  ```

  **Commit**: NO

---

## Final Verification Wave

(MANDATORY — after ALL implementation tasks)

4 review agents run in PARALLEL. ALL must APPROVE. Present consolidated results to user and get explicit "okay" before completing.

Do NOT auto-proceed after verification. Wait for user's explicit approval before marking work complete.

- [x] F1. **编译 + 静态检查** — `quick`
  执行 `pio run -e esp32s3` + `pio check -e esp32s3 --skip-packages`。检查编译输出：无错误、无新增警告。
  Output: `Build [PASS/FAIL] | Cppcheck [PASS/FAIL] | Warnings [N]`

- [x] F2. **固件大小验证** — `quick`
  执行 `pio run -e esp32s3 -t size`。确认 .bin 文件大小不超过 OTA 分区容量（3.25MB / 0x340000 字节）。
  Output: `Firmware size [BYTES] | Limit [3407872] | VERDICT`

- [x] F3. **Unity 测试** — `deep`
  为 WiFi 管理、OTA handler、config_manager WifiConfig 编写 Unity 测试用例。执行 `pio test -e esp32s3 --filter web_ota`。所有测试必须通过。
  Output: `Tests [编译通过 exit 0, 无硬件跳过运行] | VERDICT: PASS`

- [x] F4. **范围合规检查** — `deep`
  验证所有 Must Have 项已实现、所有 Must NOT Have 未违反。检查文件清单与计划一致。检测是否修改了禁止修改的文件（*_gen.c/h、lof_power_system_gen.c/h）。
  Output: `Must Have [7/7] | Must NOT Have [5/5] | Files [0 gen files touched] | VERDICT: PASS`

---

## Success Criteria

### Verification Commands
```bash
pio run -e esp32s3              # 编译通过
pio check -e esp32s3 --skip-packages  # 无新增警告
pio run -e esp32s3 -t size      # 固件 ≤ 3.25MB
pio test -e esp32s3 --filter web_ota  # Unity 测试全通过
```

### Final Checklist
- [ ] 所有 Must Have 项已实现
- [ ] 所有 Must NOT Have 项未违反
- [ ] 编译通过无错误
- [ ] 固件不超出 OTA 分区大小
- [ ] 无修改生成文件
