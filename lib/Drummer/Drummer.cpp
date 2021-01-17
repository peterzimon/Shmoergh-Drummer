#include <Arduino.h>
#include "Drummer.h"

#define POT_MAX_VALUE 1023.0
#define BLINK_DELAY 50

Drummer::Drummer() {}

uint16_t Drummer::mapKnob(uint16_t noOfOptions, uint16_t potValue) {
    int range = ceil(POT_MAX_VALUE / float(noOfOptions));
    return floor(potValue / range);
}

void Drummer::blink(uint8_t pin) {
    digitalWrite(pin, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(pin, LOW);
}