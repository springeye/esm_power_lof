#include "web_server.h"
#include "html_page.h"
#include "../../include/app_config.h"
#include "../app/config_manager.h"
#include "../ota/ota_handler.h"
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

namespace web_server {
namespace {

    AsyncWebServer* g_server = nullptr;
    bool s_running = false;

} // namespace

void start() {
    if (s_running) return;

    g_server = new AsyncWebServer(WEB_SERVER_PORT);

    // GET / — 返回 Gzip 压缩的 HTML 页面
    g_server->on("/", HTTP_GET, [](AsyncWebServerRequest* req) {
        AsyncWebServerResponse* resp = req->beginResponse_P(
            200, "text/html", html_page_gz, html_page_gz_len);
        resp->addHeader("Content-Encoding", "gzip");
        req->send(resp);
    });

    // POST /api/wifi/config — 保存 WiFi 凭据到 NVS
    g_server->on("/api/wifi/config", HTTP_POST,
        [](AsyncWebServerRequest* req) {
            req->send(200, "application/json", "{\"ok\":true}");
        },
        nullptr,
        [](AsyncWebServerRequest* req, uint8_t* data, size_t len, size_t index, size_t total) {
            // 使用 ArduinoJson 解析 POST body
            JsonDocument doc;
            DeserializationError err = deserializeJson(doc, data, len);
            if (err) {
                req->send(400, "application/json", "{\"ok\":false,\"error\":\"invalid json\"}");
                return;
            }

            const char* ssid = doc["ssid"];
            const char* password = doc["password"];

            if (ssid && strlen(ssid) > 0 && strlen(ssid) <= 32
                && password && strlen(password) <= 64) {
                config_manager::set_wifi_ssid(ssid);
                config_manager::set_wifi_password(password);
                config_manager::save_to_nvs();
            }
        }
    );

    // GET /api/wifi/status — 返回已保存凭据状态（密码脱敏）
    g_server->on("/api/wifi/status", HTTP_GET, [](AsyncWebServerRequest* req) {
        char ssid_buf[33] = {0};
        char pwd_buf[65] = {0};
        config_manager::get_wifi_ssid(ssid_buf, sizeof(ssid_buf));
        config_manager::get_wifi_password(pwd_buf, sizeof(pwd_buf));

        String resp;
        if (ssid_buf[0] != '\0') {
            resp = "{\"saved\":true,\"ssid\":\"";
            resp += ssid_buf;
            resp += "\",\"password\":\"***\"}";
        } else {
            resp = "{\"saved\":false}";
        }
        req->send(200, "application/json", resp);
    });

    // POST /update — OTA 固件上传
    g_server->on("/update", HTTP_POST,
        [](AsyncWebServerRequest* req) {
            bool ok = (ota_handler::get_progress() == 100);
            req->send(ok ? 200 : 500, "application/json",
                      ok ? "{\"ok\":true}" : "{\"ok\":false,\"error\":\"OTA failed\"}");
        },
        [](AsyncWebServerRequest* req, const String& filename,
           size_t index, uint8_t* data, size_t len, bool final) {
            ota_handler::handle_upload(req, filename, index, data, len, final);
        }
    );

    g_server->begin();
    s_running = true;
}

void stop() {
    if (!s_running) return;

    g_server->end();
    delete g_server;
    g_server = nullptr;
    s_running = false;
}

bool is_running() {
    return s_running;
}

} // namespace web_server
