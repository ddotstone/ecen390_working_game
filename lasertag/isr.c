#include "lockoutTimer.h"
#include "transmitter.h"
#include "trigger.h"
#include "hitLedTimer.h"
#include "buffer.h"
#include "interrupts.h"
#include "autoReloadTimer.h"
#include "invincibilityTimer.h"
#include "sound.h"
// The interrupt service routine (ISR) is implemented here.
// Add function calls for state machine tick functions and
// other interrupt related modules.

// Perform initialization for interrupt and timing related modules.
void isr_init() {
    lockoutTimer_init();
    transmitter_init();
    trigger_init();
    hitLedTimer_init();
    buffer_init();
    sound_init();
    autoReloadTimer_init();
    invincibilityTimer_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {
    lockoutTimer_tick();
    transmitter_tick();
    trigger_tick();
    hitLedTimer_tick();
    buffer_pushover(interrupts_getAdcData());
    sound_tick();
    autoReloadTimer_tick();
    invincibilityTimer_tick();

}
