/*
    Genric utilities
    Created by Peter Zimon, 2021 Jan 16
*/

#ifndef Utils_h
#define Utils_h
#define NELEMS(x)  (sizeof(x) / sizeof((x)[0]))
#include <Arduino.h>

class Utils {
    public: 
        Utils();
        uint16_t mapKnob(uint16_t noOfOptions, uint16_t potValue);
        void blink(uint8_t pin);
    private:
        // 
};
#endif