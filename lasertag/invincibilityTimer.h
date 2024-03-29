#ifndef INVINCIBILITYTIMER_H_
#define INVINCIBILITYTIMER_H_

#include <stdbool.h>
#include <stdint.h>

// Perform any necessary inits for the invincibility timer.
void invincibilityTimer_init();

// Standard tick function.
void invincibilityTimer_tick();

// Calling this starts the timer.
void invincibilityTimer_start(uint32_t seconds);

// Returns true if the timer is running.
bool invincibilityTimer_running();

void invincibilityTimer_turnLedOn();

// Turns the gun's hit-LED off.
void invincibilityTimer_turnLedOff();
#endif /* INVINCIBILITYTIMER_H_ */
