#include "input_bridge.h"
#include "input/keys.h"
#include "settings_ui.h"
#include "screen_manager.h"
#include "view_manager.h"
#include "chart_view.h"
#include "../app/app_state.h"
#include "../power/psu_fsm.h"
#include <lvgl.h>

namespace {
    lv_obj_t* g_home = nullptr;

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

        if (ev->key_id == 0) {
            view_manager::view_manager_cycle(-1);
        } else if (ev->key_id == 2) {
            view_manager::view_manager_cycle(+1);
        } else if (ev->key_id == 1) {
            HomeView cur = view_manager::view_manager_get_current();
            if (cur != VIEW_DEFAULT) {
                chart_view::chart_view_cycle_window();
            }
        }

        delete ev;
    }
}

namespace ui_bridge {
    void input_bridge_attach_home(lv_obj_t* home) {
        g_home = home;
    }

    void input_handle_key(uint8_t key_id, const KeyState& state) {
        // ─ 设置页面处理 ─
        if (settings_ui::is_active()) {
            settings_ui::handle_key(key_id, state);
            return;
        }

        // ─ 双击 K2（id 1）→ 进入设置 ─
        if (state.event == KEY_DOUBLE_CLICK) {
            if (key_id == 1) {
                screen_manager_show_settings();
            }
            return;
        }

        // ─ PSU 电源按键（KEY_K2 = id 1）─
        if (key_id == 1) {
            // 长按 → 关机
            if (state.event == KEY_LONG) {
                app_state::psu_event_request.store(
                    static_cast<uint8_t>(EVT_KEY_LONG));
                return;
            }
            // 短按 → 开机
            if (state.event == KEY_SHORT) {
                app_state::psu_event_request.store(
                    static_cast<uint8_t>(EVT_KEY_SHORT));
                return;
            }
        }

        // ─ 短按：导航 ─
        if (state.event != KEY_SHORT) return;
        if (!g_home) return;

        auto* ev = new InputEvent{g_home, key_id};
        lv_async_call(do_key_event, ev);
    }
}
