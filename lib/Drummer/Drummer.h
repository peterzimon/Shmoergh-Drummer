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
        Drummer();

        // Utils
        uint16_t mapKnob(uint16_t noOfOptions, uint16_t potValue);
        void blink(uint8_t pin);

        // Algoriddims

    private:
        // 
};
#endif