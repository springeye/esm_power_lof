#include "input_bridge.h"
#include "input/keys.h"
#include <lvgl.h>

namespace {
    lv_obj_t* g_home = nullptr;

    // 可聚焦控件索引（来自 T10 分析，按需扩展）
    // 当前简化实现：KEY_UP/DOWN 在 home 子对象间循环聚焦
    // KEY_ENTER 对当前聚焦对象发送 LV_EVENT_CLICKED
    static int g_focus_idx = 0;
    static const int FOCUSABLE_COUNT = 3;  // CH1/CH2/CH3 电流区

    lv_obj_t* get_focusable(int idx) {
        if (!g_home) return nullptr;
        // 电流区：home->child(5)->child(0/1/2)
        lv_obj_t* current_area = lv_obj_get_child(g_home, 5);
        if (!current_area) return nullptr;
        return lv_obj_get_child(current_area, idx);
    }

    struct InputEvent {
        lv_obj_t* home;
        uint8_t key_id;
    };

    void do_key_event(void* user_data) {
        auto* ev = static_cast<InputEvent*>(user_data);
        if (!ev) return;
        if (ev->home != g_home || !g_home) {
            delete ev;
            return;
        }

        lv_obj_t* prev = get_focusable(g_focus_idx);

        if (ev->key_id == 0) {
            // KEY_UP: 向上循环
            g_focus_idx = (g_focus_idx - 1 + FOCUSABLE_COUNT) % FOCUSABLE_COUNT;
        } else if (ev->key_id == 2) {
            // KEY_DOWN: 向下循环
            g_focus_idx = (g_focus_idx + 1) % FOCUSABLE_COUNT;
        } else if (ev->key_id == 1) {
            // KEY_ENTER: 发送点击事件
            lv_obj_t* target = get_focusable(g_focus_idx);
            if (target) {
                lv_obj_send_event(target, LV_EVENT_CLICKED, nullptr);
            }
            delete ev;
            return;
        }

        // 更新焦点状态
        if (prev) lv_obj_clear_state(prev, LV_STATE_FOCUSED);
        lv_obj_t* next = get_focusable(g_focus_idx);
        if (next) lv_obj_add_state(next, LV_STATE_FOCUSED);

        delete ev;
    }
}

namespace ui_bridge {
    void input_bridge_attach_home(lv_obj_t* home) {
        g_home = home;
        g_focus_idx = 0;
    }

    void input_handle_key(uint8_t key_id, const KeyState& state) {
        // 仅响应短按（KEY_SHORT），忽略 KEY_IDLE / KEY_LONG
        if (state.event != KEY_SHORT) return;
        if (!g_home) return;

        // input_task 在 Core 0 调用本函数，LVGL 在 Core 1 运行；
        // 通过 lv_async_call 把所有 LVGL 操作派发到 LVGL 线程，
        // 避免跨核访问 LVGL 内部状态导致的数据竞争。
        auto* ev = new InputEvent{g_home, key_id};
        lv_async_call(do_key_event, ev);
    }
}
