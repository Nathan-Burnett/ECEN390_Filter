/*
This software is provided for student assignment use in the Department of
Electrical and Computer Engineering, Brigham Young University, Utah, USA.
Users agree to not re-host, or redistribute the software, in source or binary
form, to other persons or other institutions. Users may modify and use the
source code for personal or educational use.
For questions, contact Brad Hutchings or Jeff Goeders, https://ece.byu.edu/
*/

#include "interrupts.h"
#include "runningModes.h"
#include "buttons.h"
#include "detector.h"
#include "display.h"
#include "filter.h"
#include "histogram.h"
#include "hitLedTimer.h"
#include "intervalTimer.h"
#include "isr.h"
#include "ledTimer.h"
#include "leds.h"
#include "lockoutTimer.h"
#include "mio.h"
#include "queue.h"
#include "sound.h"
#include "switches.h"
#include "transmitter.h"
#include "trigger.h"
#include "utils.h"
#include "xparameters.h"

#include <stdio.h>

#define RESET 0
#define SHOTS_PER_CLIP 10

#define INTERRUPTS_CURRENTLY_ENABLED true
#define INTERRUPTS_CURRENTLY_DISABLE false


//#define IGNORE_OWN_FREQUENCY

/*
This file (runningModes2.c) is separated from runningModes.c so that
check_and_zip.py does not include provided code for grading. Code for
submission can be added to this file and will be graded. The code in
runningModes.c can give you some ideas about how to implement other
modes here.
*/

void runningModes_twoTeams() {

  uint16_t hitCount = 0; // each player starts with 0 hits on them
  uint8_t lives = LIVES; // set each players life to 3
  uint8_t shotsInClip = SHOTS_PER_CLIP; // give the player a starting clip of 10
  uint16_t lockOutTimerCount = RESET; // this will be used so we can count the amount of times we have been waiting
  runningModes_initAll();
  // More initialization...
  bool ignoredFrequencies[FILTER_FREQUENCY_COUNT];
  for (uint16_t i = 0; i < FILTER_FREQUENCY_COUNT; i++)
    ignoredFrequencies[i] = false; // init all ignored frequencies to false
  
  #ifdef IGNORE_OWN_FREQUENCY
    printf("ignoring own frequency.\n");
    ignoredFrequencies[runningModes_getFrequencySetting()] = true; // set the current frequency setting to be ignored
  #endif

  detector_init(ignoredFrequencies); // init the detector with the ignoredFrequencies
  trigger_enable(); // enable the trigger

  interrupts_initAll(true); // init all interrupts
  interrupts_enableTimerGlobalInts(); // timer generates interrupts
  interrupts_startArmPrivateTimer(); // private time start
  interrupts_enableArmInts(); // ARM will now see interrupts

  lockoutTimer_start(); //start to miss all the shots from before the start of the game
  //play start up sound
  if (!sound_isBusy()) sound_playSound(sound_gameStart_e);


  // Implement game loop...
  while (lives > 0 ) {
    transmitter_setFrequencyNumber(runningModes_getFrequencySetting());
    detector(INTERRUPTS_CURRENTLY_ENABLED);
    if (detector_hitDetected()){
      detector_clearHit();
      hitCount++;
      if (hitCount == HITS_PER_LIFE){
        hitCount = RESET;
        --lives;
        // play lost life sound
      }
    }
  }
  interrupts_disableArmInts(); // Done with game loop, disable the interrupts.
  //hitLedTimer_turnLedOff();    // Save power :-)
  runningModes_printRunTimeStatistics(); // Print the run-time statistics to the
                                         // TFT.
  printf("Two-team mode terminated after detecting %d shots.\n", hitCount);
}


