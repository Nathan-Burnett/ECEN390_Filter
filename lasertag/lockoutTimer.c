#include "lockoutTimer.h"
#include "buttons.h"
#include "intervalTimer.h"
#include "isr.h"
#include <stdint.h>
#include <stdio.h>

#define COUNT_MAX 50000
#define INTERVAL_COUNT_NUM 2

volatile static bool timer_on;
static bool timer_check;
static uint32_t count_val;

// Calling this starts the timer.
void lockoutTimer_start() { timer_on = true; }

// Returns true if the timer is running.
bool lockoutTimer_running() {
  // if running, return true, else false
  if (timer_on == true)
    return true;
  else
    return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// States for the trigger state machine.
enum lockoutTimer_st_t {
  init_st, // Start here, transition out of this state on the first tick.
  count_st // stays here for the duration of the count.
};
static enum lockoutTimer_st_t currentState;

// init function
// sets the mio pins somehow
void lockoutTimer_init() { currentState = init_st; }

// standard tick function
void lockoutTimer_tick() {
  // Perform state update first.
  switch (currentState) {
  case init_st:
    // if enabled, move on
    if (timer_on == true) {
      // printf("turned on\n");
      currentState = count_st;
    }
    break;
  case count_st:
    // if we hit the count max, turn the timer off
    if (count_val == COUNT_MAX) {
      timer_on = false; // reset flag
      // printf("turned off\n");
      currentState = init_st;
    }
    break;
  }
  // Perform state action next.
  switch (currentState) {
  case init_st:
    count_val = 0; // reset counter
    break;
  case count_st:
    count_val++; // increment counter
    break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Test function assumes interrupts have been completely enabled and
// lockoutTimer_tick() function is invoked by isr_function().
// Prints out pass/fail status and other info to console.
// Returns true if passes, false otherwise.
// This test uses the interval timer to determine correct delay for
// the interval timer.
bool lockoutTimer_runTest() {

  // isr_init();
  // isr_function();

  intervalTimer_reset(INTERVAL_COUNT_NUM);
  // Start an interval timer,
  intervalTimer_start(INTERVAL_COUNT_NUM);
  // Invoke lockoutTimer_start(),
  lockoutTimer_start();
  // Wait while lockoutTimer_running() is true (another while-loop),
  while (lockoutTimer_running()) {
  }
  // Once lockoutTimer_running() is false, stop the interval timer,
  intervalTimer_stop(INTERVAL_COUNT_NUM);
  // Print out the time duration from the interval timer.
  printf("%9.7e\n",
         intervalTimer_getTotalDurationInSeconds(INTERVAL_COUNT_NUM));
}