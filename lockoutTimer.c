#include "lockoutTimer.h"
#include "isr.h"
#include <stdio.h>
#include <stdint.h>

#define COUNT_MAX 50000

static bool timer_on;
static bool timer_check;
static uint32_t count_val;

// Calling this starts the timer.
void lockoutTimer_start()
{
    timer_on = true;
}


// Returns true if the timer is running.
bool lockoutTimer_running()
{
  if(timer_on == true)
    return true;
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// States for the trigger state machine.
enum lockoutTimer_st_t {
	init_st,                 // Start here, transition out of this state on the first tick. 
	count_st                 // stays here for the duration of the count.
};
static enum lockoutTimer_st_t currentState;

//init function
//sets the mio pins somehow
void lockoutTimer_init()
{
    currentState = init_st;
}

//standard tick function
void lockoutTimer_tick() {  
  // Perform state update first.
  switch(currentState) {
    case init_st:
      if(timer_on == true)
      {
        currentState = count_st;
      }
      break;
    case count_st:
      if(count_val == COUNT_MAX)
      {
        timer_on = false;
        currentState = init_st;
      }
      break;
  }
  
  // Perform state action next.
  switch(currentState) {
    case init_st:
      count_val = 0;
      break;
    case count_st:
      count_val++;
      break;
  }  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
