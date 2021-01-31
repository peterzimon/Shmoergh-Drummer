/*
    Drummer
    Created by Peter Zimon, 2021 Jan 16
*/

#ifndef Drummer_h
#define Drummer_h
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#include <Arduino.h>

class Drummer {
    public: 
        Drummer(int maxIntensity = 0);

        uint16_t mapKnob(uint16_t noOfOptions, uint16_t potValue);
        void trigger(uint8_t pin);

        uint16_t extraNotes(uint16_t map, int intensity = 0);
        uint16_t bpm(int sixteenths);
        uint16_t shuffleDelay(float pulseLength, float shuffleResolution, float shuffleValue);

    private:
        int _maxIntensity = 0;
};
#endif