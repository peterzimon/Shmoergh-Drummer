#include <Arduino.h>
#include <math.h>
#include "patterns.h"
#include <Drummer.h>

// Pin setup
#define RESET_BUTTON 8                  // Moves sequence back to position 0
#define CLOCK_IN 2                      // Usually in 16ths
#define CLOCK_PULSE_LENGTH 10           // (ms) Adjust this according to the clock source pulse length
#define TRIGGER_PULSE_LENGTH 30         // (ms) How long should a trigger last
#define PATTERN_SELECTOR_BD A0          // Bass drum pattern selector
#define PATTERN_SELECTOR_SN A1          // Bass drum pattern selector
#define PATTERN_SELECTOR_HHC A2         // Bass drum pattern selector
#define PATTERN_SELECTOR_HHO A3         // Bass drum pattern selector

// Clock
#define DOWNBEAT 0b1000100010001000

// Preset pins
// #define CLOCK_LED 3                  // Clock (= clockIn / 4)
// #define BD_OUT 4                     // Bass drum output
// #define SN_OUT 5                     // Snare output
// #define HHC_OUT 6                    // HiHat closed output
// #define HHO_OUT 7                    // HiHat open output

// Sequencer
int pulseKeeper = 0;                    // Variable to keep a pulse as long as it should last
volatile bool clockState = false;
bool pulseState = false;
bool triggerState = false;
uint16_t currentStep = 0b1000000000000000;   // Actual step in the sequence, stored in binary, using less memory
int patternBD = 0;
int patternSN = 0;
int patternHHC = 0;
int patternHHO = 0;
int noOfPatternsBD = 0;
int noOfPatternsSN = 0;
int noOfPatternsHHC = 0;
int noOfPatternsHHO = 0;

// Button states
int resetButtonState = 0;
int prevResetState = 0;

// Utility variables
int patternMux = 0;
Drummer drummer;

// Interrupt routine
void onClockIn() {
    clockState = true;
}

void readButtons() {
    resetButtonState = digitalRead(RESET_BUTTON);
}

void initPatterns() {
    patternBD = drummer.mapKnob(noOfPatternsBD, analogRead(PATTERN_SELECTOR_BD));
    patternSN = drummer.mapKnob(noOfPatternsSN, analogRead(PATTERN_SELECTOR_SN));
    patternHHC = drummer.mapKnob(noOfPatternsHHC, analogRead(PATTERN_SELECTOR_HHC));
    patternHHO = drummer.mapKnob(noOfPatternsHHO, analogRead(PATTERN_SELECTOR_HHO));
}

void setup() {
    // Init pins
    pinMode(CLOCK_IN, INPUT);
    pinMode(RESET_BUTTON, INPUT_PULLUP);

    // Set PORTD
    //             76543210
    DDRD = DDRD | B11111000;
    PORTD |= B00000000;

    // Listen to clock in on INT0 (digital pin 2)
    attachInterrupt(digitalPinToInterrupt(CLOCK_IN), onClockIn, RISING);

    // Init patterns
    noOfPatternsBD = NELEMS(seqBD);
    noOfPatternsSN = NELEMS(seqSN);
    noOfPatternsHHC = NELEMS(seqHHC);
    noOfPatternsHHO = NELEMS(seqHHO);
    initPatterns();

    // Begin serial output
    Serial.begin(9600);
}

void loop() {
    // Listen to external sync signals
    if (clockState && !pulseState) {

        int portDOut = B00000000;

        // Blink clock on every downbeat - on pin 3 of PORTD
        if (currentStep & uint16_t(DOWNBEAT)) {
            portDOut = portDOut | B00001000;
        }

        // Trigger bass drum - on pin 4 of PORTD
        if (currentStep & uint16_t(seqBD[patternBD])) {
            portDOut = portDOut | B00010000;
        }

        // Trigger snare - on pin 5 of PORTD
        if (currentStep & uint16_t(seqSN[patternSN])) {
            portDOut = portDOut | B00100000;
        }

        // // Trigger hihat closed - on pin 6 of PORTD
        if (currentStep & uint16_t(seqHHC[patternHHC])) {
            portDOut = portDOut | B01000000;
        }

        // // Trigger hihat open - on pin 6 of PORTD
        if (currentStep & uint16_t(seqHHO[patternHHO])) {
            portDOut = portDOut | B10000000;
        }

        // Update outputs
        PORTD |= portDOut;
        
        // Multiplex reading of patterns (analog outputs) so that it doesn't delay triggers
        // Basically, care only one change at a time and only at 16th notes. This also
        // avoids sudden changes.
        switch (patternMux)
        {
        case 0:
            patternBD = drummer.mapKnob(noOfPatternsBD, analogRead(PATTERN_SELECTOR_BD));
            patternMux++;
            break;
        case 1:
            patternSN = drummer.mapKnob(noOfPatternsSN, analogRead(PATTERN_SELECTOR_SN));
            patternMux++;
            break;
        case 2:
            patternHHC = drummer.mapKnob(noOfPatternsHHC, analogRead(PATTERN_SELECTOR_HHC));
            patternMux++;
            break;
        case 3:
            patternHHO = drummer.mapKnob(noOfPatternsHHO, analogRead(PATTERN_SELECTOR_HHO));
            patternMux = 0;
            break;
        default:
            break;
        }
        
        // Reset steps
        currentStep >>= 1;
        if (currentStep == 0) {
            currentStep = 0b1000000000000000;
        }

        clockState = false;
        pulseState = true;
        triggerState = true;
        pulseKeeper = millis();

        // Hack to fix dropping serial
        delay(TRIGGER_PULSE_LENGTH);
    }

    // TODO: Read various buttons with multiplexing to avoid delays and skipped triggers
    if (digitalRead(RESET_BUTTON) == LOW) {
        currentStep = 0;
    }

    // Reset output after trigger length duration
    if (triggerState) {
        PORTD &= B00000011;
        triggerState = false;
    }

    // Open up for external clock trigger
    if (pulseState && (millis() - pulseKeeper) > CLOCK_PULSE_LENGTH) {
        pulseState = false;
    }
}