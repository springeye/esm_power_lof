#include "input_bridge.h"
#include "input/keys.h"
#include "settings_ui.h"
#include "view_manager.h"
#include "chart_view.h"
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
        if (settings_ui::is_active()) {
            settings_ui::handle_key(key_id, state);
            return;
        }

        if (state.event == KEY_LONG && key_id == 1) {
            settings_ui::show();
            return;
        }

        if (state.event != KEY_SHORT) return;
        if (!g_home) return;

        auto* ev = new InputEvent{g_home, key_id};
        lv_async_call(do_key_event, ev);
    }
}
