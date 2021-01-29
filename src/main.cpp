/*
Shmøergh Drummer - Created by Peter Zimon
---------------------------------------------------------------
Modular synth drum module that can triggers sequences on 4 channels. 
The premise of this module is that the output resembles a real drummer,
so the kick, the snare and the hihats are triggered like it would be
played by a drummer not a machine.

There are 4 output channels with 5V trigger outputs: a kick, a snare,
a closed and an open hihat, but ofc. it can be used to trigger anything.
When clock is received, a 16beat loop is going on constantly. The output
of each channel is a combination of two parameters: a fixed base pattern 
and a intensity pattern that is added to the base. There are multiple base
patterns and multiple intensity patterns for each channel. Additionally
the intensity is semi-randomized, so the "fills" will never sound exactly 
the same.
*/

#include <Arduino.h>
#include <math.h>
#include "patterns.h"
#include <Drummer.h>

// Pin setup
#define RESET_BUTTON 8                  // Moves sequence back to position 0
#define CLOCK_IN 2                      // Usually in 16ths
#define PATTERN_SELECTOR_BD A0          // Bass drum pattern selector
#define PATTERN_SELECTOR_SN A1          // Snare drum pattern selector
#define PATTERN_SELECTOR_HHC A2         // Hihat Closed drum pattern selector
#define PATTERN_SELECTOR_HHO A3         // Hihat Open drum pattern selector
#define INTENSITY_KNOB A4               // Well... it's the... wait for it... INTENSITY KNOB (I know right)
#define SHUFFLE_KNOB A5                 // Swing/shuffle value knob

// Constants
#define CLOCK_PULSE_LENGTH 20           // (ms) Adjust this according to the clock source pulse length
#define TRIGGER_PULSE_LENGTH 20         // (ms) How long should a trigger last
#define SHUFFLE_RESOLUTION 20           // How sensitive shuffle should be
#define AUTO_RESET_TIME 3000            // How long it should wait before autoreset
#define RESET_STEP 0b1000000000000001               // Where to start sequence after auto reset. Dealing with latency

// Handling long/short press
#define LONG_PRESS_TIME 500

// Clock
#define DOWNBEAT 0b1000100010001000

// Preset pins – Used only to trigger outputs individually. While playing the sequence the output is written directly to port D
#define CLOCK_LED 3                  // Clock (= clockIn / 4)
#define BD_OUT 4                     // Bass drum output
#define SN_OUT 5                     // Snare output
#define HHC_OUT 6                    // HiHat closed output
#define HHO_OUT 7                    // HiHat open output

// Sequencer
unsigned long pulseKeeper             = 0;                    // Variable to keep a pulse as long as it should last
volatile bool clockState    = false;
bool pulseState             = false;
bool triggerState           = false;
uint16_t currentStep        = 0b1000000000000000;   // Actual step in the sequence, stored in binary, using less memory
uint8_t currentSixteenth    = 0;
int patternBD               = 0;
int patternSN               = 0;
int patternHHC              = 0;
int patternHHO              = 0;
int noOfPatternsBD          = 0;
int noOfPatternsSN          = 0;
int noOfPatternsHHC         = 0;
int noOfPatternsHHO         = 0;
int intensity               = 0;
int currentIntensity        = 0;
int shuffleValue            = 0;                    // 0 - 50
uint16_t extraNotesBD       = 0b0000000000000000;
uint16_t extraNotesSN       = 0b0000000000000000;
uint16_t extraNotesHHC      = 0b0000000000000000;
uint16_t extraNotesHHO      = 0b0000000000000000;
uint16_t pulseLength        = 0;
unsigned long lastClock               = 0;

// Button states
int resetButtonState = LOW;
int prevResetState = LOW;
unsigned long pressedTime = 0;
unsigned long releasedTime = 0;

// Utility variables
int analogMux = 0;
int intensityMux = 0;
Drummer drummer;

// Interrupt routine
void onClockIn() {
    clockState = true;
}

void initButtons() {
    pinMode(CLOCK_IN, INPUT);
    pinMode(RESET_BUTTON, INPUT_PULLUP);

    resetButtonState = digitalRead(RESET_BUTTON);
    prevResetState = resetButtonState;
}

void initKnobs() {
    patternBD = drummer.mapKnob(noOfPatternsBD, analogRead(PATTERN_SELECTOR_BD));
    patternSN = drummer.mapKnob(noOfPatternsSN, analogRead(PATTERN_SELECTOR_SN));
    patternHHC = drummer.mapKnob(noOfPatternsHHC, analogRead(PATTERN_SELECTOR_HHC));
    patternHHO = drummer.mapKnob(noOfPatternsHHO, analogRead(PATTERN_SELECTOR_HHO));
    intensity = drummer.mapKnob(INTENSITY_LEVELS, analogRead(INTENSITY_KNOB));
    shuffleValue = drummer.mapKnob(SHUFFLE_RESOLUTION, analogRead(SHUFFLE_KNOB));
}

void playStartupAnimation() {
    delay(100);
    drummer.trigger(CLOCK_LED);
    drummer.trigger(BD_OUT);
    drummer.trigger(SN_OUT);
    drummer.trigger(HHC_OUT);
    drummer.trigger(HHO_OUT);
}

void blinkLed(int led, int times = 1, int delayTime = 50) {
    for (int i = 0; i < times; i++) {
        drummer.trigger(led);
        delay(delayTime);
    }
}

void resetSequence(bool autoreset) {
    currentStep = RESET_STEP;
    currentSixteenth = 0;
    clockState = false;
    shuffleValue = 0;
    lastClock = millis();

    if (autoreset) {
        blinkLed(CLOCK_LED, 3, 50);
    }
}

void calculateIntensity(bool force) {
    /*
    The extra note calculator is pretty cool but very calculating heavy.
    In order to not compromise the accuracy these notes should be calculated
    only if there's a change in intensity and only if it hasn't been 
    calculated for the given drum at that intensity. Also the whole thing can 
    be multiplexed. 
    /* -------------------------------------------------------------------- */
    if (intensity != currentIntensity || force) {
        // Add intensity one by one
        switch (intensityMux)
        {
        case 0:
            extraNotesBD = drummer.extraNotes(intensityBD[intensity]);
            intensityMux++;
            break;
        case 1:
            extraNotesSN = drummer.extraNotes(intensitySN[intensity]);
            intensityMux++;
            break;
        case 2:
            extraNotesHHC = drummer.extraNotes(intensityHHC[intensity]);
            intensityMux++;
            break;
        case 3:
            extraNotesHHO = drummer.extraNotes(intensityHHO[intensity]);
            intensityMux = 0;
            currentIntensity = intensity;
            break;
        default:
            break;
        }

        Serial.println("---");
        Serial.println(intensity);
    }
}

void setup() {

    // Init inputs
    initButtons();
    initKnobs();

    // Set PORTD
    //             76543210
    DDRD = DDRD | B11111000;
    PORTD |= B00000000;

    // Listen to clock in on INT0 (digital pin 2)
    attachInterrupt(digitalPinToInterrupt(CLOCK_IN), onClockIn, FALLING);

    // Init patterns
    noOfPatternsBD  = NELEMS(seqBD);
    noOfPatternsSN  = NELEMS(seqSN);
    noOfPatternsHHC = NELEMS(seqHHC);
    noOfPatternsHHO = NELEMS(seqHHO);

    // Begin serial output
    Serial.begin(9600);

    // Bootup led parade (note: also hits triggers)
    playStartupAnimation();

    resetSequence(true);
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
        if (currentStep & uint16_t(seqBD[patternBD]) || currentStep & extraNotesBD) {
            portDOut = portDOut | B00010000;
        }

        // Trigger snare - on pin 5 of PORTD
        if (currentStep & uint16_t(seqSN[patternSN]) || currentStep & extraNotesSN) {
            portDOut = portDOut | B00100000;
        }

        // // Trigger hihat closed - on pin 6 of PORTD
        if (currentStep & uint16_t(seqHHC[patternHHC]) || currentStep & extraNotesHHC) {
            portDOut = portDOut | B01000000;
        }

        // // Trigger hihat open - on pin 6 of PORTD
        if (currentStep & uint16_t(seqHHO[patternHHO]) || currentStep & extraNotesHHO) {
            portDOut = portDOut | B10000000;
        }

        if (shuffleValue != 0 && currentSixteenth % 2) {
            delay((drummer.shuffleDelay(float(pulseLength), float(SHUFFLE_RESOLUTION), float(shuffleValue))));
        }
        
        // Multiplex reading of patterns (analog outputs) so that it doesn't delay triggers
        // Basically, care only one change at a time and only at 16th notes. This also
        // avoids sudden changes.
        switch (analogMux)
        {
        case 0:
            patternBD = drummer.mapKnob(noOfPatternsBD, analogRead(PATTERN_SELECTOR_BD));
            analogMux++;
            break;
        case 1:
            patternSN = drummer.mapKnob(noOfPatternsSN, analogRead(PATTERN_SELECTOR_SN));
            analogMux++;
            break;
        case 2:
            patternHHC = drummer.mapKnob(noOfPatternsHHC, analogRead(PATTERN_SELECTOR_HHC));
            analogMux++;
            break;
        case 3:
            patternHHO = drummer.mapKnob(noOfPatternsHHO, analogRead(PATTERN_SELECTOR_HHO));
            analogMux++;
            break;
        case 4:
            intensity = drummer.mapKnob(INTENSITY_LEVELS, analogRead(INTENSITY_KNOB));
            analogMux++;
            break;
        case 5:
            shuffleValue = drummer.mapKnob(SHUFFLE_RESOLUTION, analogRead(SHUFFLE_KNOB));
            analogMux = 0;
            break;
        default:
            break;
        }

        calculateIntensity(false);

        // Update outputs
        PORTD |= portDOut;
        triggerState = true;
        
        // Reset steps
        currentStep >>= 1;
        if (currentStep == 0) {
            currentStep = 0b1000000000000000;
        }

        // Shuffle stuff
        currentSixteenth++;
        if (currentSixteenth >= 16) currentSixteenth = 0;
        pulseLength = millis() - pulseKeeper;
        
        clockState = false;
        pulseState = true;
        triggerState = true;
        
        pulseKeeper = millis();
        lastClock = millis();

        // I kept this here becuase previously I had problems with the millis() approach. This here worked...
        // Delay for the length of a pulse length...
        // delay(CLOCK_PULSE_LENGTH);

        // ...and reset the output
        // PORTD &= B00000011;
    }

    if (triggerState && (millis() - pulseKeeper) > TRIGGER_PULSE_LENGTH) {
        PORTD &= B00000011;
        triggerState = false;
    }

    if (pulseState && (millis() - pulseKeeper) > CLOCK_PULSE_LENGTH) {
        pulseState = false;
    }

    /*  
    Reset button
    
    While playing
    -------------
    - when short pressed, sequence restarts from top on release
    - when long pressed rolls the dice on intensity (recalculates extra random notes)
    /* ------------------------------------------------- */
    resetButtonState = digitalRead(RESET_BUTTON);
    if  (resetButtonState != prevResetState) {
        if (resetButtonState == LOW) { // Reset button is pressed
            pressedTime = millis();
        } else { // Reset button is released
            releasedTime = millis();
            long pressedDuration = releasedTime - pressedTime;
            if (pressedDuration > LONG_PRESS_TIME) {
                calculateIntensity(true);
            } else {
                resetSequence(false);
            }
        }
        prevResetState = resetButtonState;
        delay(50);
    }
    

    // Autoreset after 2 seconds of clock not coming in
    if ((millis() - lastClock) > AUTO_RESET_TIME && currentStep != RESET_STEP) {
        resetSequence(true);
    }
}