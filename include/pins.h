#pragma once

#if defined(BOARD_VARIANT_RELEASE)
#  include "boards/pins_release.h"
#elif defined(BOARD_VARIANT_DEV)
#  include "boards/pins_dev.h"
#else
#  include "boards/pins_dev.h"
#endif
