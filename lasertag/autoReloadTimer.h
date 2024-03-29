#ifndef AUTORELOADTIMER_H_
#define AUTORELOADTIMER_H_

#include <stdbool.h>

// The auto-reload timer is always looking at the remaining shot-count from the
// trigger state-machine. When it goes to 0, it starts a configurable delay and
// after the delay expires, it sets the remaining shots to a specific value.

// Default, Defined in terms of 100 kHz ticks.
#define AUTO_RELOAD_EXPIRE_VALUE 300000

#define AUTO_RELOAD_SHOT_VALUE 10 // Default

// Need to init things.
void autoReloadTimer_init();

// Standard tick function.
void autoReloadTimer_tick();

// Calling this starts the timer.
void autoReloadTimer_quick();

// Returns true if the timer is currently running.
bool autoReloadTimer_running();

// Disables the autoReloadTimer and re-initializes it.
void autoReloadTimer_cancel();

#endif