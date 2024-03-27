/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

/*
The code in runningModes.c can be an example for implementing the game here.
*/

#include <stdio.h>
#include <utils.h>

#include "hitLedTimer.h"
#include "interrupts.h"
#include "runningModes.h"
#include "mio.h"
#include "leds.h"
#include "buttons.h"
#include "switches.h"
#include "display.h"
#include "sound.h"
#include "invincibilityTimer.h"
#include "filter.h"
#include "trigger.h"
#include "lockoutTimer.h"
#include "transmitter.h"
#include "detector.h"
#include "utils.h"
#include "display.h"


#define TEAM_1 6
#define TEAM_2 9
#define LIVES 3
#define HEALTH 5
#define INVINCIBILTY_TIME 5
#define INTERRUPTS_CURRENTLY_ENABLED true
#define FILTER_OUT_TIME 500
#define SWITCH_1_MASK 0x1



// This game supports two teams, Team-A and Team-B.
// Each team operates on its own configurable frequency.
// Each player has a fixed set of lives and once they
// have expended all lives, operation ceases and they are told
// to return to base to await the ultimate end of the game.
// The gun is clip-based and each clip contains a fixed number of shots
// that takes a short time to reload a new clip.
// The clips are automatically loaded.
// Runs until BTN3 is pressed.
void game_twoTeamTag(void) {
  // Init
  uint16_t hitCount = 0;
  runningModes_initAll();
  detector_init();

  sound_setVolume(sound_minimumVolume_e);
  sound_playSound(sound_gameStart_e);

  // Configuration...
  uint16_t lives = LIVES;
  uint16_t health = HEALTH;
  
  
  // Get the result of Switch 0 to set the player frequency  
  uint16_t team = switches_read() & SWITCH_1_MASK ? TEAM_2 : TEAM_1;
  transmitter_setFrequencyNumber(team);
  transmitter_setContinuousMode(false);

  bool ignoredFrequencies[FILTER_FREQUENCY_COUNT];
  
  //Build ignored Frequencies array so that every frequency is ignored except for the enemy team
  for(int i = 0; i < FILTER_FREQUENCY_COUNT; i++){
    //Set ignored frequency to true for every frequency except for the frequency of the enemys team
    ignoredFrequencies[i] = (i != (team == TEAM_1? TEAM_2: TEAM_1));
  }

  detector_setIgnoredFrequencies(ignoredFrequencies);
  trigger_enable();
  transmitter_setFrequencyNumber(team);
  char teamNumber[10];
  sprintf(teamNumber,"Team %d", team);
  display_setCursor(100,100);
  display_print(teamNumber);

  interrupts_enableTimerGlobalInts(); // enable global interrupts.
  interrupts_startArmPrivateTimer();  // start the main timer.
  interrupts_enableArmInts(); // now the ARM processor can see interrupts.

  // Implement game loop...
  // Getting shot, shooting, keeping track of lives, and making 
  while(true){
    detector(INTERRUPTS_CURRENTLY_ENABLED);
    if(detector_hitDetected()){
      printf("hit\n");
      detector_clearHit();
      health--;
      if(health  <= 0){
        lives--;
        // If you are dead exit the game loop
        if(lives <= 0){
            printf("Game Over\n");
            sound_playSound(sound_gameOver_e);
            break;
        } else {
          printf("out of health\n");
          sound_playSound(sound_loseLife_e);
          invincibilityTimer_start(INVINCIBILTY_TIME);
          health = HEALTH;
        }
      }
      else{
        sound_playSound(sound_hit_e);

      }
      printf("Lives: %d Health: %d\n",lives,health);
    }
  }
  trigger_disable();
  runningModes_printRunTimeStatistics(); // Print the run-time statistics.
  utils_msDelay(FILTER_OUT_TIME);

// Yell at the player to return to the base forever
  while(!(buttons_read() & BUTTONS_BTN3_MASK)){
    printf("PLAY SILENCE\n"); 
    while(sound_isBusy());
    sound_playSound(sound_oneSecondSilence_e);

    printf("PLAY GO BACK TO BASE\n");
    while(sound_isBusy());
    sound_playSound(sound_returnToBase_e);
    }
  // End game loop...
  interrupts_disableArmInts(); // Done with game loop, disable the interrupts.
}

