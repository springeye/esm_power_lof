#pragma once
#include <cstdint>

enum ScreenRotation : uint8_t {
    ROT_0   = 0,
    ROT_90  = 1,
    ROT_180 = 2,
    ROT_270 = 3
};

namespace screen_rotation {

void init();
void rotate_cw();
void rotate_ccw();
ScreenRotation get_current();
void set_rotation(ScreenRotation rot);

} // namespace screen_rotation
