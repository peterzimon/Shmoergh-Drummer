#include <Arduino.h>

const int NO_OF_STEPS = 16;

// Bass drum patterns
uint8_t seqBD[][NO_OF_STEPS] = {
    {1,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,0,0},
    {1,0,0,0, 0,0,0,0, 1,0,0,0, 0,0,0,0},
    {1,0,0,0, 1,0,0,0, 1,0,0,0, 0,0,0,0},
    {1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0},
    {1,0,1,0, 1,0,1,0, 1,0,1,0, 1,0,0,1}
};

// Snare patterns
uint8_t seqSN[][NO_OF_STEPS] = {
    {0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0},
    {0,0,0,0, 1,0,0,0, 0,0,1,0, 1,0,0,0}
};

// HiHat closed patterns
uint8_t seqHHC[][NO_OF_STEPS] = {
    {1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0},
    {1,0,1,1, 1,0,1,1, 1,0,1,1, 1,0,1,1}
};

// HiHat open patterns
uint8_t seqHHO[][NO_OF_STEPS] = {
    {0,0,0,0, 0,0,0,0, 0,0,0,0, 0,0,1,0}
};