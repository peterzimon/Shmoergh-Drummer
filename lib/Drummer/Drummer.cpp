#include <Arduino.h>
#include <math.h>
#include "Drummer.h"

#define POT_MAX_VALUE 1023.0
#define BLINK_DELAY 50
#define CHANCE_LOWER_LIMIT 0.5
#define CHANCE_UPPER_LIMIT 0.9

Drummer::Drummer(int maxIntensity = 0) {
    this->_maxIntensity = maxIntensity;
}

uint16_t Drummer::mapKnob(uint16_t noOfOptions, uint16_t potValue) {
    int range = ceil(POT_MAX_VALUE / float(noOfOptions));
    return floor(potValue / range);
}

void Drummer::trigger(uint8_t pin) {
    digitalWrite(pin, HIGH);
    delay(BLINK_DELAY);
    digitalWrite(pin, LOW);
}

uint16_t Drummer::extraNotes(uint16_t map, int intensity) {
    // if (intensity == 0) return;

    // Loop through the bits in the map and if it's 1 then randomize it
    uint16_t currentMap = 1;
    uint16_t extranotes = 0;

    // Calculate how much chance the new notes have. Depends on intensity
    float chance = CHANCE_LOWER_LIMIT + (((CHANCE_UPPER_LIMIT - CHANCE_LOWER_LIMIT)/(_maxIntensity - 2)) * (intensity - 1));

    for (int i = 0; i < 16; i++) {
        if (uint16_t(currentMap) & map) {
            if (random(0, _maxIntensity + 1) < (_maxIntensity * chance)) {
                extranotes |= currentMap;
            }
        }
        currentMap <<= 1;
    }
    return extranotes;
}

uint16_t Drummer::bmp(int sixteenths) {
    return floor(60000 / (sixteenths * 4));
}

uint16_t Drummer::shuffleDelay(float pulseLength, float shuffleResolution,  float shuffleValue) {
    return round((((pulseLength / 1.5) - (pulseLength / 100)) / shuffleResolution) * shuffleValue); // (pulseLength / 100) is the spacing between the full pulse length and max shuffle time
}