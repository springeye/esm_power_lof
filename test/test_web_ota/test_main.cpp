/**
 * @file test/test_web_ota/test_main.cpp
 * @brief Web OTA 功能 Unity 测试用例
 *
 * 测试范围：
 *   - WifiConfig 结构体字段默认值、容量限制、边界行为
 *   - WifiState 枚举值正确性
 *   - ota_progress 原子变量读写（初始值、范围 0-100、重置）
 *   - config_manager Wi-Fi getter/setter（纯逻辑，不依赖硬件）
 *
 * 约定：
 *   - Unity 框架（<unity.h>）
 *   - setUp()/tearDown() 即使为空也实现
 *   - 使用 RUN_TEST() 注册用例
 *   - 不依赖真实 WiFi 硬件
 */

#include <unity.h>
#include <cstring>

#include "app/config_manager.h"
#include "app/app_state.h"
#include "wifi/wifi_manager.h"

// ═══════════════════════════════════════════════════════════════════════════════
// setUp / tearDown
// ═══════════════════════════════════════════════════════════════════════════════

void setUp(void) {
    // 每个测试前执行（本测试套件中暂无需特殊初始化）
}

void tearDown(void) {
    // 每个测试后执行（本测试套件中暂无需特殊清理）
}

// ═══════════════════════════════════════════════════════════════════════════════
// 1. WifiConfig 结构体测试
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 验证 WifiConfig 零初始化后的默认值
 */
void test_wifi_config_default_construction(void) {
    config_manager::WifiConfig cfg{};
    TEST_ASSERT_EQUAL_STRING("", cfg.ssid);
    TEST_ASSERT_EQUAL_STRING("", cfg.password);
    TEST_ASSERT_FALSE(cfg.web_mgmt_enabled);
}

/**
 * @brief 验证 SSID 缓冲区可容纳 32 字符 + null 终止符
 */
void test_wifi_config_ssid_capacity(void) {
    config_manager::WifiConfig cfg{};
    const char* max_ssid = "12345678901234567890123456789012";  // 32 chars
    std::strncpy(cfg.ssid, max_ssid, sizeof(cfg.ssid) - 1);
    cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';
    TEST_ASSERT_EQUAL(32, std::strlen(cfg.ssid));
    TEST_ASSERT_EQUAL_STRING(max_ssid, cfg.ssid);
}

/**
 * @brief 验证 SSID 数组大小 = 33（32 字符 + \0），防止溢出
 */
void test_wifi_config_ssid_buffer_size(void) {
    config_manager::WifiConfig cfg{};
    TEST_ASSERT_EQUAL(33, sizeof(cfg.ssid));
}

/**
 * @brief 验证密码缓冲区可容纳 64 字符 + null 终止符
 */
void test_wifi_config_password_capacity(void) {
    config_manager::WifiConfig cfg{};
    const char* max_pwd =
        "12345678901234567890123456789012"
        "34567890123456789012345678901234";  // 64 chars
    std::strncpy(cfg.password, max_pwd, sizeof(cfg.password) - 1);
    cfg.password[sizeof(cfg.password) - 1] = '\0';
    TEST_ASSERT_EQUAL(64, std::strlen(cfg.password));
    TEST_ASSERT_EQUAL_STRING(max_pwd, cfg.password);
}

/**
 * @brief 验证密码数组大小 = 65（64 字符 + \0），防止溢出
 */
void test_wifi_config_password_buffer_size(void) {
    config_manager::WifiConfig cfg{};
    TEST_ASSERT_EQUAL(65, sizeof(cfg.password));
}

/**
 * @brief 验证 web_mgmt_enabled 字段的读写行为
 */
void test_wifi_config_web_mgmt_field_rw(void) {
    config_manager::WifiConfig cfg{};
    TEST_ASSERT_FALSE(cfg.web_mgmt_enabled);

    cfg.web_mgmt_enabled = true;
    TEST_ASSERT_TRUE(cfg.web_mgmt_enabled);

    cfg.web_mgmt_enabled = false;
    TEST_ASSERT_FALSE(cfg.web_mgmt_enabled);
}

// ═══════════════════════════════════════════════════════════════════════════════
// 2. WifiState 枚举测试
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 验证 WifiState 枚举基础值
 *        OFF=0, AP_MODE=1, STA_MODE=2（与 design/interface 一致）
 */
void test_wifi_state_enum_values(void) {
    TEST_ASSERT_EQUAL_UINT8(0, static_cast<uint8_t>(wifi_mgr::WifiState::OFF));
    TEST_ASSERT_EQUAL_UINT8(1, static_cast<uint8_t>(wifi_mgr::WifiState::AP_MODE));
    TEST_ASSERT_EQUAL_UINT8(2, static_cast<uint8_t>(wifi_mgr::WifiState::STA_MODE));
}

// ═══════════════════════════════════════════════════════════════════════════════
// 3. ota_progress 原子变量测试
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 验证 ota_progress 初始值 = -1（无 OTA）
 */
void test_ota_progress_initial_value(void) {
    // app_state.cpp: ota_progress{-1}
    TEST_ASSERT_EQUAL_INT8(-1, app_state::get_ota_progress());
}

/**
 * @brief 验证 ota_progress 可设置为 0（OTA 开始）
 */
void test_ota_progress_set_zero(void) {
    app_state::set_ota_progress(0);
    TEST_ASSERT_EQUAL_INT8(0, app_state::get_ota_progress());
}

/**
 * @brief 验证 ota_progress 可设置为中间值 50（百分比）
 */
void test_ota_progress_set_mid(void) {
    app_state::set_ota_progress(50);
    TEST_ASSERT_EQUAL_INT8(50, app_state::get_ota_progress());
}

/**
 * @brief 验证 ota_progress 可设置为 100（OTA 完成）
 */
void test_ota_progress_set_full(void) {
    app_state::set_ota_progress(100);
    TEST_ASSERT_EQUAL_INT8(100, app_state::get_ota_progress());
}

/**
 * @brief 验证 ota_progress 可从 100 重置为 -1
 */
void test_ota_progress_reset_to_no_ota(void) {
    app_state::set_ota_progress(100);
    TEST_ASSERT_EQUAL_INT8(100, app_state::get_ota_progress());
    app_state::set_ota_progress(-1);
    TEST_ASSERT_EQUAL_INT8(-1, app_state::get_ota_progress());
}

/**
 * @brief 验证 ota_progress 边界值 0 和 100
 */
void test_ota_progress_boundary_values(void) {
    app_state::set_ota_progress(0);
    TEST_ASSERT_EQUAL_INT8(0, app_state::get_ota_progress());

    app_state::set_ota_progress(100);
    TEST_ASSERT_EQUAL_INT8(100, app_state::get_ota_progress());
}

// ═══════════════════════════════════════════════════════════════════════════════
// 4. config_manager Wi-Fi getter/setter 测试（纯逻辑，不依赖 NVS 硬件）
// ═══════════════════════════════════════════════════════════════════════════════

/**
 * @brief 验证 init 后 web_mgmt_enabled 默认值为 false
 */
void test_config_web_mgmt_default_after_init(void) {
    config_manager::init();
    TEST_ASSERT_FALSE(config_manager::get_web_mgmt_enabled());
}

/**
 * @brief 验证 set_web_mgmt_enabled(true) → get 返回 true
 */
void test_config_web_mgmt_set_true(void) {
    config_manager::set_web_mgmt_enabled(true);
    TEST_ASSERT_TRUE(config_manager::get_web_mgmt_enabled());
}

/**
 * @brief 验证 set_web_mgmt_enabled(false) → get 返回 false
 */
void test_config_web_mgmt_set_false(void) {
    config_manager::set_web_mgmt_enabled(true);
    TEST_ASSERT_TRUE(config_manager::get_web_mgmt_enabled());
    config_manager::set_web_mgmt_enabled(false);
    TEST_ASSERT_FALSE(config_manager::get_web_mgmt_enabled());
}

/**
 * @brief 验证 init 后 SSID 默认值为空字符串
 */
void test_config_wifi_ssid_default_empty(void) {
    char buf[33] = {0};
    config_manager::get_wifi_ssid(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("", buf);
}

/**
 * @brief 验证 set_wifi_ssid → get_wifi_ssid 正常读写
 */
void test_config_wifi_ssid_set_and_get(void) {
    const char* test_ssid = "MyTestWiFi";
    config_manager::set_wifi_ssid(test_ssid);
    char buf[33] = {0};
    config_manager::get_wifi_ssid(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING(test_ssid, buf);
}

/**
 * @brief 验证 set_wifi_ssid 最大长度 32 字符
 */
void test_config_wifi_ssid_max_length(void) {
    const char* max_ssid = "12345678901234567890123456789012";  // 32 chars
    config_manager::set_wifi_ssid(max_ssid);
    char buf[33] = {0};
    config_manager::get_wifi_ssid(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING(max_ssid, buf);
}

/**
 * @brief 验证 set_wifi_ssid 超过 32 字符时被截断
 */
void test_config_wifi_ssid_overflow_truncation(void) {
    const char* overflow =
        "1234567890123456789012345678901234567890";  // 40 chars
    config_manager::set_wifi_ssid(overflow);
    char buf[33] = {0};
    config_manager::get_wifi_ssid(buf, sizeof(buf));
    TEST_ASSERT_EQUAL(32, std::strlen(buf));
    TEST_ASSERT_EQUAL_STRING(
        "12345678901234567890123456789012", buf);  // truncated to 32
}

/**
 * @brief 验证 set_wifi_ssid(nullptr) 不崩溃且不改变已有值
 */
void test_config_wifi_ssid_null_pointer_set(void) {
    const char* valid_ssid = "ValidSSID";
    config_manager::set_wifi_ssid(valid_ssid);
    // 传递 nullptr 不应崩溃，也不应改变已有 SSID
    config_manager::set_wifi_ssid(nullptr);
    char buf[33] = {0};
    config_manager::get_wifi_ssid(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING(valid_ssid, buf);
}

/**
 * @brief 验证 get_wifi_ssid 缓冲区 size=1 时安全处理（仅写入 \0）
 */
void test_config_wifi_ssid_buffer_size_one(void) {
    config_manager::set_wifi_ssid("TestSSID");
    char buf[1] = {0xFF};
    config_manager::get_wifi_ssid(buf, sizeof(buf));
    // n=1, buf[0] = '\0' (strncpy with n-1=0 copies nothing, then buf[0]='\0')
    TEST_ASSERT_EQUAL_STRING("", buf);
}

/**
 * @brief 验证 get_wifi_ssid(nullptr, ...) 不崩溃
 */
void test_config_wifi_ssid_null_buffer_get(void) {
    // 传递 nullptr 缓冲区不应崩溃
    config_manager::get_wifi_ssid(nullptr, 0);
    config_manager::get_wifi_ssid(nullptr, 33);
    // 执行到这里没有崩溃即通过
    TEST_ASSERT_TRUE(true);
}

/**
 * @brief 验证 init 后密码默认值为空字符串
 */
void test_config_wifi_password_default_empty(void) {
    char buf[65] = {0};
    config_manager::get_wifi_password(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("", buf);
}

/**
 * @brief 验证 set_wifi_password → get_wifi_password 正常读写
 */
void test_config_wifi_password_set_and_get(void) {
    const char* test_pwd = "MySecurePassword123";
    config_manager::set_wifi_password(test_pwd);
    char buf[65] = {0};
    config_manager::get_wifi_password(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING(test_pwd, buf);
}

/**
 * @brief 验证 set_wifi_password 最大长度 64 字符
 */
void test_config_wifi_password_max_length(void) {
    const char* max_pwd =
        "12345678901234567890123456789012"
        "34567890123456789012345678901234";  // 64 chars
    config_manager::set_wifi_password(max_pwd);
    char buf[65] = {0};
    config_manager::get_wifi_password(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING(max_pwd, buf);
}

/**
 * @brief 验证 set_wifi_password 超过 64 字符时被截断
 */
void test_config_wifi_password_overflow_truncation(void) {
    const char* overflow =
        "12345678901234567890123456789012"
        "34567890123456789012345678901234567890";  // 70 chars
    config_manager::set_wifi_password(overflow);
    char buf[65] = {0};
    config_manager::get_wifi_password(buf, sizeof(buf));
    TEST_ASSERT_EQUAL(64, std::strlen(buf));
}

/**
 * @brief 验证 set_wifi_password(nullptr) 不崩溃且不改变已有值
 */
void test_config_wifi_password_null_pointer_set(void) {
    const char* valid_pwd = "ValidPassword";
    config_manager::set_wifi_password(valid_pwd);
    // 传递 nullptr 不应崩溃，也不应改变已有密码
    config_manager::set_wifi_password(nullptr);
    char buf[65] = {0};
    config_manager::get_wifi_password(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING(valid_pwd, buf);
}

/**
 * @brief 验证 get_wifi_password 缓冲区 size=1 时安全处理
 */
void test_config_wifi_password_buffer_size_one(void) {
    config_manager::set_wifi_password("TestPwd");
    char buf[1] = {0xFF};
    config_manager::get_wifi_password(buf, sizeof(buf));
    TEST_ASSERT_EQUAL_STRING("", buf);
}

/**
 * @brief 验证 get_wifi_password(nullptr, ...) 不崩溃
 */
void test_config_wifi_password_null_buffer_get(void) {
    // 传递 nullptr 缓冲区不应崩溃
    config_manager::get_wifi_password(nullptr, 0);
    config_manager::get_wifi_password(nullptr, 65);
    // 执行到这里没有崩溃即通过
    TEST_ASSERT_TRUE(true);
}

// ═══════════════════════════════════════════════════════════════════════════════
// setup / loop（PlatformIO Unity 入口）
// ═══════════════════════════════════════════════════════════════════════════════

void setup() {
    UNITY_BEGIN();

    // ── 1. WifiConfig 结构体测试（纯内存操作，无硬件依赖） ──
    RUN_TEST(test_wifi_config_default_construction);
    RUN_TEST(test_wifi_config_ssid_capacity);
    RUN_TEST(test_wifi_config_ssid_buffer_size);
    RUN_TEST(test_wifi_config_password_capacity);
    RUN_TEST(test_wifi_config_password_buffer_size);
    RUN_TEST(test_wifi_config_web_mgmt_field_rw);

    // ── 2. WifiState 枚举测试 ──
    RUN_TEST(test_wifi_state_enum_values);

    // ── 3. ota_progress 原子变量测试（纯内存操作） ──
    RUN_TEST(test_ota_progress_initial_value);
    RUN_TEST(test_ota_progress_set_zero);
    RUN_TEST(test_ota_progress_set_mid);
    RUN_TEST(test_ota_progress_set_full);
    RUN_TEST(test_ota_progress_reset_to_no_ota);
    RUN_TEST(test_ota_progress_boundary_values);

    // ── 4. config_manager Wi-Fi getter/setter 集成测试 ──
    //     config_manager::init() 内部会尝试 NVS 加载；
    //     若无硬件（NVS 分区不可用），prefs.begin() 静默失败，
    //     set_defaults_locked() 设置的默认值保留不变，
    //     后续 getter/setter 的内存操作不受影响。
    config_manager::init();

    RUN_TEST(test_config_web_mgmt_default_after_init);
    RUN_TEST(test_config_web_mgmt_set_true);
    RUN_TEST(test_config_web_mgmt_set_false);

    RUN_TEST(test_config_wifi_ssid_default_empty);
    RUN_TEST(test_config_wifi_ssid_set_and_get);
    RUN_TEST(test_config_wifi_ssid_max_length);
    RUN_TEST(test_config_wifi_ssid_overflow_truncation);
    RUN_TEST(test_config_wifi_ssid_null_pointer_set);
    RUN_TEST(test_config_wifi_ssid_buffer_size_one);
    RUN_TEST(test_config_wifi_ssid_null_buffer_get);

    RUN_TEST(test_config_wifi_password_default_empty);
    RUN_TEST(test_config_wifi_password_set_and_get);
    RUN_TEST(test_config_wifi_password_max_length);
    RUN_TEST(test_config_wifi_password_overflow_truncation);
    RUN_TEST(test_config_wifi_password_null_pointer_set);
    RUN_TEST(test_config_wifi_password_buffer_size_one);
    RUN_TEST(test_config_wifi_password_null_buffer_get);

    UNITY_END();
}

void loop() {
    // PlatformIO Unity 测试结束后 loop 空闲运行
}
