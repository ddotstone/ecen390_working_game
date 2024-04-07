#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include "filter.h"
#include "detector.h"
#include "transmitter.h"
#include "mio.h"
#include "buttons.h"
#include "switches.h"
#include "utils.h"


// Uncomment for debug prints
//#define DEBUG
 
#if defined(DEBUG)
#include <stdio.h>
#include "xil_printf.h"
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else
#define DPRINTF(...)
#define DPCHAR(ch)
#endif

#define FREQUENCY_COUNT 10
#define TRANSMITTER_HIGH_VALUE 1
#define TRANSMITTER_LOW_VALUE 0
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 10
#define BOUNCE_DELAY 5
#define TIME_DELAY 400
#define TEST_TICK_COUNT 200


// The transmitter state machine generates a square wave output at the chosen
// frequency as set by transmitter_setFrequencyNumber(). The step counts for the
// frequencies are provided in filter.h

// State of transmitter
typedef enum {
  NOT_TRANSMITTING,    // Setting transmitter to the initial state
  TRANSMITTING_HIGH, // Transmits high
  TRANSMITTING_LOW, // Transmits low
  INIT // Setting the timer back to 0 and reinitializing
} transmitter_state_t;


//Global Variables
volatile static transmitter_state_t transmitterState; //current state of Transmitter
volatile static uint16_t transmittingFrequency; //current output Frequency
volatile static uint16_t transmittingFrequencyModified; //Holds altered frequency if applicable to sync when not transmitting
volatile static uint16_t tickCountPeriod; //Timer to keep track of states
volatile bool continuousFlag; //Determines if the code should be run in continous format
volatile bool debugFlag; //Determines if debug outputs should be printed
static bool isCurrJedi;

// Standard init function.
void transmitter_init() {
    transmitterState = INIT; // Sets to init state
    tickCountPeriod = TRANSMITTER_PULSE_WIDTH; // Sets the timer to TRANSMITTER_PULSE_WIDTH
    mio_init(false);  // false disables any debug printing if there is a system failure during init.
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);  // Configure the signal direction of the pin to be an output.
    debugFlag = false; // Sets the debug flag to false
}

//Function that sets jf1 to zero
void transmitter_set_jf1_to_one() {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH_VALUE); // Write a '1' to JF-1.
}

//Function that sets jf1 to zero
void transmitter_set_jf1_to_zero() {
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW_VALUE); // Write a '0' to JF-1.
}

// Standard tick function.
void transmitter_tick() {
    static uint16_t timer = 0;

    //Transmiting Switch for Transitional Logic
    switch(transmitterState) //State update
    {
        case NOT_TRANSMITTING:   // transmitter is not emiting
            if (continuousFlag) { transmitterState = TRANSMITTING_HIGH; }
           break;

        case TRANSMITTING_HIGH:  // Transmitter is high
            //If timer is up, set transmitter state to INIT
            if (timer == tickCountPeriod && !isCurrJedi) { transmitterState = INIT; }
            //If transmitter is at frequency tick count, switch to low state
            else if (!(timer % (filter_frequencyTickTable[transmittingFrequency] / 2))) {
                transmitter_set_jf1_to_zero();
                transmitterState = TRANSMITTING_LOW;
                //Debug print
                if (debugFlag) {printf("\n");}

            }
            break;

        case TRANSMITTING_LOW:  // Transmitter is low

            //If timer is up, set transmitter state to INIT
            if (timer == tickCountPeriod && !isCurrJedi) { transmitterState = INIT; }
            //If transmitter is at frequency tick count, switch to low state
            else if (!(timer % (filter_frequencyTickTable[transmittingFrequency] / 2))) {
                transmitter_set_jf1_to_one();
                transmitterState = TRANSMITTING_HIGH; 
                //Debug print
                if (debugFlag) {printf("\n");}
            }
            break;

        case INIT:  // Transmitter in reinitializing
            transmitterState = NOT_TRANSMITTING;
            transmitter_set_jf1_to_zero();
            break;

        default:    //default case essentially an error state
            printf("No state\n");
    }   


    //Transmiting Switch for Action Logic
    switch(transmitterState) //State action
    {
        case NOT_TRANSMITTING:   // transmitter is not emiting
            transmittingFrequency = transmittingFrequencyModified;
            break;

        case TRANSMITTING_HIGH:  // Transmitter is high
            //Debug print
            if (debugFlag) {printf("1");}
            timer++;
            break;

        case TRANSMITTING_LOW:    // Transmitter is low
            //Debug print
            if (debugFlag) {printf("0");}
            timer++;
            break;

        case INIT:  // Transmitter in reinitializing
            //Reset timer to 0
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

void transmitter_stop(){
    transmitterState = INIT;
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

    //Set timer tick count and debug flag
    debugFlag = true;
    tickCountPeriod = TEST_TICK_COUNT;


    //Loop until button 3 is pressed
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {         // Run continuously until BTN3 is pressed.
            uint16_t switchValue = switches_read() % FILTER_FREQUENCY_COUNT;  // Compute a safe number from the switches.
            transmitter_setFrequencyNumber(switchValue);          // set the frequency number based upon switch value.
            transmitter_run();                                    // Start the transmitter.
            while (transmitter_running()) {                       // Keep ticking until it is done.
            transmitter_tick();                                 // tick.
            utils_msDelay(TRANSMITTER_TEST_TICK_PERIOD_IN_MS);  // short delay between ticks.
        }
        printf("\ncompleted one test period.\n");
    }

    //Debounce and wait for button 3 unpressed
    do {utils_msDelay(BOUNCE_DELAY);} while (buttons_read());
    printf("exiting transmitter_runTest()\n");
    
    //Reset timer tick count and debug flag
    tickCountPeriod = TRANSMITTER_PULSE_WIDTH;
    debugFlag = false;
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
    printf("starting transmitter_runTestNoncontinuos()\n");

    uint16_t switchesValue = 0;
    transmitter_setContinuousMode(false);

    //Loop until button 3 pressed
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) { 
        switchesValue = switches_read() % FILTER_FREQUENCY_COUNT; // Read the slide switches and use the numerical value of the switches as the frequency index. When all switches are in the down position (closest to the bottom of the board), you should generate the waveform for frequency 0. Sliding switch (SW0) upward would select frequency 1, and so forth.
        transmitter_setFrequencyNumber(switchesValue); // Set the frequency using the transmitter_setFrequencyNumber() function.
        transmitter_run(); //Invoke transmitter_run() and then wait for transmitter_running() to return false.
        
        //Loop until transmitter stops running
        while (transmitter_running()) { continue; }
        utils_msDelay(TIME_DELAY); // Use the utils_msDelay() function to delay for approximately 400 ms, long enough to demonstrate the operation of the transmitter on the oscilloscope.
    }
    //Loop until Button 3 is released
    while ((buttons_read() & BUTTONS_BTN3_MASK)) { continue; }
    printf("exiting transmitter_runTestNoncontinuos()\n");

}

void transmitter_isJedi(bool isJedi){
    isCurrJedi = isJedi;
}


// Tests the transmitter in continuous mode.
// To perform the test, connect the oscilloscope probe
// to the transmitter and ground probes on the development board
// prior to running this test.
// Transmitter should continuously generate the proper waveform
// at the transmitter-probe pin and change frequencies.
// in response to changes in the slide switches.
// Test runs until BTN3 is pressed.
// Depends on the interrupt handler to call tick function.
void transmitter_runTestContinuous() {
    printf("starting transmitter_runTestContinuos()\n");
    uint16_t switchesValue = 0;
    transmitter_setContinuousMode(true);
    transmitter_run(); // Invoke transmitter_setContinuousMode(true), transmitter_run() prior to entering an endless loop.

    // Loop until Button 3 is pressed:
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {  
        switchesValue = switches_read() % FILTER_FREQUENCY_COUNT;
        transmitter_setFrequencyNumber(switchesValue); //Set the frequency using the transmitter_setFrequencyNumber() function.
    }

    // Wait for the release of Button 3.
    while ((buttons_read() & BUTTONS_BTN3_MASK)) { continue; } 
    printf("exiting transmitter_runTestContinuos()\n");
}

