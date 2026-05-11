# Learnings

## 2026-05-11 Wave 1-2 完成

### 编译修复经验
- `html_page.h` 需要 `#include <cstdint>` 才能使用 `uint8_t`/`uint32_t`
- `ota_handler.h` 需要 `#include <WString.h>` 才能使用 `String` 类型
- LSP 报错（lvgl.h/pgmspace.h/WString.h 找不到）全为 clangd 误报，不影响 `pio run`

### SettingsItem 字段顺序（修改后）
`get_float, set_float, get_u8, set_u8, get_u16, set_u16, get_bool, set_bool, preset_values, preset_count`
所有现有 item 初始化列表末尾需追加 `nullptr, nullptr,`（bool 字段）

### html_page.h 重新生成方式
PowerShell 读取 `ui/web/index.html` → GZipStream → 逗号分隔 C 数组写入 `src/web/html_page.h`
字节之间必须用逗号分隔（`0x1f, 0x8b`），不能用空格

### wifi_manager.cpp 关键实现
- `start_ap()` SSID = `WEB_AP_SSID_PREFIX` + MAC[3-5] hex
- 密码写入 `app_state::wifi_ap_password`（char[7]，6位hex+null）
- 固定 IP 192.168.4.1

### 编译结果（Wave 2 完成后）
- RAM: 83.4%（273KB/320KB）
- Flash: 36.5%（1.24MB/3.4MB）—— 远低于 OTA_MAX_SIZE 限制

## 2026-05-11 WiFi STA 状态机实现

### 实现要点
- `start_sta()` 从 `config_manager::get_wifi_ssid()`/`get_wifi_password()` 读取 NVS 凭据
- config_manager 接口使用 buffer+size 方式（`get_wifi_ssid(char* buf, size_t n)`），非 const char* 返回值
- 空 SSID 直接返回 false，不尝试连接
- 轮询 15s 超时（30次 × 500ms `vTaskDelay`），避免看门狗
- 超时后 `WiFi.disconnect(true)` + `WiFi.mode(WIFI_OFF)` 清理
- `stop()` 关闭 AP 后检查 SSID：有凭据 → `start_sta()`，无凭据 → OFF
- 需要 `#include <freertos/FreeRTOS.h>` + `<freertos/task.h>` 才能使用 `vTaskDelay`/`pdMS_TO_TICKS`
- `#include "../app/config_manager.h"` 相对 include 路径

### 编译验证
- `pio run -e esp32s3` 通过，无错误无警告

## 2026-05-11 14:27 - POST /update 路由集成
- web_server.cpp 编译在首次 pio run 时被 data_bridge.cpp:170 的预存错误阻断，第二次运行成功（可能是增量编译缓存问题）
- g_server->on() 的 onFileUpload 回调签名为 (AsyncWebServerRequest*, const String&, size_t, uint8_t*, size_t, bool)，与 ota_handler::handle_upload 一致

