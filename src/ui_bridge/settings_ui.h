#pragma once

#include <cstdint>

#include "../input/keys.h"

namespace settings_ui {

void init();
void show();
void hide();
void handle_key(uint8_t key_id, const KeyState& state);
bool is_active();

} // namespace settings_ui
