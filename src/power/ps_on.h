#pragma once

/**
 * @brief Initialize PS_ON and PW_OK GPIO pins.
 *
 * PS_ON (GPIO27): output, HIGH=off (ATX convention)
 * PW_OK (GPIO34): input, no internal pullup (input-only pin)
 */
void ps_on_init(void);

/**
 * @brief Assert PS_ON (turn ATX PSU on).
 * Drives PS_ON LOW.
 */
void ps_on_assert(void);

/**
 * @brief Deassert PS_ON (turn ATX PSU off).
 * Drives PS_ON HIGH.
 */
void ps_on_deassert(void);

/**
 * @brief Read PW_OK signal.
 * @return true if PW_OK is HIGH (power good)
 */
bool ps_on_pwok_read(void);
