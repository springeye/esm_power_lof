# Issues

## 已解决

### html_page.h 数组格式错误（已修复）
- 第一次生成时字节间用空格而非逗号分隔
- 已用 PowerShell 脚本重新生成正确格式

### header 缺少 include（已修复）
- `html_page.h` 缺 `#include <cstdint>`
- `ota_handler.h` 缺 `#include <WString.h>`

## 待注意

### Task 7 STA 状态机注意事项
- STA 连接失败不得触发故障报警
- STA 模式下不启动 Web 服务
- 关闭 web_mgmt 开关时，若有 NVS 凭据则自动连接路由器

### Task 11 Web API 路由注意事项
- 不引入 ArduinoJson，手动解析 JSON
- POST /api/wifi 接收 ssid+password，存入 NVS
- GET /api/status 返回当前状态 JSON
