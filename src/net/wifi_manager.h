#pragma once

#include <cstdint>

namespace wifi_manager {

enum class WifiStatus {
    DISCONNECTED,
    CONNECTING,
    CONNECTED,
    AP_MODE
};

void init();
void start_ap();
void stop();
bool connect_sta();
const char* get_ap_password();
const char* get_ap_ssid();
WifiStatus get_status();
void loop();

} // namespace wifi_manager
