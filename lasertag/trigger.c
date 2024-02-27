#include <stdint.h>
#include <stdbool.h>
#include "trigger.h"
#include "transmitter.h"
#include "buttons.h"
#include "mio.h"

// Uncomment for debug prints
#define DEBUG
 
#if defined(DEBUG)
#include <stdio.h>
#include "xil_printf.h"
#define DPRINTF(...) printf(__VA_ARGS__)
#define DPCHAR(ch) outbyte(ch)
#else
#define DPRINTF(...)
#define DPCHAR(ch)
#endif

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.
#define TRIGGER_GUN_TRIGGER_MIO_PIN 10
#define DEBOUNCE_WAIT_TIME 5000
#define GUN_TRIGGER_PRESSED 1

typedef uint16_t trigger_shotsRemaining_t;
volatile bool ignoreGunInput;
volatile bool singleShot;

// State of lockout timer
typedef enum {
  INIT,    // Setting transmitter to the initial state
  WAIT, // Hit detected and lock out commences for .5 seconds
  DEBOUNCED_PRESS, // Setting the timer back to 0
  DEBOUNCE_RELEASE
} trigger_state_t;

volatile static trigger_state_t triggerState;
volatile bool disableTrigger;

// Trigger can be activated by either btn0 or the external gun that is attached to TRIGGER_GUN_TRIGGER_MIO_PIN
// Gun input is ignored if the gun-input is high when the init() function is invoked.
bool triggerPressed() {
	return ((!ignoreGunInput & (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) || 
                (buttons_read() & BUTTONS_BTN0_MASK));
}

// Init trigger data-structures.
// Initializes the mio subsystem.
// Determines whether the trigger switch of the gun is connected
// (see discussion in lab web pages).
void trigger_init() {
    disableTrigger = true;
    mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
    // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
    if (triggerPressed()) {
        ignoreGunInput = true;
    }
    triggerState = INIT;
}

// Standard tick function.
void trigger_tick() {
    static uint16_t timer = 0;
    switch(triggerState) //State transition
    {
        case INIT:   // Setting timer to the initial state waiting to get initial hit
            if (triggerPressed()) {
                triggerState = WAIT;
            }
            break;

        case WAIT:  // Hit detected and lock out commences for .5 seconds
            if (!triggerPressed() && !disableTrigger) {
                triggerState = INIT;
            }
            else if (timer == DEBOUNCE_WAIT_TIME) {
                triggerState = DEBOUNCED_PRESS;
                DPCHAR('D');
                DPCHAR('\n');
            }
            break;

        case DEBOUNCED_PRESS:    // Lock out time is completed and waiting for new hit
            if (!triggerPressed()) {
                triggerState = DEBOUNCE_RELEASE;
            }
            break;

        case DEBOUNCE_RELEASE:    // Lock out time is completed and waiting for new hit
            if (triggerPressed()) {
                triggerState = DEBOUNCED_PRESS;
            }
            else if (timer == DEBOUNCE_WAIT_TIME) {
                DPCHAR('U');
                DPCHAR('\n');
                triggerState = INIT;
            }
            break;

        default:    //default case
            printf("No state");
    }

     switch(triggerState) //State action
    {
        case INIT:   // Setting timer to the initial state waiting to get initial hit
            singleShot = true;
            timer = 0;
            break;

        case WAIT:  // Hit detected and lock out commences for .05 seconds
            timer++;
            break;

        case DEBOUNCED_PRESS:    // Lock out time is completed and waiting for new hit
            if (singleShot) {
                transmitter_run();
                singleShot = false;
            }
            timer = 0;
            break;   

        case DEBOUNCE_RELEASE:
            timer++;
            break;
        default:    //default case
            printf("No state");
    }
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).
void trigger_enable() {
    disableTrigger = true;
    ignoreGunInput = false;
    triggerState = INIT;
}

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() {
    disableTrigger = false;
    ignoreGunInput = true;
    triggerState = INIT;
}

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() {
    return 0;
}

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t count) {

}

// Runs the test continuously until BTN3 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
// Depends on the interrupt handler to call tick function.
void trigger_runTest() {
    printf("starting trigger_runTest()\n");
    trigger_enable();
    while (!(buttons_read() & BUTTONS_BTN3_MASK)) {}        // Run continuously until BTN3 is pressed. 
    while ((buttons_read() & BUTTONS_BTN3_MASK)) {} 
    printf("exiting trigger_runTest()\n");
    trigger_disable();


}
