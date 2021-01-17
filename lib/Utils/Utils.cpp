#include <Arduino.h>
#include "Utils.h"

#define POT_MAX_VALUE 1023
#define BLINK_DELAY 50

Utils::Utils() {}

int Utils::mapKnob(int noOfOptions, int potValue) {
    float range = POT_MAX_VALUE / noOfOptions;
    return floor(potValue / range);
}

void Utils::blink(int pin) {
    digitalWrite(pin, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(pin, LOW);
}