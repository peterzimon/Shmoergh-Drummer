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

uint16_t Drummer::extraNotes(uint16_t map) {
    // Loop through the bits in the map and if it's 1 then randomize it
    uint16_t currentMap = 1;
    uint16_t extranotes = 0;
    for (int i = 0; i < 16; i++) {
        if (uint16_t(currentMap) & map) {
            if (random(0, 2)) {
                extranotes |= currentMap;
            }
        }
        currentMap <<= 1;
    }
    return extranotes;
}