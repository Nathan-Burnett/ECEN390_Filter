#include "isr.h"
#include "transmitter.h"
#include "trigger.h"
#include "hitLedTimer.h"
#include "lockoutTimer.h"
#include <stdio.h>
#include <stdint.h>
void isr_init()
{
    transmitter_init();
    trigger_init();
    hitLedTimer_init();
    lockoutTimer_init();
}

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function()
{
    //add tick functions in here
    transmitter_tick();
    trigger_tick();
    hitLedTimer_tick();
    lockoutTimer_tick();
}