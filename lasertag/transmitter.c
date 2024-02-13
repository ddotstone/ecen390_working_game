#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "filter.h"
#include "transmitter.h"
#include "mio.h"
#include "buttons.h"
#include "switches.h"
#include "utils.h"

#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10
#define BOUNCE_DELAY 5
#define TIME_DELAY 400
// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h

// State of lockout timer
typedef enum {
  NOT_TRANSMITTING,    // Setting transmitter to the initial state
  TRANSMITTING_HIGH, // Hit detected and lock out commences for .5 seconds
  TRANSMITTING_LOW, // Lock out time is completed and waiting for new hit
  INIT // Setting the timer back to 0
} transmitter_state_t;

volatile static transmitter_state_t transmitterState;
volatile static uint16_t transmittingFrequency;
volatile static uint16_t transmittingFrequencyModified;
volatile bool continuousFlag;

// Standard init function.
void transmitter_init() {
    transmitterState = INIT;
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.
}

void transmitter_set_jf1_to_one() {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE); // Write a '1' to JF-1.
}

void transmitter_set_jf1_to_zero() {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write a '0' to JF-1.
}

// Standard tick function.
void transmitter_tick() {
    static uint16_t timer = 0;
    switch(transmitterState) //State update
    {
        case NOT_TRANSMITTING:   // Setting transmitter to the initial state
            if (continuousFlag) { transmitterState = TRANSMITTING_HIGH; }
           break;

        case TRANSMITTING_HIGH:  // Hit detected and lock out commences for .5 seconds
            if (timer == TRANSMITTER_PULSE_WIDTH) { transmitterState = INIT; }
            else if (!(timer % (filter_frequencyTickTable[transmittingFrequency] / 2))) {
                transmitter_set_jf1_to_zero();
                transmitterState = TRANSMITTING_LOW;
                printf("\n");

            }
            break;

        case TRANSMITTING_LOW:  // Lock out time is completed and waiting for new hit
            if (timer == TRANSMITTER_PULSE_WIDTH) { transmitterState = INIT; }
            else if (!(timer % (filter_frequencyTickTable[transmittingFrequency] / 2))) {
                transmitter_set_jf1_to_one();
                transmitterState = TRANSMITTING_HIGH; 
                printf("\n");
            }
            break;

        case INIT:  // Lock out time is completed and waiting for new hit
            transmitterState = NOT_TRANSMITTING;
            transmitter_set_jf1_to_zero();
            break;

        default:    //default case essentially an error state
            printf("No state\n");
    }   


    switch(transmitterState) //State action
    {
        case NOT_TRANSMITTING:   // Setting transmitter to the initial state waiting to get initial hit
            transmittingFrequency = transmittingFrequencyModified;
            break;

        case TRANSMITTING_HIGH:  // Hit detected and lock out commences for .5 seconds
            printf("1 ");
            timer++;
            break;

        case TRANSMITTING_LOW:    // Lock out time is completed and waiting for new hit
            printf("0 ");
            timer++;
            break;

        case INIT:  // Lock out time is completed and waiting for new hit
            timer = 0;
            break;

        default:    //default case
            printf("No state");
    } 
}

// Activate the transmitter.
void transmitter_run() {
    transmitterState = TRANSMITTING_HIGH;
}

// Returns true if the transmitter is still running.
bool transmitter_running() {
    return transmitterState;
}

// Sets the frequency number. If this function is called while the
// transmitter is running, the frequency will not be updated until the
// transmitter stops and transmitter_run() is called again.
void transmitter_setFrequencyNumber(uint16_t frequencyNumber) {
    transmittingFrequencyModified = frequencyNumber;
}

// Returns the current frequency setting.
uint16_t transmitter_getFrequencyNumber() {
    return transmittingFrequency;
}

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise, it
// transmits one burst and stops. To set continuous mode, you must invoke
// this function prior to calling transmitter_run(). If the transmitter is
// currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or wait
// until a 200 ms burst is complete. NOTE: while running continuously,
// the transmitter will only change frequencies in between 200 ms bursts.
void transmitter_setContinuousMode(bool continuousModeFlag) {
    continuousFlag = continuousModeFlag;
}

/******************************************************************************
***** Test Functions
******************************************************************************/

// Prints out the clock waveform to stdio. Terminates when BTN3 is pressed.
// Does not use interrupts, but calls the tick function in a loop.
void transmitter_runTest() {
    printf("starting transmitter_runTest()\n");
    transmitter_init();             
    transmitter_setContinuousMode(false);
                        // init the transmitter.
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {         // Run continuously until BTN3 is pressed.
            uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
            transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
            transmitter_run();                                    // Start the transmitter.
            while (transmitter_running()) {                       // Keep ticking until it is done.
            transmitter_tick();                                 // tick.
            utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
        }
        printf("completed one test period.\n");
    }
    do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
    printf("exiting transmitter_runTest()\n");
}

// Tests the transmitter in non-continuous mode.
// The test runs until BTN3 is pressed.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test. You should see about a 300 ms dead
// spot between 200 ms pulses.
// Should change frequency in response to the slide switches.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestNoncontinuous() {
    uint16_t switchesValue = 0;
    transmitter_setContinuousMode(false);

    while (!(buttons_read() & BUTTONS_BTN3_MASK)) { 
        switchesValue = switches_read() % FILTER_FREQUENCY_COUNT; // Read the slide switches and use the numerical value of the switches as the frequency index. When all switches are in the down position (closest to the bottom of the board), you should generate the waveform for frequency 0. Sliding switch (SW0) upward would select frequency 1, and so forth.
        transmitter_setFrequencyNumber(switchesValue); // Set the frequency using the transmitter_setFrequencyNumber() function.
        transmitter_run(); //Invoke transmitter_run() and then wait for transmitter_running() to return false.
        while (transmitter_running()) { continue; }
        utils_msDelay(TIME_DELAY); // Use the utils_msDelay() function to delay for approximately 400 ms, long enough to demonstrate the operation of the transmitter on the oscilloscope.
    } // Loop until Button 3 is pressed:
    
    while ((buttons_read() & BUTTONS_BTN3_MASK)) { continue; } // Wait for the release of Button 3.
}

// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies
// in response to changes in the slide switches.
// Test runs until BTN3 is pressed.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestContinuous() {
    uint16_t switchesValue = 0;
    transmitter_setContinuousMode(true);
    transmitter_run(); // Invoke transmitter_setContinuousMode(true), transmitter_run() prior to entering an endless loop.
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {  // Loop until Button 3 is pressed:
        switchesValue = switches_read() % FILTER_FREQUENCY_COUNT;
        transmitter_setFrequencyNumber(switchesValue); //Set the frequency using the transmitter_setFrequencyNumber() function.
    }
    while ((buttons_read() & BUTTONS_BTN3_MASK)) { continue; } // Wait for the release of Button 3.

}

