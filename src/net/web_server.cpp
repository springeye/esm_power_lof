#include "web_server.h"
#include "app_config.h"
#include "wifi_manager.h"
#include "ota_handler.h"
#include "../app/config_manager.h"

#include <ESPAsyncWebServer.h>
#include <Arduino.h>

namespace {

AsyncWebServer* server = nullptr;
bool server_started = false;

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESM Power System</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 50%, #0f3460 100%);
            min-height: 100vh;
            padding: 20px;
            color: #fff;
        }
        .container { max-width: 480px; margin: 0 auto; }
        .header {
            text-align: center;
            padding: 30px 0;
        }
        .header h1 {
            font-size: 24px;
            font-weight: 600;
            background: linear-gradient(90deg, #00d2ff, #3a7bd5);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
            margin-bottom: 8px;
        }
        .status {
            display: inline-flex;
            align-items: center;
            gap: 8px;
            padding: 6px 12px;
            background: rgba(255,255,255,0.1);
            border-radius: 20px;
            font-size: 14px;
        }
        .status-dot {
            width: 8px;
            height: 8px;
            border-radius: 50%;
            background: #00ff88;
            animation: pulse 2s infinite;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        .card {
            background: rgba(255,255,255,0.05);
            backdrop-filter: blur(10px);
            border-radius: 16px;
            padding: 24px;
            margin-bottom: 20px;
            border: 1px solid rgba(255,255,255,0.1);
        }
        .card-title {
            display: flex;
            align-items: center;
            gap: 10px;
            font-size: 18px;
            font-weight: 500;
            margin-bottom: 20px;
            color: #00d2ff;
        }
        .form-group {
            margin-bottom: 16px;
        }
        .form-group label {
            display: block;
            font-size: 14px;
            color: rgba(255,255,255,0.7);
            margin-bottom: 8px;
        }
        .input-wrapper {
            position: relative;
        }
        .input-wrapper input {
            width: 100%;
            padding: 14px 16px;
            padding-left: 44px;
            background: rgba(0,0,0,0.3);
            border: 1px solid rgba(255,255,255,0.1);
            border-radius: 12px;
            color: #fff;
            font-size: 16px;
            transition: all 0.3s;
        }
        .input-wrapper input:focus {
            outline: none;
            border-color: #00d2ff;
            box-shadow: 0 0 0 3px rgba(0,210,255,0.1);
        }
        .input-icon {
            position: absolute;
            left: 14px;
            top: 50%;
            transform: translateY(-50%);
            font-size: 18px;
        }
        .toggle-password {
            position: absolute;
            right: 14px;
            top: 50%;
            transform: translateY(-50%);
            background: none;
            border: none;
            color: rgba(255,255,255,0.5);
            cursor: pointer;
            font-size: 18px;
        }
        .btn {
            width: 100%;
            padding: 14px;
            border: none;
            border-radius: 12px;
            font-size: 16px;
            font-weight: 500;
            cursor: pointer;
            transition: all 0.3s;
            background: linear-gradient(135deg, #00d2ff 0%, #3a7bd5 100%);
            color: #fff;
        }
        .btn:hover {
            transform: translateY(-2px);
            box-shadow: 0 8px 20px rgba(0,210,255,0.3);
        }
        .btn:disabled {
            opacity: 0.5;
            cursor: not-allowed;
            transform: none;
        }
        .upload-zone {
            border: 2px dashed rgba(255,255,255,0.2);
            border-radius: 12px;
            padding: 40px 20px;
            text-align: center;
            transition: all 0.3s;
            cursor: pointer;
        }
        .upload-zone:hover, .upload-zone.dragover {
            border-color: #00d2ff;
            background: rgba(0,210,255,0.05);
        }
        .upload-zone input { display: none; }
        .upload-icon { font-size: 48px; margin-bottom: 12px; }
        .upload-text { color: rgba(255,255,255,0.7); }
        .file-name {
            margin-top: 12px;
            padding: 10px;
            background: rgba(0,210,255,0.1);
            border-radius: 8px;
            font-size: 14px;
            word-break: break-all;
        }
        .progress-bar {
            height: 8px;
            background: rgba(255,255,255,0.1);
            border-radius: 4px;
            margin-top: 16px;
            overflow: hidden;
        }
        .progress-fill {
            height: 100%;
            background: linear-gradient(90deg, #00d2ff, #3a7bd5);
            border-radius: 4px;
            transition: width 0.3s;
            width: 0%;
        }
        .progress-text {
            text-align: center;
            margin-top: 8px;
            font-size: 14px;
            color: rgba(255,255,255,0.7);
        }
        .toast {
            position: fixed;
            bottom: 20px;
            left: 50%;
            transform: translateX(-50%);
            padding: 14px 24px;
            border-radius: 12px;
            font-size: 14px;
            opacity: 0;
            transition: opacity 0.3s;
            z-index: 1000;
        }
        .toast.success { background: #00c853; }
        .toast.error { background: #ff5252; }
        .toast.show { opacity: 1; }
        .footer {
            text-align: center;
            padding: 20px;
            font-size: 12px;
            color: rgba(255,255,255,0.4);
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>ESM Power System</h1>
            <div class="status">
                <div class="status-dot"></div>
                <span id="status-text">在线</span>
            </div>
        </div>

        <div class="card">
            <div class="card-title">
                <span>📶</span>
                <span>WiFi 配置</span>
            </div>
            <form id="wifi-form">
                <div class="form-group">
                    <label>网络名称 (SSID)</label>
                    <div class="input-wrapper">
                        <span class="input-icon">📡</span>
                        <input type="text" id="ssid" name="ssid" placeholder="输入WiFi名称" required>
                    </div>
                </div>
                <div class="form-group">
                    <label>密码</label>
                    <div class="input-wrapper">
                        <span class="input-icon">🔐</span>
                        <input type="password" id="password" name="password" placeholder="输入WiFi密码">
                        <button type="button" class="toggle-password" onclick="togglePassword()">👁</button>
                    </div>
                </div>
                <button type="submit" class="btn">保存配置</button>
            </form>
        </div>

        <div class="card">
            <div class="card-title">
                <span>⬆️</span>
                <span>固件更新</span>
            </div>
            <div class="upload-zone" id="upload-zone">
                <input type="file" id="firmware-file" accept=".bin">
                <div class="upload-icon">📦</div>
                <div class="upload-text">点击或拖拽固件文件到此处</div>
                <div class="file-name" id="file-name" style="display:none;"></div>
            </div>
            <div class="progress-bar" id="progress-bar" style="display:none;">
                <div class="progress-fill" id="progress-fill"></div>
            </div>
            <div class="progress-text" id="progress-text" style="display:none;"></div>
            <button type="button" class="btn" id="upload-btn" style="margin-top:16px;" disabled>开始更新</button>
        </div>

        <div class="footer">
            ESM Power System v1.0
        </div>
    </div>

    <div class="toast" id="toast"></div>

    <script>
        function togglePassword() {
            const input = document.getElementById('password');
            input.type = input.type === 'password' ? 'text' : 'password';
        }

        function showToast(message, type) {
            const toast = document.getElementById('toast');
            toast.textContent = message;
            toast.className = 'toast ' + type + ' show';
            setTimeout(() => toast.className = 'toast', 3000);
        }

        document.getElementById('wifi-form').addEventListener('submit', async (e) => {
            e.preventDefault();
            const ssid = document.getElementById('ssid').value;
            const password = document.getElementById('password').value;

            try {
                const response = await fetch('/wifi', {
                    method: 'POST',
                    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },
                    body: 'ssid=' + encodeURIComponent(ssid) + '&password=' + encodeURIComponent(password)
                });

                if (response.ok) {
                    showToast('WiFi配置已保存', 'success');
                } else {
                    showToast('保存失败', 'error');
                }
            } catch (err) {
                showToast('网络错误', 'error');
            }
        });

        const uploadZone = document.getElementById('upload-zone');
        const fileInput = document.getElementById('firmware-file');
        const uploadBtn = document.getElementById('upload-btn');
        const fileName = document.getElementById('file-name');
        const progressBar = document.getElementById('progress-bar');
        const progressFill = document.getElementById('progress-fill');
        const progressText = document.getElementById('progress-text');

        uploadZone.addEventListener('click', () => fileInput.click());
        uploadZone.addEventListener('dragover', (e) => {
            e.preventDefault();
            uploadZone.classList.add('dragover');
        });
        uploadZone.addEventListener('dragleave', () => uploadZone.classList.remove('dragover'));
        uploadZone.addEventListener('drop', (e) => {
            e.preventDefault();
            uploadZone.classList.remove('dragover');
            if (e.dataTransfer.files.length) {
                fileInput.files = e.dataTransfer.files;
                fileInput.dispatchEvent(new Event('change'));
            }
        });

        fileInput.addEventListener('change', () => {
            if (fileInput.files.length) {
                fileName.textContent = fileInput.files[0].name;
                fileName.style.display = 'block';
                uploadBtn.disabled = false;
            }
        });

        uploadBtn.addEventListener('click', async () => {
            if (!fileInput.files.length) return;

            const formData = new FormData();
            formData.append('firmware', fileInput.files[0]);

            progressBar.style.display = 'block';
            progressText.style.display = 'block';
            uploadBtn.disabled = true;

            try {
                const xhr = new XMLHttpRequest();
                xhr.open('POST', '/update', true);

                xhr.upload.onprogress = (e) => {
                    if (e.lengthComputable) {
                        const percent = Math.round((e.loaded / e.total) * 100);
                        progressFill.style.width = percent + '%';
                        progressText.textContent = percent + '%';
                    }
                };

                xhr.onload = () => {
                    if (xhr.status === 200) {
                        showToast('固件更新成功，设备即将重启', 'success');
                        setTimeout(() => location.reload(), 3000);
                    } else {
                        showToast('更新失败: ' + xhr.responseText, 'error');
                        uploadBtn.disabled = false;
                    }
                };

                xhr.onerror = () => {
                    showToast('网络错误', 'error');
                    uploadBtn.disabled = false;
                };

                xhr.send(formData);
            } catch (err) {
                showToast('上传失败', 'error');
                uploadBtn.disabled = false;
            }
        });
    </script>
</body>
</html>
)rawliteral";

void handle_root(AsyncWebServerRequest* request) {
    request->send_P(200, "text/html", index_html);
}

void handle_wifi(AsyncWebServerRequest* request) {
    if (request->hasParam("ssid", true)) {
        String ssid = request->getParam("ssid", true)->value();
        String password = request->hasParam("password", true) ?
            request->getParam("password", true)->value() : "";

        config_manager::set_wifi_ssid(ssid.c_str());
        config_manager::set_wifi_password(password.c_str());

        request->send(200, "text/plain", "OK");
    } else {
        request->send(400, "text/plain", "Missing SSID");
    }
}

void handle_status(AsyncWebServerRequest* request) {
    String json = "{\"status\":\"online\",\"ssid\":\"";
    json += config_manager::get_wifi_ssid();
    json += "\"}";
    request->send(200, "application/json", json);
}

} // namespace

namespace web_server {

void init() {
    server = new AsyncWebServer(WEB_SERVER_PORT);
}

void start() {
    if (server_started || !server) {
        return;
    }

    server->on("/", HTTP_GET, handle_root);
    server->on("/wifi", HTTP_POST, handle_wifi);
    server->on("/status", HTTP_GET, handle_status);

    ota_handler::setup_routes();

    server->begin();
    server_started = true;
}

void stop() {
    if (server && server_started) {
        server->end();
        server_started = false;
    }
}

AsyncWebServer* get_server() {
    return server;
}

} // namespace web_server
