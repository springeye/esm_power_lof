#include "power/ps_on.h"
#include "pins.h"
#include <Arduino.h>

void ps_on_init(void) {
    // PS_ON: output, default HIGH (PSU off)
    pinMode(PSON_PIN, OUTPUT);
    digitalWrite(PSON_PIN, HIGH);

    // PW_OK: input only, no internal pullup (GPIO34 has no pullup)
    pinMode(PWOK_PIN, INPUT);
}

void ps_on_assert(void) {
    // LOW = PSU on (ATX convention)
    digitalWrite(PSON_PIN, LOW);
}

void ps_on_deassert(void) {
    // HIGH = PSU off
    digitalWrite(PSON_PIN, HIGH);
}

bool ps_on_pwok_read(void) {
    return digitalRead(PWOK_PIN) == HIGH;
}
