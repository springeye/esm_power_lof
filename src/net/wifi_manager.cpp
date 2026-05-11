#include "wifi_manager.h"
#include "app_config.h"
#include "../app/config_manager.h"

#include <WiFi.h>
#include <esp_wifi.h>
#include <Arduino.h>

namespace {

char ap_password[9] = "";
char ap_ssid[33] = "";
wifi_manager::WifiStatus current_status = wifi_manager::WifiStatus::DISCONNECTED;
unsigned long connect_start_time = 0;
bool ap_started = false;

void generate_random_password() {
    const char charset[] = "ABCDEFGHJKLMNPQRSTUVWXYZabcdefghjkmnpqrstuvwxyz23456789";
    for (int i = 0; i < 8; i++) {
        ap_password[i] = charset[random(sizeof(charset) - 1)];
    }
    ap_password[8] = '\0';
}

void generate_ap_ssid() {
    uint8_t mac[6];
    esp_wifi_get_mac(WIFI_IF_STA, mac);
    snprintf(ap_ssid, sizeof(ap_ssid), "ESM-Power-%02X%02X", mac[4], mac[5]);
}

} // namespace

namespace wifi_manager {

void init() {
    WiFi.mode(WIFI_OFF);
    generate_ap_ssid();
    generate_random_password();
}

void start_ap() {
    if (ap_started) {
        return;
    }

    WiFi.mode(WIFI_AP);
    WiFi.softAP(ap_ssid, ap_password, WIFI_AP_CHANNEL, 0, WIFI_AP_MAX_CONN);

    IPAddress local_ip(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(local_ip, gateway, subnet);

    ap_started = true;
    current_status = WifiStatus::AP_MODE;
}

void stop() {
    if (ap_started) {
        WiFi.softAPdisconnect(true);
        ap_started = false;
    }
    WiFi.disconnect(true);
    WiFi.mode(WIFI_OFF);
    current_status = WifiStatus::DISCONNECTED;
}

bool connect_sta() {
    const char* ssid = config_manager::get_wifi_ssid();
    const char* password = config_manager::get_wifi_password();

    if (strlen(ssid) == 0) {
        return false;
    }

    if (ap_started) {
        WiFi.softAPdisconnect(true);
        ap_started = false;
    }

    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);

    connect_start_time = millis();
    current_status = WifiStatus::CONNECTING;

    return true;
}

const char* get_ap_password() {
    return ap_password;
}

const char* get_ap_ssid() {
    return ap_ssid;
}

WifiStatus get_status() {
    return current_status;
}

void loop() {
    if (current_status == WifiStatus::CONNECTING) {
        if (WiFi.status() == WL_CONNECTED) {
            current_status = WifiStatus::CONNECTED;
        } else if (millis() - connect_start_time > WIFI_CONNECT_TIMEOUT_MS) {
            WiFi.disconnect();
            current_status = WifiStatus::DISCONNECTED;
        }
    }
}

} // namespace wifi_manager
