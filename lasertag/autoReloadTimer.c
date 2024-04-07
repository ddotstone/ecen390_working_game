#include <stdbool.h>
#include <stdio.h>
#include "autoReloadTimer.h"
#include "sound.h"
#include "trigger.h"


// State machine for autoReloadTimer
enum autoReloadTimer_st_t {
	WAITING_RELOAD,                 // Start here, transition out of this state on the first tick.
	LOCKEDOUT                       //Currently Locked Out
};

static volatile enum autoReloadTimer_st_t autoReload_s; //Current Timer State
uint32_t tick_counter; //Tick count

// Inits trigger enabled and load correct shot count
void autoReloadTimer_init(){
    trigger_setRemainingShotCount(AUTO_RELOAD_SHOT_VALUE);
    trigger_enable(); //Enable Trigger
    autoReload_s =  WAITING_RELOAD; //Start Timer in waiting for reload
    tick_counter = 0; //Reset Counter
}

// Standard tick function.
void autoReloadTimer_tick(){
    switch(autoReload_s){ //State transition 
        case WAITING_RELOAD:
            //If no shots left, transition to Lockedout and disable the trigger
            if(trigger_getRemainingShotCount() == 0){
                trigger_disable(); //Disable Trigger
                autoReload_s = LOCKEDOUT;
                
            }
            break;
        case LOCKEDOUT:
            //If the tick count has hit expire value, transition to waiting_reload and reenable trigger
            // as well as reset shotcount and play reload sound
            if(tick_counter == AUTO_RELOAD_EXPIRE_VALUE){
                tick_counter = 0;
                autoReload_s = WAITING_RELOAD;
                trigger_enable(); //Reenable Trigger
                trigger_setRemainingShotCount(AUTO_RELOAD_SHOT_VALUE); //Reset Shots
                sound_playSound(sound_gunReload_droid); //Play Reload Sound

            }
            break;
        default:
            break;
    }
    //Action State Machine for autoReload_s
    switch(autoReload_s) { //State Action
        case WAITING_RELOAD: 
            tick_counter = 0; //Reset tick
            break;
        case LOCKEDOUT:
            tick_counter++; //Increment 1 tick each call
            break;
        default:
            break;
    }
}

// Calling this starts starts a quick reload
void autoReloadTimer_quick(){
    sound_playSound(sound_gunReload_droid); //Play Reload Sound
    autoReload_s = WAITING_RELOAD; //Set state to waiting for Reload
    trigger_setRemainingShotCount(AUTO_RELOAD_SHOT_VALUE); // Set all remaing shots 
    tick_counter = 0; //Reset Tick Counter
}

// Returns true if the timer is currently running.
bool autoReloadTimer_running(){
    return autoReload_s == LOCKEDOUT;
}

// Disables the autoReloadTimer and re-initializes it.
void autoReloadTimer_cancel(){
    autoReload_s = WAITING_RELOAD;
}
