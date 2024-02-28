#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "leds.h"
#include "mio.h"
#include "hitLedTimer.h"
#include "utils.h"
#include "buttons.h"

//setting defines for lockout timer
#define LOCKOUT_TIME .5
#define LED0_MASK 0x0001
#define HIT_TIMER_EXPIRE_VALUE 50000
#define WAIT_TIME 300
#define TRANSMIT_HIGH 1

// State of lockout timer
typedef enum {
  INIT,    // Initializer
  LED_ON, // LED high
  LED_OFF // LED low and wait for hig
} hitTimer_state_t;


volatile static hitTimer_state_t timerState;
volatile bool ledTimerEnabled;

// The hitLedTimer is active for 1/2 second once it is started.
// While active, it turns on the LED connected to MIO pin 11
// and also LED LD0 on the ZYBO board.

// Need to init things.
void hitLedTimer_init() {
    timerState = INIT;
    mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);
    ledTimerEnabled = true;
}

// Standard tick function.
void hitLedTimer_tick(){
     static uint16_t timer = 0;
    
    //Transitional Logic for timerState
    switch(timerState) //State update
    {
        case LED_OFF:   // Setting timer to the initial state waiting to get initial hit
           break;

        case LED_ON:  // Hit detected LED high for 500 ms

            // If timer reaches expire tick count, transition to init state
            if (timer == HIT_TIMER_EXPIRE_VALUE) {
                timerState = INIT;
                hitLedTimer_turnLedOff();
            }
            break;

        case INIT:  // Reinitializes values
            timerState = LED_OFF;
            break;

        default:    //default case essentially an error state
            printf("No state\n");
    }   

    //Action Logic for timerState
    switch(timerState) //State action
    {
        case LED_OFF:   // Setting timer to the initial state waiting to get initial hit
            timer = 0;
            break;

        case LED_ON:  // Hit detected LED high for 500 ms
            timer++;
            break;

        case INIT:    // Reinitializes values
            break;

        default:    //default case
            printf("No state");
    }
}

// Calling this starts the timer.
void hitLedTimer_start() {
    if (ledTimerEnabled) {
        hitLedTimer_turnLedOn();
        timerState = LED_ON;
    }
}

// Returns true if the timer is currently running.
bool hitLedTimer_running() {
    return (timerState == LED_ON);
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
    leds_write(LED0_MASK);
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, TRANSMIT_HIGH);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
    leds_write(0);
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, 0);
}

// Disables the hitLedTimer.
void hitLedTimer_disable() {
    ledTimerEnabled = false;
}

// Enables the hitLedTimer.
void hitLedTimer_enable() {
    ledTimerEnabled = true;
}

// Runs a visual test of the hit LED until BTN3 is pressed.
// The test continuously blinks the hit-led on and off.
// Depends on the interrupt handler to call tick function.
void hitLedTimer_runTest() {
    printf("starting hitLedTimer_runTest()\n");

    //Wait for button 3 press
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) { 
        hitLedTimer_start();
        while (hitLedTimer_running()) {}
        utils_msDelay(WAIT_TIME);
    } 

    //Wait for button 3 release
    while ((buttons_read() & BUTTONS_BTN3_MASK)) {} 
    printf("exiting hitLedTimer_runTest()\n");

}
