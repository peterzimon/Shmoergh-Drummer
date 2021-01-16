#include <Arduino.h>
#include <math.h>
#include "patterns.h"

// Pin setup
#define RESET_BUTTON 8              // Moves sequence back to position 0
#define CLOCK_IN 2                  // Usually in 16ths
#define CLOCK_PULSE_LENGTH 10       // (ms) Adjust this according to the clock source pulse length
#define TRIGGER_LENGTH 50           // (ms) How long should a trigger last
#define PATTERN_SELECTOR_BD A0      // Bass drum pattern selector

// Preset pins
// CLOCK_LED 3             Clock (= clockIn / 4)
// BD_OUT 4                Bass drum output
// SN_OUT 5                Snare output
// HHC_OUT 6               HiHat closed output
// HHO_OUT 7               HiHat open output

// Sequencer
int pulseKeeper = 0;            // Variable to keep a pulse as long as it should last
int clockState = LOW;
int pulseState = LOW;
int currentStep = 0;            // Actual step in the sequence
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
        if (seqSN[0][currentStep] == 1) {
            portDOut = portDOut | B00100000;
        }

        // Trigger hihat closed - on pin 6 of PORTD
        if (seqHHC[0][currentStep] == 1) {
            portDOut = portDOut | B01000000;
        }

        // Trigger hihat open - on pin 6 of PORTD
        if (seqHHO[0][currentStep] == 1) {
            portDOut = portDOut | B10000000;
        }

        // Update outputs
        PORTD = portDOut;

        // TODO: multiplex reading of analog output so that it doesn't delay triggers
        // int bdPattern = analogRead(PATTERN_SELECTOR_BD);
        // Serial.println(bdPattern);
        patternBD = getPattern(noOfPatternsBD, analogRead(PATTERN_SELECTOR_BD));
        Serial.print("BD pattern: ");
        Serial.println(patternBD);
        
        // Reset
        currentStep++;
        if (currentStep == 16) {
            currentStep = 0;
        }

        clockState = LOW;
        pulseState = HIGH;
        pulseKeeper = millis();
    }

    // TODO: Read various buttons with multiplexing to avoid delays and skipped triggers
    if (digitalRead(RESET_BUTTON) == LOW) {
        currentStep = 0;
    }

    // Reset output after trigger length duration
    if (millis() - pulseKeeper > TRIGGER_LENGTH) {
        delay(20);
        Serial.begin(9600);
        PORTD = B00000000;
    }

    // Open up for external clock trigger
    if (pulseState == HIGH && millis() - pulseKeeper > CLOCK_PULSE_LENGTH) {
        pulseState = LOW;
    }
}