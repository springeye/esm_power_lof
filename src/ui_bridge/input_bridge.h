#pragma once
#include <cstdint>
#include "../input/keys.h"
struct _lv_obj_t;

namespace ui_bridge {
/** 缓存 home 根对象，供 input_handle_key 使用 */
void input_bridge_attach_home(struct _lv_obj_t* home);
/** 替代 ui_events::handle_key，由 tasks.cpp::input_task 调用 */
void input_handle_key(uint8_t key_id, const KeyState& state);
}
