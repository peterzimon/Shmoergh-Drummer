#include <Arduino.h>
#include <math.h>
#include "patterns.h"

// Pin setup
#define RESET_BUTTON 8              // Moves sequence back to position 0
#define CLOCK_IN 2                  // Usually in 16ths
#define CLOCK_PULSE_LENGTH 10       // (ms) Adjust this according to the clock source pulse length
#define TRIGGER_PULSE_LENGTH 20     // (ms) How long should a trigger last
#define PATTERN_SELECTOR_BD A0      // Bass drum pattern selector
#define PATTERN_SELECTOR_SN A1      // Bass drum pattern selector
#define PATTERN_SELECTOR_HHC A2     // Bass drum pattern selector
#define PATTERN_SELECTOR_HHO A3     // Bass drum pattern selector

// Preset pins
// #define CLOCK_LED 3                 // Clock (= clockIn / 4)
// #define BD_OUT 4                    // Bass drum output
// #define SN_OUT 5                    // Snare output
// #define HHC_OUT 6                   // HiHat closed output
// #define HHO_OUT 7                   // HiHat open output

// Sequencer
int pulseKeeper = 0;                // Variable to keep a pulse as long as it should last
int clockState = LOW;
int pulseState = LOW;
int triggerState = LOW;
int currentStep = 0;                // Actual step in the sequence
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

// Just blink a pin
void blink(int pin) {
    digitalWrite(pin, HIGH);
    delay(50);
    digitalWrite(pin, LOW);
}

// Interrupt routine
void onClockIn() {
    clockState = HIGH;
}

void readButtons() {
    resetButtonState = digitalRead(RESET_BUTTON);
}

int getPattern(int noOfPatterns, int potValue) {
    int maxPotValue = 1023;
    float range = maxPotValue / noOfPatterns;
    return floor(potValue / range);
}

void setup() {
    // Init pins
    pinMode(CLOCK_IN, INPUT);
    pinMode(RESET_BUTTON, INPUT_PULLUP);

    // Set PORTD
    //             76543210
    DDRD = DDRD | B11111000;
    PORTD = B00000000;

    // Listen to clock in on INT0 (digital pin 2)
    attachInterrupt(digitalPinToInterrupt(CLOCK_IN), onClockIn, RISING);

    // Count number of patters
    noOfPatternsBD = (sizeof(seqBD) / sizeof(seqBD[0])) - 1;
    noOfPatternsSN = (sizeof(seqSN) / sizeof(seqSN[0])) - 1;
    noOfPatternsHHC = (sizeof(seqHHC) / sizeof(seqHHC[0])) - 1;
    noOfPatternsHHO = (sizeof(seqHHO) / sizeof(seqHHO[0])) - 1;

    // Begin serial output
    Serial.begin(9600);
    Serial.println(noOfPatternsBD);
}

void loop() {
    // Listen to external sync signals
    if (clockState == HIGH && pulseState == LOW) {

        int portDOut = B00000000;

        // Blink clock on every downbeat - on pin 3 of PORTD
        if (currentStep % 4 == 0) {
            portDOut = portDOut | B00001000;
        }

        // Trigger bass drum - on pin 4 of PORTD
        if (seqBD[patternBD][currentStep] == 1) {
            portDOut = portDOut | B00010000;
        }

        // Trigger snare - on pin 5 of PORTD
        if (seqSN[patternSN][currentStep] == 1) {
            portDOut = portDOut | B00100000;
        }

        // Trigger hihat closed - on pin 6 of PORTD
        if (seqHHC[patternHHC][currentStep] == 1) {
            portDOut = portDOut | B01000000;
        }

        // Trigger hihat open - on pin 6 of PORTD
        if (seqHHO[patternHHO][currentStep] == 1) {
            portDOut = portDOut | B10000000;
        }

        // Update outputs
        PORTD |= portDOut;
        
        //Multiplex reading of patterns (analog outputs) so that it doesn't delay triggers
        // Basically, care only one change at a time and only at 16th notes. This also
        // avoids sudden changes.
        patternMux++;
        switch (patternMux)
        {
        case 0:
            patternBD = getPattern(noOfPatternsBD, analogRead(PATTERN_SELECTOR_BD));
            break;
        case 1:
            patternSN = getPattern(noOfPatternsSN, analogRead(PATTERN_SELECTOR_SN));
            break;
        case 2:
            patternHHC = getPattern(noOfPatternsHHC, analogRead(PATTERN_SELECTOR_HHC));
            break;
        case 3:
            patternHHO = getPattern(noOfPatternsHHO, analogRead(PATTERN_SELECTOR_HHO));
            patternMux = 0;
            break;
        default:
            break;
        }
        
        // Reset
        currentStep++;
        if (currentStep == 16) {
            currentStep = 0;
        }

        clockState = LOW;
        pulseState = HIGH;
        triggerState = HIGH;
        pulseKeeper = millis();

        delay(TRIGGER_PULSE_LENGTH); // Hack to fix dropping serial out
    }

    // TODO: Read various buttons with multiplexing to avoid delays and skipped triggers
    if (digitalRead(RESET_BUTTON) == LOW) {
        currentStep = 0;
    }

    // Reset output after trigger length duration
    if (triggerState == HIGH && (millis() - pulseKeeper) > TRIGGER_PULSE_LENGTH) {
        PORTD &= B00000011;
        triggerState = LOW;
    }

    // Open up for external clock trigger
    if (pulseState == HIGH && (millis() - pulseKeeper) > CLOCK_PULSE_LENGTH) {
        pulseState = LOW;
    }
}