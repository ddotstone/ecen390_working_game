
#include <stdbool.h>
#include <stdio.h>
#include "intervalTimer.h"
#include "lockoutTimer.h"

//setting defines for lockout timer
#define LOCKOUT_TIME .5

// State of lockout timer
typedef enum {
  INIT,    // Setting timer to the initial state waiting to get initial hit
  LOCKEDOUT, // Hit detected and lock out commences for .5 seconds
  UNLOCKED // Lock out time is completed and waiting for new hit
} lockoutTimer_state_t;

// Touchscreen names
volatile static lockoutTimer_state_t timerState;

// Creating the timer counter


// The lockoutTimer is active for 1/2 second once it is started.
// It is used to lock-out the detector once a hit has been detected.
// This ensures that only one hit is detected per 1/2-second interval.


// Perform any necessary inits for the lockout timer.
void lockoutTimer_init() {
    timerState = INIT;
}

// Standard tick function.
void lockoutTimer_tick() {
    static uint16_t timer = 0;
    switch(timerState) //State update
    {
        case UNLOCKED:   // Setting timer to the initial state waiting to get initial hit
           break;

        case LOCKEDOUT:  // Hit detected and lock out commences for .5 seconds
            if (timer == LOCKOUT_TIMER_EXPIRE_VALUE) {
                timerState = INIT;
            }
            break;

        case INIT:  // Lock out time is completed and waiting for new hit
            timerState = UNLOCKED;
            break;

        default:    //default case essentially an error state
            printf("No state\n");
    }   


    switch(timerState) //State action
    {
        case UNLOCKED:   // Setting timer to the initial state waiting to get initial hit
            timer = 0;
            break;

        case LOCKEDOUT:  // Hit detected and lock out commences for .5 seconds
            timer++;
            break;

        case INIT:    // Lock out time is completed and waiting for new hit
            break;

        default:    //default case
            printf("No state");
    }
}

// Calling this starts the timer.
void lockoutTimer_start() {
    timerState = LOCKEDOUT;
}

// Returns true if the timer is running.
bool lockoutTimer_running() {
    //printf("Checking Timerstate\n");
    return (timerState == LOCKEDOUT);
}

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {
    intervalTimer_init(INTERVAL_TIMER_TIMER_1);
    
    //waiting for timerState to change
    lockoutTimer_start();
    intervalTimer_start(INTERVAL_TIMER_TIMER_1);
    //waiting for timerState to change again
    while(true){
        if (!lockoutTimer_running()) {
            intervalTimer_stop(INTERVAL_TIMER_TIMER_1);

            //send whether or not the test passed
            printf("TEST COMPLETE. Running time : %f \n", intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_TIMER_1));
            return (intervalTimer_getTotalDurationInSeconds(INTERVAL_TIMER_TIMER_1) == LOCKOUT_TIME);
        }
    }
    
}