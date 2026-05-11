# Learnings - WiFi Panel Integration

## 2026-05-11: 确认和修复

### 1. BOOL 开关联动已正确
`settings_ui.cpp:294-308` 中 `write_item_value()` 的 BOOL case 已正确联动：
- `item.get_bool == config_manager::get_web_mgmt_enabled` 通过函数指针比对识别 "Web 管理" 项
- ON → `wifi_mgr::start_ap()`，OFF → `wifi_mgr::stop()`

### 2. wifi_mgr::init() 缺失
`wifi_mgr::init()` 未在任何地方调用，已在 `tasks.cpp:start_all()` 中添加。
- `init()` 仅调用 `WiFi.mode(WIFI_OFF)` 设置初始状态，无其他依赖
- 应在 `start_ap()` 之前调用

### 3. AP 密码标签时序问题
`rebuild_page()` 在 SYSTEM 页创建密码标签，但 BOOL toggle 后（同一页内）标签不更新。
- 修复：存储 `g_system_pwd_label` 指针，在 `enter_edit_mode` 的 BOOL toggle 后刷新
- `lv_obj_clean(g_content_area)` 会销毁旧标签，但页面切换时 rebuild_page 会重新创建
