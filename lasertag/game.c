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
#include "isr.h"
#include "display.h"
#include "intervalTimer.h"


#define TEAM_1 6
#define TEAM_2 9
#define LIVES 3
#define HEALTH_JEDI 5
#define HEALTH_DROID 1
#define INVINCIBILTY_TIME 5
#define INTERRUPTS_CURRENTLY_ENABLED true
#define FILTER_OUT_TIME 500
#define SWITCH_1_MASK 0x1
#define CENTER_SCREEN 90,80
#define LIVES_TEXT 20,220
#define HEALTH_TEXT 20,180
#define GAME_OVER_TEXT_LOC 80,120
#define GAME_OVER_TEXT "Game Over"
#define CHAR_SPACES 10
#define TEXT_SIZE 3
#define GO_TEXT_SIZE 4

uint16_t lives;
uint16_t health;
uint16_t prevHealth;
uint16_t prevLives;
uint16_t team;

//Helper function to print Health and Lives to screen
//Will clear old health and lives and print new
static void printHealthLives();
//Helper function to initialize all necissary files for game to run
static void initializers_all();
//Helper function to initially print all information to the screen
static void initialize_diplay();


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
  initializers_all();
  //Set Game Volume and also Set start Sound
  sound_setVolume(sound_mediumHighVolume_e);
  sound_playSound(sound_gameStart_e);

  // Configuration Lives
  
  // Get the result of Switch 0 to set the player frequency  
  team = switches_read() & SWITCH_1_MASK ? TEAM_2 : TEAM_1;
  transmitter_setFrequencyNumber(team);

  //Run in Single Shooter Mode

  //Build ignored Frequencies array so that every frequency is ignored except for the enemy team
  bool ignoredFrequencies[FILTER_FREQUENCY_COUNT];
  for(int i = 0; i < FILTER_FREQUENCY_COUNT; i++){
    //Set ignored frequency to true for every frequency except for the frequency of the enemys team
    ignoredFrequencies[i] = (i != (team == TEAM_1? TEAM_2: TEAM_1));
  }
  bool isTeamOne = (team==TEAM_1);
  transmitter_setContinuousMode(isTeamOne);
  detector_setIgnoredFrequencies(ignoredFrequencies);

  //Enable Trigger and Set Frequency Number
  trigger_enable();
  transmitter_setFrequencyNumber(team);

  initialize_diplay();

  //Begin Interrupts and Start Timers
  interrupts_enableTimerGlobalInts(); // enable global interrupts.
  interrupts_startArmPrivateTimer();  // start the main timer.
  interrupts_enableArmInts(); // now the ARM processor can see interrupts.
  lives = LIVES;
  health = isTeamOne ? HEALTH_JEDI:HEALTH_DROID;
  //Wait till staring sound over and clear any hits it may cause
  while(sound_isBusy()){};
  detector_clearHit();

  // Checks for Shots, handles hits, keeps track of lives and health.
  while(true){
    //Run Detector Function
    detector(INTERRUPTS_CURRENTLY_ENABLED);
    //If hit detected, handle lives and health
    if(detector_hitDetected()){
      printf("hit\n"); //Debug Print 
      detector_clearHit(); //Clear Hit if Registered
      health--; //Decrement Health

      //If health is equal to zero, decrease lives
      if(health  <= 0){ 
        lives--; //Decrement Lives
        // If you are dead exit the game loop
        if(lives <= 0){
            printf("Game Over\n"); //Debug Print
            sound_playSound(sound_gameOver_e); //Play Game Over Sound
            break; //If game over break out of game loop
        } else {
          printf("out of health\n"); //Debug Print
          sound_playSound(sound_loseLife_e); //Play loseLife sound
          invincibilityTimer_start(INVINCIBILTY_TIME); //Start Invincibility Time
          health = isTeamOne ? HEALTH_JEDI:HEALTH_DROID; //Reset Health
        }
      }
      else{
        sound_playSound(sound_hit_e); //If No death do Sound_Hit

      }
      //If the health has changed, reprint to display
      if (health != prevHealth){
        printHealthLives(); 
      }
      //Debug Pring the Lives and health
      printf("Lives: %d Health: %d\n",lives,health);
    }
  }

  //If game over disable trigger, allow any sounds to finish playing, and write
  //game over to screen
  trigger_disable();
  utils_msDelay(FILTER_OUT_TIME);
  display_fillScreen(DISPLAY_BLACK);
  display_setTextSize(GO_TEXT_SIZE);
  display_setCursor(GAME_OVER_TEXT_LOC);
  display_setTextColor(DISPLAY_WHITE);
  display_print(GAME_OVER_TEXT);

// Yell at the player to return to the base forever
  while(!(buttons_read() & BUTTONS_BTN3_MASK)){
    
    while(sound_isBusy());    //Wait for sound to end
    sound_playSound(sound_oneSecondSilence_e); //Play One Second Silence

    while(sound_isBusy()); // Wait for sound to end
    sound_playSound(sound_returnToBase_e); //Play Return to base

    }
  // End game loop...
  interrupts_disableArmInts(); // Done with game loop, disable the interrupts.
}


//Helper function to print Health and Lives to screen
//Will clear old health and lives and print new
static void printHealthLives(){

  //Clear Health
  char healthNumber[CHAR_SPACES];
  sprintf(healthNumber,"Health: %d", prevHealth);
  display_setCursor(HEALTH_TEXT);
  display_setTextColor(DISPLAY_BLACK);
  display_print(healthNumber);
  
  //Clear Lives
  char livesNumber[CHAR_SPACES];
  sprintf(livesNumber,"Lives: %d", prevLives);
  display_setCursor(LIVES_TEXT);
  display_setTextColor(DISPLAY_BLACK);
  display_print(livesNumber);

  //Print new health
  sprintf(healthNumber,"Health: %d", health);
  display_setCursor(HEALTH_TEXT);
  display_setTextColor(DISPLAY_MAGENTA);
  display_print(healthNumber);
  
  //Pring new lives
  sprintf(livesNumber,"Lives: %d", lives);
  display_setCursor(LIVES_TEXT);
  display_setTextColor(DISPLAY_GREEN);
  display_print(livesNumber);
  prevHealth = health;
  prevLives = lives;
}


//Helper function to initialize all necissary files for game to run
static void initializers_all(){
  detector_init();
  filter_init();
  detector_init();
  // isr_init() should include calls to: transmitter, trigger,
  // hitLedTimer, lockoutTimer, sound, and buffer init
  isr_init();
  intervalTimer_initAll();
  // Init all interrupts (but does not enable the interrupts at the devices).
  // Call last
  interrupts_initAll(false); // A true argument enables error messages

}

//Helper function to initially print all information to the screen
static void initialize_diplay(){

  //Clear Screen, Set text Size
  display_fillScreen(DISPLAY_BLACK);
  display_setTextSize(TEXT_SIZE);

  //Print Team Number
  char teamNumber[CHAR_SPACES];
  sprintf(teamNumber,"Team %d", team);
  display_setCursor(CENTER_SCREEN);
  display_setTextColor(DISPLAY_WHITE);
  display_print(teamNumber);
  
  //Print health Number
  char healthNumber[CHAR_SPACES];
  sprintf(healthNumber,"Health: %d", health);
  display_setCursor(HEALTH_TEXT);
  display_setTextColor(DISPLAY_MAGENTA);
  display_print(healthNumber);

  //Print Lives Number
  char livesNumber[CHAR_SPACES];
  sprintf(livesNumber,"Lives: %d", lives);
  display_setCursor(LIVES_TEXT);
  display_setTextColor(DISPLAY_GREEN);
  display_print(livesNumber);
}

