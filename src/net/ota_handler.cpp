#include "ota_handler.h"
#include "web_server.h"
#include "app_config.h"

#include <ESPAsyncWebServer.h>
#include <Update.h>
#include <Arduino.h>
#include <esp_task_wdt.h>

namespace {

bool ota_in_progress = false;

void handle_update(AsyncWebServerRequest* request) {
    if (!Update.hasError()) {
        request->send(200, "text/plain", "OK");
        delay(OTA_REBOOT_DELAY_MS);
        ESP.restart();
    } else {
        request->send(500, "text/plain", Update.errorString());
    }
}

void handle_upload(AsyncWebServerRequest* request, String filename, size_t index,
                   uint8_t* data, size_t len, bool final) {
    if (!index) {
        ota_in_progress = true;
        if (!Update.begin(UPDATE_SIZE_UNKNOWN, U_FLASH)) {
            Update.printError(Serial);
            return;
        }
    }

    if (len) {
        if (Update.write(data, len) != len) {
            Update.printError(Serial);
            Update.abort();
            ota_in_progress = false;
            return;
        }

        esp_task_wdt_reset();
        delay(OTA_WATCHDOG_FEED_MS);
    }

    if (final) {
        if (Update.end(true)) {
            ota_in_progress = false;
        } else {
            Update.printError(Serial);
            Update.abort();
            ota_in_progress = false;
        }
    }
}

} // namespace

namespace ota_handler {

void init() {
    ota_in_progress = false;
}

void setup_routes() {
    AsyncWebServer* server = web_server::get_server();
    if (!server) {
        return;
    }

    server->on("/update", HTTP_POST, handle_update, handle_upload);
}

} // namespace ota_handler
