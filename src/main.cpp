#include <Arduino.h>

// Pin setup
#define RESET_BUTTON 4          // Moves sequence back to position 0
#define CLOCK_IN 2              // Usually in 16ths
#define CLOCK_PULSE_LENGTH 10   // (ms) Adjust this according to the clock source pulse length
#define TRIGGER_LENGTH 50        // (ms) How long should a trigger last

// Preset pins
// CLOCK_LED 3             // Clock (= clockIn / 4)
// BD_OUT 4                // Bass drum output
// SN_OUT 5                // Snare output
// HC_OUT 6                // HiHat closed output
// HO_OUT 7                // HiHat open output

// ------------------------------------------------
// Test sequence
uint8_t seqBD[16] = {1,0,0,0, 1,0,0,0, 1,0,0,0, 1,0,0,0};
uint8_t seqSN[16] = {0,0,0,0, 1,0,0,0, 0,0,0,0, 1,0,0,0};
uint8_t seqHC[16] = {1,0,1,1, 1,0,1,1, 1,0,1,1, 1,0,1,1};

// ------------------------------------------------

// Sequencer
int pulseKeeper = 0;            // Variable to keep a pulse as long as it should last
int clockState = LOW;
int pulseState = LOW;
int currentStep = 0;            // Actual step in the sequence

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

    // Begin serial output
    Serial.begin(9600);
}

void loop() {
    // if (millis() - pulseKeeper > TRIGGER_LENGTH) {
    //     PORTD = B00000000;
    // }

    // Listen to external sync signals
    if (clockState == HIGH && pulseState == LOW) {

        Serial.println(currentStep);

        int portDOut = B00000000;

        // Blink clock on every downbeat - on pin 3 of PORTD
        if (currentStep % 4 == 0) {
            portDOut = portDOut | B00001000;
            // blink(3);
        }

        // Trigger bass drum - on pin 4 of PORTD
        if (seqBD[currentStep] == 1) {
            portDOut = portDOut | B00010000;
        }

        // Trigger snare - on pin 5 of PORTD
        if (seqSN[currentStep] == 1) {
            portDOut = portDOut | B00100000;
        }

        // Trigger hihat closed - on pin 6 of PORTD
        if (seqHC[currentStep] == 1) {
            portDOut = portDOut | B01000000;
        }

        // Update outputs
        PORTD = portDOut;
        
        // Reset
        currentStep++;
        if (currentStep == 16) {
            currentStep = 0;
        }

        clockState = LOW;
        pulseState = HIGH;
        pulseKeeper = millis();
    }

    if (millis() - pulseKeeper > TRIGGER_LENGTH) {
        delay(10);
        Serial.begin(9600);
        PORTD = B00000000;
    }

    if (pulseState == HIGH && millis() - pulseKeeper > CLOCK_PULSE_LENGTH) {
        pulseState = LOW;
    }
}