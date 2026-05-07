#pragma once

#include <cstdint>

struct _lv_obj_t;

namespace chart_view {

void chart_view_init();

void chart_view_update(uint8_t ch);

void chart_view_set_window(uint8_t ch, uint32_t window_ms);

void chart_view_cycle_window();

uint32_t chart_view_get_window_ms();

} // namespace chart_view
