#include "hitLedTimer.h"
#include "buttons.h"
#include "isr.h"
#include "leds.h"
#include "mio.h"
#include "trigger.h"
#include "utils.h"
#include <stdint.h>
#include <stdio.h>

#define COUNT_MAX 50000
#define LED_ON 1
#define LED_OFF 0
#define LED_PIN_NUM 11
#define DELAY_CONST 1000

volatile static bool led_timer_on;
volatile static bool led_timer_check;
volatile static bool led_timer_enable;
static uint32_t led_count_val;
volatile static bool led_on;

// Calling this starts the timer.
void hitLedTimer_start() { led_timer_on = true; }

// Returns true if the timer is running.
bool hitLedTimer_running() {
  // if the led is on, return true, else false
  if (led_timer_on == true)
    return true;
  else
    return false;
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn() {
  // writes led to high
  mio_writePin(LED_PIN_NUM, LED_ON);
  // writes other led to high
  leds_write(LED_ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff() {
  // write led to low
  mio_writePin(LED_PIN_NUM, LED_OFF);
  // writes other led to low
  leds_write(LED_OFF);
}

// Disables the hitLedTimer.
void hitLedTimer_disable() { led_timer_enable = false; }

// Enables the hitLedTimer.
void hitLedTimer_enable() { led_timer_enable = true; }

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// States for the trigger state machine.
enum hitLedTimer_st_t {
  init_st, // Start here, transition out of this state on the first tick.
  blink_st // stays here for the duration of the count.
};
static enum hitLedTimer_st_t currentState;

// init function
// sets the mio pins somehow
void hitLedTimer_init() {
  leds_init(false); // false disables debug errors
  mio_init(false);  // false disables any debug printing if there is a system
                    // failure during init.
  mio_setPinAsOutput(LED_PIN_NUM); // initializes the output pin
  currentState = init_st;
}

// standard tick function
void hitLedTimer_tick() {
  // Perform state update first.
  switch (currentState) {
  case init_st:
    // if we are enabled, move forward
    if (led_timer_on && led_timer_enable) {
      // printf("turned on\n");
      // turn the lights on
      hitLedTimer_turnLedOn();
      currentState = blink_st;
    }
    break;
  case blink_st:
    // if we hit our max counter
    if (led_count_val == COUNT_MAX) {
      // printf("turned off\n");
      // turn the lights off
      hitLedTimer_turnLedOff();
      led_timer_on = false; // resets flag
      currentState = init_st;
    }
    break;
  }
  // Perform state action next.
  switch (currentState) {
  case init_st:
    led_count_val = 0; // resets counter
    break;
  case blink_st:
    led_count_val++; // increments counter
    break;
  }
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Runs a visual test of the hit LED.
// The test continuously blinks the hit-led on and off.
void hitLedTimer_runTest() {
  // make a nice print out to the screen
  printf("starting hitLedTimer_runTest()\n\r");
  // buttons_init(); // Using buttons
  // trigger_init();
  hitLedTimer_init();

  // isr_init(); // init the isr
  // don't use if we're doing the trigger test
  // initialize the switch value variable here so we don't constantly make a new
  // one
  // isr_function();
  printf("right before test loop\n");
  while (!(buttons_read() &
           BUTTONS_BTN1_MASK)) { // Run continuously until btn1 is pressed.
    printf("made it into test looop\n");
    // Step 1: invoke hitLedTimer_start(),
    hitLedTimer_start();
    hitLedTimer_enable();
    // Step 2: wait until hitLedTimer_running() is false (use another while-loop
    // for this).
    while (!hitLedTimer_running())
      ;
    // Delay for 300 ms using utils_msDelay().
    utils_msDelay(DELAY_CONST);
    // Go back to Step 1.
  }
  // make a nice print out to the screen
  printf("exiting hitLedTimer_runTest()\n\r");
}