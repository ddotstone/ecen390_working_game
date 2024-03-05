#include <stdint.h>
#include <stdbool.h>
#include "trigger.h"
#include "transmitter.h"
#include "buttons.h"
#include "mio.h"

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

// The trigger state machine debounces both the press and release of gun
// trigger. Ultimately, it will activate the transmitter when a debounced press
// is detected.

#define TRIGGER_GUN_TRIGGER_MIO_PIN 10
#define DEBOUNCE_WAIT_TIME 5000
#define GUN_TRIGGER_PRESSED 1

typedef uint16_t trigger_shotsRemaining_t;
volatile bool ignoreGunInput;
volatile bool singleShot;

// State of trigger timer
typedef enum {
  INIT,    // Initializing trigger states
  WAIT, // debug button press
  DEBOUNCED_PRESS, //Wait for button low
  DEBOUNCE_RELEASE //Debounce button low
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

    //Transitional Logic for trigger state machine
    switch(triggerState) //State transition
    {
        case INIT:   // Setting timer to the initial state waiting for button press
            if (triggerPressed() && !disableTrigger) {
                triggerState = WAIT;
            }
            break;

        case WAIT:  // Debounce button press
            if (!triggerPressed()) {
                triggerState = INIT;
            }
            else if (timer == DEBOUNCE_WAIT_TIME) {
                triggerState = DEBOUNCED_PRESS;
                DPCHAR('D');
                DPCHAR('\n');
            }
            break;

        case DEBOUNCED_PRESS:    // Activate transmitter and wait for button release
            if (!triggerPressed()) {
                triggerState = DEBOUNCE_RELEASE;
            }
            break;

        case DEBOUNCE_RELEASE:    // Debounce button release
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

    //Action Logic for trigger state machine
    switch(triggerState){ //State action
        case INIT:   // Setting timer to the initial state waiting for button press
            singleShot = true;
            timer = 0;
            break;

        case WAIT:  // Debounce button press
            timer++;
            break;

        case DEBOUNCED_PRESS:    // Activate transmitter and wait for button release
            
            //Determine if trigger has been shot, if not shoots and sets singleShot to false
            if (singleShot) {
                transmitter_run();
                singleShot = false;
            }

            //Resets timer
            timer = 0;
            break;   

        case DEBOUNCE_RELEASE:  // Debounce button release
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
    disableTrigger = false;
    triggerState = INIT;
}

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() {
    disableTrigger = true;
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
