#include <stdbool.h>
#include <stdio.h>
#include "autoReloadTimer.h"
#include "sound.h"
#include "trigger.h"


// State machine for invincibiltyTimer
enum autoReloadTimer_st_t {
	WAITING_RELOAD,                 // Start here, transition out of this state on the first tick.
	LOCKEDOUT
};

static volatile enum autoReloadTimer_st_t autoReload_s;
uint32_t tick_counter;

// Need to init things.
void autoReloadTimer_init(){
    trigger_setRemainingShotCount(AUTO_RELOAD_SHOT_VALUE);
    trigger_enable();
    autoReload_s =  WAITING_RELOAD;
    tick_counter = 0;
}

// Standard tick function.
void autoReloadTimer_tick(){
    switch(autoReload_s){ //State transition 
        case WAITING_RELOAD:
            if(trigger_getRemainingShotCount() == 0){
                trigger_disable();
                sound_playSound(sound_gunReload_e);
                autoReload_s = LOCKEDOUT;
                
            }
            break;
        case LOCKEDOUT:
            if(tick_counter == AUTO_RELOAD_EXPIRE_VALUE){
                tick_counter = 0;
                autoReload_s = WAITING_RELOAD;
                trigger_enable();
                trigger_setRemainingShotCount(AUTO_RELOAD_SHOT_VALUE);
            }
            break;
        default:
            break;
    }

    switch(autoReload_s) { //State Action
        case WAITING_RELOAD:
            tick_counter = 0;
            break;
        case LOCKEDOUT:
            tick_counter++;
            break;
        default:
            break;
    }
}

// Calling this starts the timer.
void autoReloadTimer_start(){
    sound_playSound(sound_gunReload_e);
    autoReload_s = LOCKEDOUT;
    tick_counter = 0;
}

// Returns true if the timer is currently running.
bool autoReloadTimer_running(){
    return autoReload_s == LOCKEDOUT;
}

// Disables the autoReloadTimer and re-initializes it.
void autoReloadTimer_cancel(){
    autoReload_s = WAITING_RELOAD;
}
