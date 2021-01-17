#include <Arduino.h>
#include "Utils.h"

#define POT_MAX_VALUE 1023.0
#define BLINK_DELAY 50

Utils::Utils() {}

uint16_t Utils::mapKnob(uint16_t noOfOptions, uint16_t potValue) {
    int range = ceil(POT_MAX_VALUE / float(noOfOptions));
    return floor(potValue / range);
}

void Utils::blink(uint8_t pin) {
    digitalWrite(pin, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(pin, LOW);
}