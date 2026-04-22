#include "power/psu_fsm.h"

PsuState psu_fsm_transition(PsuState state, PsuFsmEvent event) {
    switch (state) {
        case PSU_OFF:
            if (event == EVT_BOOT) return PSU_STANDBY;
            break;

        case PSU_STANDBY:
            if (event == EVT_KEY_SHORT) return PSU_STARTING;
            break;

        case PSU_STARTING:
            if (event == EVT_PWOK_HIGH)  return PSU_ON;
            if (event == EVT_TIMEOUT_1S) return PSU_FAULT;
            break;

        case PSU_ON:
            if (event == EVT_KEY_LONG)        return PSU_STOPPING;
            if (event == EVT_PWOK_LOST_100MS) return PSU_FAULT;
            // KEY_SHORT in ON state → stay ON
            break;

        case PSU_STOPPING:
            if (event == EVT_PWOK_LOW) return PSU_OFF;
            break;

        case PSU_FAULT:
            if (event == EVT_FAULT_RESET) return PSU_OFF;
            break;

        default:
            break;
    }
    // Illegal transition: stay in current state
    return state;
}
