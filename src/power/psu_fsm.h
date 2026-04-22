#pragma once

/**
 * PSU State Machine (design.md D7)
 * States and events as specified in the plan.
 */

typedef enum {
    PSU_OFF = 0,
    PSU_STANDBY,
    PSU_STARTING,
    PSU_ON,
    PSU_STOPPING,
    PSU_FAULT
} PsuState;

typedef enum {
    EVT_BOOT = 0,
    EVT_KEY_SHORT,
    EVT_KEY_LONG,
    EVT_PWOK_HIGH,
    EVT_PWOK_LOW,
    EVT_PWOK_LOST_100MS,
    EVT_TIMEOUT_1S,
    EVT_FAULT_RESET
} PsuFsmEvent;

/**
 * @brief Compute next PSU state given current state and event.
 *
 * Illegal transitions return the current state unchanged.
 *
 * @param state  Current PSU state
 * @param event  Incoming event
 * @return Next PSU state
 */
PsuState psu_fsm_transition(PsuState state, PsuFsmEvent event);
