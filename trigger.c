

#include "trigger.h"
#include "transmitter.h"
#include "isr.h"
#include "mio.h"
#include "buttons.h"
#include <stdio.h>
#include <stdint.h>

#define INIT_ST_MSG "init state\n"
#define WAITING_FOR_CHANGE_ST_MSG "waiting_for_change_st\n"
#define DEBOUNCE_ST_MSG "Debounce_st\n"

#define ON true
#define OFF false
#define FULLY_LOADED 200
#define TRIGGER_GUN_TRIGGER_MIO_PIN 10
#define GUN_TRIGGER_PRESSED 1
#define GUN_TRIGGER_RELEASED 0
#define DEBOUCE_MAX 20
#define TRIGGER_PULLED true
#define TRIGGER_RELEASED false
#define RESET 0

static bool trigger_flag;
static bool ignoreGunInput;
static trigger_shotsRemaining_t ammunition;
static bool current_trigger;
static bool previous_trigger;
static uint8_t debounce_counter;

// Trigger can be activated by either btn0 or the external gun that is attached to TRIGGER_GUN_TRIGGER_MIO_PIN
// Gun input is ignored if the gun-input is high when the init() function is invoked.
bool triggerPressed() {
	return ((!ignoreGunInput & (mio_readPin(TRIGGER_GUN_TRIGGER_MIO_PIN) == GUN_TRIGGER_PRESSED)) || 
                (buttons_read() & BUTTONS_BTN0_MASK));
}

// Enable the trigger state machine. The trigger state-machine is inactive until
// this function is called. This allows you to ignore the trigger when helpful
// (mostly useful for testing).

void trigger_enable() 
{
    trigger_flag = ON;
}

// Disable the trigger state machine so that trigger presses are ignored.
void trigger_disable() 
{
    trigger_flag = OFF;
}

// Returns the number of remaining shots.
trigger_shotsRemaining_t trigger_getRemainingShotCount() 
{
    return ammunition;
}

// Sets the number of remaining shots.
void trigger_setRemainingShotCount(trigger_shotsRemaining_t ammunition) 
{
    //ammunition -1? 
    //call this after every trigger pull?
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// States for the trigger state machine.
enum trigger_st_t {
	init_st,                 // Start here, transition out of this state on the first tick.
	wait_for_change_st,        // Wait here until change in BTN0/MIO pin 10 reading
	debounce_st,    // waits here to debounce trigger change
};
static enum trigger_st_t currentState;

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
void debugStatePrint() {
  static enum trigger_st_t previousState;
  static bool firstPass = true;
  // Only print the message if:
  // 1. This the first pass and the value for previousState is unknown.
  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
  if (previousState != currentState || firstPass) {
    firstPass = false;                // previousState will be defined, firstPass is false.
    previousState = currentState;     // keep track of the last state that you were in.
    switch(currentState) {            // This prints messages based upon the state that you were in.
      case init_st:
        printf(INIT_ST_MSG);
        break;
      case wait_for_change_st:
        printf(WAITING_FOR_CHANGE_ST_MSG);
        break;
      case debounce_st:
        printf(DEBOUNCE_ST_MSG);
        break;
     }
  }
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void trigger_init() {
  mio_setPinAsInput(TRIGGER_GUN_TRIGGER_MIO_PIN);
  // If the trigger is pressed when trigger_init() is called, assume that the gun is not connected and ignore it.
  if (triggerPressed()) {
    ignoreGunInput = true;
  }
  currentState = init_st;
}

//standard tick function
void trigger_tick() {  

  //debugging function call
  debugStatePrint();

  // Perform state update first.
  switch(currentState) {
    case init_st:
    if(trigger_flag)
    {
      currentState = wait_for_change_st;
    }
      break;
    case wait_for_change_st:
      current_trigger = triggerPressed();
      if(current_trigger != previous_trigger)
      {
        debounce_counter = RESET;
        currentState = debounce_st;
      }
      break;
    case debounce_st:
      if(debounce_counter == DEBOUCE_MAX)
      {
        if(current_trigger == TRIGGER_PULLED)
          printf("D\n");
        else
          printf("U\n");
          previous_trigger = current_trigger;
          transmitter_run();
          currentState = wait_for_change_st;
      }
      break;
  }
  
  // Perform state action next.
  switch(currentState) {
    case init_st:
      previous_trigger = TRIGGER_RELEASED;
      break;
    case wait_for_change_st:
      break;
    case debounce_st:
    debounce_counter++;
      break;
  }  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


// Runs the test continuously until BTN1 is pressed.
// The test just prints out a 'D' when the trigger or BTN0
// is pressed, and a 'U' when the trigger or BTN0 is released.
void trigger_runTest() 
{
      // make a nice print out to the screen
    printf("starting trigger_runTest()\n\r");
    buttons_init(); // Using buttons
    trigger_init(); // init the transmitter.
    isr_init(); // init the isr
    // don't use if we're doing the trigger test
    // initialize the switch value variable here so we don't constantly make a new one
    isr_function();
    while (!(buttons_read() & BUTTONS_BTN1_MASK)) { // Run continuously until btn1 is pressed.}
    // make a nice print out to the screen
    printf("exiting trigger_runTest()\n\r");
}