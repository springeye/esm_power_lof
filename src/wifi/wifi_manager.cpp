/**
 * @file wifi_manager.cpp
 * @brief WiFi 模式管理器 + 完整状态机实现（AP / STA / OFF）
 *
 * 功能：
 *  - init() / start_ap() / start_sta() / stop() 控制 WiFi 生命周期
 *  - AP SSID = "ESM_POWER_SYSTEM_" + MAC 后 6 位（大写十六进制）
 *  - AP 密码 = MAC 后 6 位（大写十六进制，如 A3F21C）
 *  - STA 从 NVS 读取凭据，15s 超时轮询
 *  - stop() 关闭 AP 后检查 STA 凭据决定下一状态
 */

#include <WiFi.h>
#include <cstring>

#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

#include "wifi_manager.h"
#include "../app/app_state.h"
#include "../app/config_manager.h"
#include "../../include/app_config.h"

namespace wifi_mgr {

// ── 静态状态变量 ─────────────────────────────────────────────────────────────
static WifiState s_state = WifiState::OFF;

// ── 生命周期实现 ─────────────────────────────────────────────────────────────

void init() {
    WiFi.mode(WIFI_OFF);
    s_state = WifiState::OFF;
}

void start_ap() {
    // 1. 读取 MAC 地址
    uint8_t mac[6];
    WiFi.macAddress(mac);

    // 2. 拼接 SSID：前缀 + MAC 后 3 字节（大写 hex）
    char ssid[40];
    snprintf(ssid, sizeof(ssid), "%s%02X%02X%02X",
             WEB_AP_SSID_PREFIX, mac[3], mac[4], mac[5]);

    // 3. 生成密码：MAC 后 3 字节（大写 hex），写入全局状态
    snprintf(app_state::wifi_ap_password, sizeof(app_state::wifi_ap_password),
             "%02X%02X%02X", mac[3], mac[4], mac[5]);
    // 标记密码就绪，供跨任务读取
    app_state::wifi_ap_password_ready.store(true);

    // 4. 启动 AP 模式
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, app_state::wifi_ap_password);

    // 5. 配置固定 IP（192.168.4.1）
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1),
                      IPAddress(192, 168, 4, 1),
                      IPAddress(255, 255, 255, 0));

    s_state = WifiState::AP_MODE;
}

void stop() {
    // 先标记密码无效，防止跨任务读到半清空的数据
    app_state::wifi_ap_password_ready.store(false);

    // 清空密码
    std::memset(app_state::wifi_ap_password, 0, sizeof(app_state::wifi_ap_password));

    // 断开 AP 并关闭 WiFi
    WiFi.softAPdisconnect(true);
    WiFi.mode(WIFI_OFF);

    // 检查是否有 STA 凭据，有则切换到 STA 模式
    char ssid[33] = {0};
    config_manager::get_wifi_ssid(ssid, sizeof(ssid));
    if (ssid[0] != '\0') {
        start_sta();
    } else {
        s_state = WifiState::OFF;
    }
}

bool start_sta() {
    char ssid[33] = {0};
    char password[65] = {0};
    config_manager::get_wifi_ssid(ssid, sizeof(ssid));
    config_manager::get_wifi_password(password, sizeof(password));

    if (ssid[0] == '\0') {
        s_state = WifiState::OFF;
        return false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    constexpr int max_attempts = 30; // 15s / 500ms
    for (int i = 0; i < max_attempts; i++) {
        if (WiFi.status() == WL_CONNECTED) {
            s_state = WifiState::STA_MODE;
            return true;
        }
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    s_state = WifiState::OFF;
    return false;
}

// ── 状态查询实现 ─────────────────────────────────────────────────────────────

WifiState get_state() {
    return s_state;
}

const char* get_current_password() {
    if (app_state::wifi_ap_password_ready.load()) {
        return app_state::wifi_ap_password;
    }
    return "";
}

} // namespace wifi_mgr
