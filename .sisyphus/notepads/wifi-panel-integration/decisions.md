# Decisions - WiFi Panel Integration

## 2026-05-11

### wifi_mgr::init() 放置位置
- **决策：放在 `tasks::start_all()` 开头**
- **理由：** tasks.cpp 是 FreeRTOS 任务管理中心，所有模块初始化在此或 main.cpp 中完成。main.cpp 中已有较多外设初始化，将 WiFi 初始化放在 `start_all()` 开头更集中。
- **备选：** main.cpp 的 `setup()` 中 `config_manager::init()` 之后

### 密码标签刷新方式
- **决策：** 使用静态指针 `g_system_pwd_label` + 在 `enter_edit_mode` BOOL 分支中刷新
- **理由：** 
  1. 避免整个页面重建（`rebuild_page()` 会清除所有 row 状态）
  2. `enter_edit_mode` 是 BOOL toggle 的唯一入口（K2 短按）
  3. 只有在 SYSTEM 页时 `g_system_pwd_label` 非空，安全
- **备选：** 在 `write_item_value()` 中刷新（但该函数是通用写入函数，不应耦合 UI 逻辑）
