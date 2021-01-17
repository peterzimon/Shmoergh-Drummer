/*
    Genric utilities
    Created by Peter Zimon, 2021 Jan 16
*/

#ifndef Utils_h
#define Utils_h
#include <Arduino.h>

class Utils {
    public: 
        Utils();
        int mapKnob(int noOfOptions, int potValue);
        void blink(int pin);
    private:
        // 
};
#endif