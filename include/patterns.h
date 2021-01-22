#include <Arduino.h>
#define INTENSITY_LEVELS 6

// Base patterns
uint16_t seqBD[] = {
    0b1000000000000000,
    0b1000000010000000,
    0b1000100010000000,
    0b1000100010001000,
    0b1010100010001000,
    0b1010100010101000,
    0b1010101010101000,
    0b1010101010101010
};

uint16_t seqSN[] = {
    0b0000100000001000,
    0b0000100000001010
};

uint16_t seqHHC[] = {
    0b1000000010000000,
    0b1000100010001000,
    0b0010001000100010,
    0b1010001010100010,
    0b1010101000101010,
    0b0010101010101010,
    0b1011101110111011,
    0b1101110111011101,
    0b1111111111111111
};

uint16_t seqHHO[] = {
    0b0000000000000000
};

// Intensity
uint16_t intensityBD[INTENSITY_LEVELS] = {
    0b0000000000000000,
    0b0000000000000001,
    0b0001000000000001,
    0b0001001000000001,
    0b0001001000000101,
    0b0001001001100101
};

uint16_t intensitySN[INTENSITY_LEVELS] = {
    0b0000000000000000,
    0b0000000001000000,
    0b0000000001000001,
    0b0000001001000001,
    0b0000001001000001,
    0b0100001001000001
};

uint16_t intensityHHC[INTENSITY_LEVELS] = {
    0b0000000000000000,
    0b0100000000000000,
    0b0100000000000100,
    0b0100000000000101,
    0b0100010000000101,
    0b0100010010000101
};

uint16_t intensityHHO[INTENSITY_LEVELS] = {
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000,
    0b0000000000000000
};