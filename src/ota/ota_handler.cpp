#include "ota_handler.h"
#include "../app/app_state.h"
#include "../../include/app_config.h"
#include <Update.h>
#include <ESPAsyncWebServer.h>
#include <Arduino.h>

namespace ota_handler {
namespace {
    size_t s_written        = 0;
    size_t s_total          = 0;
    bool   s_error          = false;
    size_t s_last_report_kb = 0;
}

void handle_upload(AsyncWebServerRequest* request,
                   const String& /*filename*/,
                   size_t index,
                   uint8_t* data,
                   size_t len,
                   bool final) {
    if (index == 0) {
        // 首块：校验魔数（ESP32 固件头第一字节固定为 0xE9）
        if (len > 0 && data[0] != 0xE9) {
            s_error = true;
            request->send(400, "text/plain", "Invalid firmware");
            return;
        }
        s_written        = 0;
        s_total          = request->contentLength();
        s_error          = false;
        s_last_report_kb = 0;
        app_state::set_ota_progress(0);
        if (!Update.begin(s_total > 0 ? s_total : OTA_MAX_SIZE)) {
            s_error = true;
            request->send(500, "text/plain", "OTA begin failed");
            return;
        }
    }

    if (s_error) return;

    if (Update.write(data, len) != len) {
        s_error = true;
        Update.abort();
        request->send(500, "text/plain", "OTA write failed");
        return;
    }
    s_written += len;

    // 每 64KB 更新一次进度，避免过于频繁的原子写入
    size_t written_kb = s_written / (64 * 1024);
    if (written_kb > s_last_report_kb) {
        s_last_report_kb = written_kb;
        int8_t pct = (s_total > 0) ? (int8_t)((s_written * 100) / s_total) : 50;
        app_state::set_ota_progress(pct);
        yield(); // 让出 CPU，避免阻塞 WDT
    }

    if (final) {
        if (Update.end(true)) {
            app_state::set_ota_progress(100);
            request->send(200, "text/plain", "OTA OK");
            delay(500);
            ESP.restart();
        } else {
            s_error = true;
            app_state::set_ota_progress(-1);
            request->send(500, "text/plain", "OTA end failed");
        }
    }
}

int8_t get_progress() {
    return app_state::get_ota_progress();
}

} // namespace ota_handler
