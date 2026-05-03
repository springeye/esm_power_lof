/**
 * @file native_main_sim.cpp
 * @brief PC 原生 LVGL 模拟器主入口
 *
 * 仅在 BUILD_NATIVE 定义时编译。
 * 使用 SDL2 显示后端运行 LVGL UI，用于开发调试。
 */

#ifdef BUILD_NATIVE

#include <cstdio>
#include <cstdlib>
#include <chrono>
#include <thread>

// LVGL 和显示层
#include "display/tft_driver.h"
#include "display/lvgl_port.h"

// UI 初始化（LVGL Editor 生成）
extern "C" {
#include "../ui/lof_power_system.h"
}

// UI bridge
#include "ui_bridge/screen_manager.h"
#include "ui_bridge/data_bridge.h"
#include "ui_bridge/input_bridge.h"

// SDL 事件处理（定义在 tft_driver_native.cpp）
extern "C" bool native_sdl_poll_quit();
extern "C" void native_sdl_cleanup();

// ── 主函数 ───────────────────────────────────────────────────────────────────
int main() {
    printf("[Native] Starting LVGL Simulator...\n");

    // 1. 初始化 SDL2 显示
    tft_driver::init();

    // 2. 初始化 LVGL
    lvgl_port::init();

    // 3. 初始化 UI（LVGL Editor 生成的 UI）
    lof_power_system_init(NULL);

    // 4. 初始化 UI bridge
    ui_bridge::screen_manager_init(1500);
    ui_bridge::data_bridge_attach(ui_bridge::screen_manager_get_home());
    ui_bridge::data_bridge_init();
    ui_bridge::input_bridge_attach_home(ui_bridge::screen_manager_get_home());

    printf("[Native] UI initialized. Entering main loop...\n");
    printf("[Native] Press ESC or close window to exit.\n");

    // 5. 主循环
    auto last_time = std::chrono::steady_clock::now();

    while (true) {
        // 处理 SDL 事件（窗口关闭、键盘）
        if (native_sdl_poll_quit()) {
            printf("[Native] Quit signal received.\n");
            break;
        }

        // 推进 LVGL tick
        lvgl_port::tick_increment();

        // 处理 LVGL 任务
        lvgl_port::task_handler();

        // 控制帧率 ~30fps
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_time);
        if (elapsed.count() < 33) {
            std::this_thread::sleep_for(std::chrono::milliseconds(33 - elapsed.count()));
        }
        last_time = std::chrono::steady_clock::now();
    }

    // 6. 清理
    native_sdl_cleanup();
    printf("[Native] Simulator exited cleanly.\n");

    return 0;
}

#endif // BUILD_NATIVE
