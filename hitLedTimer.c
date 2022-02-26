#include "hitLedTimer.h"
#include "isr.h"
#include "mio.h"
#include <stdio.h>
#include <stdint.h>

#define COUNT_MAX 50000
#define LED_ON 1
#define LED_OFF 0
#define LED_PIN_NUM 11

static bool led_timer_on;
static bool led_timer_check;
static bool led_timer_enable;
static uint32_t led_count_val;
static bool led_on;

// Calling this starts the timer.
void hitLedTimer_start()
{
    led_timer_on = true;
}


// Returns true if the timer is running.
bool hitLedTimer_running()
{
  if(led_timer_on == true)
    return true;
  else
    return false;
}

// Turns the gun's hit-LED on.
void hitLedTimer_turnLedOn()
{
    mio_writePIN(LED_PIN_NUM, LED_ON);
    ledswrite(LED_ON);
}

// Turns the gun's hit-LED off.
void hitLedTimer_turnLedOff()
{
    mio_writePIN(LED_PIN_NUM, LED_OFF);
    ledswrite(LED_OFF);
}

// Disables the hitLedTimer.
void hitLedTimer_disable()
{
    led_timer_enable = true;
}

// Enables the hitLedTimer.
void hitLedTimer_enable()
{
    led_timer_enable = false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// States for the trigger state machine.
enum hitLedTimer_st_t {
	init_st,                 // Start here, transition out of this state on the first tick. 
	blink_st                 // stays here for the duration of the count.
};
static enum hitLedTimer_st_t currentState;

//init function
//sets the mio pins somehow
void hitLedTimer_init()
{
    currentState = init_st;
}

//standard tick function
void hitLedTimer_tick() {  
  // Perform state update first.
  switch(currentState) {
    case init_st:
      if(led_timer_on && led_timer_enable)
      {
        hitLedTimer_turnLedOn();
        currentState = blink_st;
      }
      break;
    case blink_st:
      if(led_count_val == COUNT_MAX)
      {
        hitLedTimer_turnLedOff();
        led_timer_on = false;
        currentState = init_st;
      }
      break;
  }
  
  // Perform state action next.
  switch(currentState) {
    case init_st:
      led_count_val = 0;
      break;
    case blink_st:
      led_count_val++;
      break;
  }  
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
