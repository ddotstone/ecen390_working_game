#include <stdint.h>
#include <stdio.h>
#include "invincibilityTimer.h"
#include "intervalTimer.h"
#include "trigger.h"
#include "hitLedTimer.h"
#include "mio.h"
#include "leds.h"



#define FREQUENCY 100000
#define LED0_MASK 0x0001
#define TRANSMIT_HIGH 1

// State machine for invincibiltyTimer
enum invincibilityTimer_st_t {
	INIT,                 // Start here, transition out of this state on the first tick.
	INVINCIBLE,
    DISABLED
};

volatile static enum invincibilityTimer_st_t currentState; //Current State of Timer
volatile static uint32_t timerCount; //Tick Count for timer
volatile static uint32_t timerMaxValue; //Maximum ticks before reset timer
volatile static bool start; //Flag to start timer

// Perform any necessary inits for the invincibility timer.
void invincibilityTimer_init(){
    start = false;
    currentState = INIT;
    //Set Hit LED to output
    mio_setPinAsOutput(HIT_LED_TIMER_OUTPUT_PIN);

};

// Standard tick function.
void invincibilityTimer_tick(){
    //Transitions
    switch(currentState){
        //Init state
        case INIT:
            currentState = DISABLED;
            timerCount = 0;
            break;

        case INVINCIBLE:
        //Wait state, waiting for high signal
            if((timerCount >= timerMaxValue)){
                currentState = DISABLED;
                start = false;
                timerCount = 0;
                trigger_enable(); //Enable the Trigger
                hitLedTimer_enable();//Enable Hit Led
                invincibilityTimer_turnLedOff(); //Turn off LED

            }
            break;

        case DISABLED:
        //Lit state, LED is on
            if(start){
                currentState = INVINCIBLE;
                timerCount = 0;
                trigger_disable(); //Disable Trigger
                hitLedTimer_disable(); //Disable hitLedTimer
                invincibilityTimer_turnLedOn(); //Turn on the LED

            }
            break;
    }

    //Actions
    switch(currentState){
        //this one's basically fake
         case INIT:
            break;

        //Wait state, waiting for high signal
        case INVINCIBLE:
            timerCount++;
            break;

        //unlit state, LED is OFF
        case DISABLED:
            break;
    }
};

// Calling this starts the timer.
void invincibilityTimer_start(uint32_t seconds){
    start = true;
    hitLedTimer_disable(); //Disable hit LED Timer
    timerMaxValue = seconds * FREQUENCY;
};

// Returns true if the timer is running.
bool invincibilityTimer_running(){
    return start;
};


//Helper function to turn on Hit LED if invincibility
void invincibilityTimer_turnLedOn() {
    leds_write(LED0_MASK);
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, TRANSMIT_HIGH);
}

//Helper function to turn off Hit LED if not invincibile
void invincibilityTimer_turnLedOff() {
    leds_write(0);
    mio_writePin(HIT_LED_TIMER_OUTPUT_PIN, 0);
}