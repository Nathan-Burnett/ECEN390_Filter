
#include "trigger.h"
#include "interrupts.h"
#include "hitLedTimer.h"
#include "xparameters.h"
#include "transmitter.h"
#include "lockoutTimer.h"
#include "isr.h"

#define TIMER_PERIOD 1.0E-5 //Constant to make sure the board runs at 100 kHz
#define TIMER_CLOCK_FREQUENCY (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2) //Formula comes from documentation
#define TIMER_LOAD_VALUE ((TIMER_PERIOD * TIMER_CLOCK_FREQUENCY) - 1.0) //More documentation formula


// Performs inits for anything in isr.c
void isr_init()
{
    interrupts_initAll(true); //main interrupt init function.
    interrupts_setPrivateTimerLoadValue(TIMER_LOAD_VALUE); //Set the right timing value in the private timer
    interrupts_enableTimerGlobalInts(); //Enable global interrupts.
    interrupts_startArmPrivateTimer(); //Start the main ARM timer.
    interrupts_enableArmInts(); //Enable the ARM processor can see interrupts.
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function()
{
    //trigger_tick(); //Run the tick function for trigger
    //hitLedTimer_tick(); //Run the tick function for the hitLedTimer
    //transmitter_tick(); //Run the tick funciton for transmitter_tick
    lockoutTimer_tick();    //Run the tick function for lockoutTimer
}
