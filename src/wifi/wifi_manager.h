#pragma once
/**
 * @file wifi_manager.h
 * @brief WiFi 模式管理器 + 完整状态机（AP / STA / OFF）
 *
 * 职责：
 *  - 管理 WiFi 模式生命周期（OFF → AP_MODE → STA_MODE / OFF）
 *  - AP 模式下基于 MAC 地址生成唯一 SSID 和密码
 *  - STA 模式下从 NVS 读取凭据连接路由器
 *  - 密码写入 app_state::wifi_ap_password 供 Web 服务器使用
 *
 * 状态机：
 *   OFF ──(start_ap)──▶ AP_MODE
 *   AP_MODE ──(start_sta◎)──▶ STA_MODE（有凭据）
 *   AP_MODE ──(stop◎)──▶ OFF（无凭据）
 *   STA_MODE ──(stop◎)──▶ OFF
 *
 * 约束：
 *  - 不启动 Web 服务器（Task 8 负责）
 *  - 仅在 esp32s3 env 编译（依赖 <WiFi.h>）
 */

#include <cstdint>

namespace wifi_mgr {

// ── WiFi 状态枚举 ────────────────────────────────────────────────────────────
enum class WifiState : uint8_t {
    OFF     = 0,  // WiFi 关闭
    AP_MODE = 1,  // 接入点模式（热点）
    STA_MODE = 2  // 客户端模式（连接路由器）
};

// ── 生命周期管理 ─────────────────────────────────────────────────────────────
void init();      // 初始化 WiFi（设为 WIFI_OFF 状态）
void start_ap();  // 启动 AP 模式（SSID = 前缀 + MAC 后6位，密码 = MAC 后6位 hex）
void stop();      // 停止当前模式：AP → 检查 STA 凭据决定下一状态，STA → OFF
bool start_sta(); // 启动 STA 模式：读 NVS 凭据 → 连接路由器 → 15s 超时

// ── 状态查询 ─────────────────────────────────────────────────────────────────
WifiState get_state();              // 获取当前 WiFi 状态
const char* get_current_password(); // 获取当前 AP 密码（指向 app_state::wifi_ap_password）

} // namespace wifi_mgr
