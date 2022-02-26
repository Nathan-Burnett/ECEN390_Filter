

#include <stdint.h>
#include <stdio.h>
#include "filter.h"
#include "mio.h"
#include "utils.h"
#include "isr.h"
#include "buttons.h"
#include "switches.h"
#include "transmitter.h"

//START DEFINE STATEMENTS
// 200 ms worth of data
#define TRANSMITTER_WAVEFORM_LENGTH 20000
// general initializer for various counters and other values
#define TRANSMITTER_INITIALIZER 0
// used to show that we want to run a transmission
#define TRANSMITTER_RUN_TRANSMISSION true
// used to run in continuous mode, used for testing purposes
#define TRANSMITTER_CONTINUOUS_MODE true
// when our transmitted signal is a one/high
#define TRANSMITTER_HIGH 1
// when our transmitted signal is a zero/low
#define TRANSMITTER_LOW 0
// we store the duty cycle value of our transmission
#define TRANSMITTER_DUTY_CYCLE 0.5
// the pin that we write to using mio
#define TRANSMITTER_OUTPUT_PIN 13
// used to suppress debug statements in mio
#define TRANSMITTER_SUPPRESS_PRINTS false
// This was used in an initial test of our code before using the interrupt method
#define TRANSMITTER_TEST_TICK_PERIOD_IN_MS 1
// This is to make sure that we are doing the right interval start and stops in non-continuous mode run
#define TRANSMITTER_TEST_LONG_DELAY 300
//END DEFINE STATEMENTS

//START GLOBALS, FLAGS AND OTHER VARS
// used to know what transmission frequency we want to spend
static uint16_t frequency_number;
// used as a flag to say that we should begin transmission
static volatile bool begin_transmitting;
// flag to show we are transmitting
static volatile bool is_transmitting;
// changed to determine whether we are in continuous mode or otherwise
static volatile bool continuous_mode;
// counter to see if we've transmitted the full 200 ms worth of data
static uint32_t waveform_transmit_counter;
// counter for sending the high and low bits of the signal
static uint16_t transmit_low_high_counter;
// the 1/high or 0/low that we're sending at a time
static uint16_t transmit_value;
// the frequency choice that we are currently sending
static uint16_t acting_frequency;
// used for enabling debug prints in a test mode
static bool test_mode_prints;
//END GLOBALS, FLAGS AND OTHER VARS

// This is a debug state print routine. It will print the names of the states each
// time tick() is called. It only prints states if they are different than the
// previous state.
//void debugStatePrint() {
//  static Control_st_t previousState;
//  static bool firstPass = true;
//  // Only print the message if:
//  // 1. This the first pass and the value for previousState is unknown.
//  // 2. previousState != currentState - this prevents reprinting the same state name over and over.
//  if (previousState != currentState || firstPass) {
//    firstPass = false;                // previousState will be defined, firstPass is false.
//    previousState = currentState;     // keep track of the last state that you were in.
//    switch(currentState){
//      case init_st:
//        printf("init_st\n\r");  //Print that it is in the init state
//        break;
//
//      case wait_st:
//          printf("wait_st\n\r");  //Print that it is in the init state
//          break;
//
//      case low_st:
//          printf("low_st\n\r");  //Print that it is in the init state
//          break;
//
//      case high_st:
//          printf("high_st\n\r");  //Print that it is in the init state
//          break;
//     }
//  }
//}


// The transmitter state machine generates a square wave output at the chosen frequency
// as set by transmitter_setFrequencyNumber(). The step counts for the frequencies
// are provided in filter.h

// States for the controller state machine.
enum transmitter_st_t {
    init_st, // Start here, transition out of this state on the first tick.
    wait_to_transmit_st, //waiting for signal to
    transmit_high_st, //timer to make sure the trigger is debounced and pulled
    transmit_low_st, //waiting for trigger release
} transmitter_currentState; // named specifically for this state machine


// Standard init function.
void transmitter_init(){
    // Initialize to the first frequency
    frequency_number = TRANSMITTER_INITIALIZER;
    // Start with transmitting low rather than high
    transmit_value = TRANSMITTER_LOW;
    // used to see whether we are transmitting or not
    is_transmitting = !TRANSMITTER_RUN_TRANSMISSION;
    // actual trigger to start the transmission
    begin_transmitting = !TRANSMITTER_RUN_TRANSMISSION;
    // counter initialized
    waveform_transmit_counter = TRANSMITTER_INITIALIZER;
    // counter initialized
    transmit_low_high_counter = TRANSMITTER_INITIALIZER;
    // init our input/output pins
    mio_init(TRANSMITTER_SUPPRESS_PRINTS);
    // we want this pin to be an output, not an input
    mio_setPinAsOutput(TRANSMITTER_OUTPUT_PIN);
    // and we want to make sure that we're transmitting low at first
    mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW);
    // normally we want to disable our debug prints
    test_mode_prints = TRANSMITTER_SUPPRESS_PRINTS;
}

// Starts the transmitter.
void transmitter_run(){
    // raise the flag for the state machine
    begin_transmitting = TRANSMITTER_RUN_TRANSMISSION;
}

// Returns true if the transmitter is still running.
bool transmitter_running(){
    return is_transmitting; //returns the flag from the state machine
}

void transmitter_setFrequencyNumber(uint16_t frequencyNumber)
{
    frequency_number = frequencyNumber;  //reset the global variable with the passed in variable
}

//Debug statemachine for troubleshooting
void debugTransmitterStatePrint() {
    static transmitter_st_t transmitter_previousState;
    static bool firstPass = true;
    // Only print the message if:
    // 1. This the first pass and the value for previousState is unknown.
    // 2. previousState != currentState - this prevents reprinting the same state name over and over.
    if (transmitter_previousState != transmitter_currentState || firstPass) {
        firstPass = false; // previousState will be defined, firstPass is false.
        transmitter_previousState = transmitter_currentState; // keep track of the last state that you were in.
        //printf("msCounter:%d\n\r", msCounter);
        switch(transmitter_currentState) { // This prints messages based upon the state that you were in.
        case init_st:
            printf("init_st\n\r");
            break;
        case wait_to_transmit_st:
            printf("wait_to_transmit_st\n\r");
            break;
        case transmit_high_st:
            printf("transmit_high_st\n\r");
            break;
        case transmit_low_st:
            printf("transmit_low_st\n\r");
            break;
        }
    }
}

// Standard tick function.
void transmitter_tick(){
    // check to see if prints are enabled
//    if (test_mode_prints)
//    {
//        // call the debug state print
//        debugTransmitterStatePrint();
//    }

    // Perform state update first. (Mealy)
    switch(transmitter_currentState) {
    case init_st:
        transmitter_currentState = wait_to_transmit_st; //Go into the wait state
        waveform_transmit_counter = TRANSMITTER_INITIALIZER;    //just in case it somehow didnt work, initialize the counter
        transmit_low_high_counter = TRANSMITTER_INITIALIZER; //same initialization to be sure
        break;

    case wait_to_transmit_st:
        // we stay here until we get a bump to transmit or we're in continuous mode
        if(begin_transmitting || continuous_mode)
        {
            transmitter_currentState = transmit_high_st;    //start by going to the transmit high
            acting_frequency = frequency_number;    //update the acting frequency with the set frequency number
            begin_transmitting = !TRANSMITTER_RUN_TRANSMISSION; // turn off our begin transmission flag
            is_transmitting = TRANSMITTER_RUN_TRANSMISSION; //turn on the fact that we're transmitting flag
            mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH); //set the output pin as high
        }
        break;

    case transmit_high_st:
        if (waveform_transmit_counter > TRANSMITTER_WAVEFORM_LENGTH) { // we have hit the end of the 200 ms transmission go back to wait for transmit state
            transmitter_currentState = wait_to_transmit_st; //go to wait to transmit
            is_transmitting = !TRANSMITTER_RUN_TRANSMISSION; //no longer transmitting
            waveform_transmit_counter = TRANSMITTER_INITIALIZER;    //reset the length counter
            transmit_low_high_counter = TRANSMITTER_INITIALIZER;    //reset the low high counter
            mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW);  //make sure our output pin is set to low
        }// also check to see if we've transmitted high enough
        else if (transmit_low_high_counter >=
                (filter_frequencyTickTable[acting_frequency]*TRANSMITTER_DUTY_CYCLE) ) { //if it has transmitted half the duty cycle, switch to transmit low
            transmitter_currentState = transmit_low_st; //transition to transmit low
            mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW);  //set transmitter value to low
            transmit_low_high_counter = TRANSMITTER_INITIALIZER; //reset our high low transmit counter
        }
        break;

    case transmit_low_st:
        if (waveform_transmit_counter > TRANSMITTER_WAVEFORM_LENGTH) { // we have hit the end of the our 200 ms transmission go back to wait for transmit state
            transmitter_currentState = wait_to_transmit_st; // go back to that state
            is_transmitting = !TRANSMITTER_RUN_TRANSMISSION; // no longer transmitting
            waveform_transmit_counter = TRANSMITTER_INITIALIZER;    // reset the length counter
            transmit_low_high_counter = TRANSMITTER_INITIALIZER;    // reset the high and low counter
            mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_LOW);  // reset the pin to low
        }
        else if (transmit_low_high_counter >=
                (filter_frequencyTickTable[acting_frequency]*TRANSMITTER_DUTY_CYCLE) ) { // if we have transmitted half the duty cycle, switch to transmit low
            transmitter_currentState = transmit_high_st; // transition back to transmit high
            mio_writePin(TRANSMITTER_OUTPUT_PIN, TRANSMITTER_HIGH);
            // set transmitter value to high
            transmit_low_high_counter = TRANSMITTER_INITIALIZER; // reset our high low transmit counter
        }
        break;

    default:
        // we should never go here
        printf("triggerControl_tick state update: hit default\n\r");
        break;
    }

    // Perform state action next.
    switch(transmitter_currentState) {
    case init_st:
        //no actions in the init state
        break;

    case wait_to_transmit_st:
        // no action in the wait state
        break;

    case transmit_high_st:
        waveform_transmit_counter++; // increment the counters for waveform transmission
        transmit_low_high_counter++; // increment the counters for high low counter.
        break;

    case transmit_low_st:
        waveform_transmit_counter++; // increment the counters for waveform transmission and high low counter.
        transmit_low_high_counter++; // increment the counters for high low counter.
        break;

    default:
        //Default catch all statement
        printf("trigger_tick state action: hit default\n\r");
        break;
    }
}


// Tests the transmitter.
void transmitter_runTest(){
    // make a nice print out to the screen
    printf("starting transmitter_runTest()\n\r");
    buttons_init(); // Using buttons
    switches_init(); // and switches.
    transmitter_init(); // init the transmitter.
    isr_init(); // init the isr
    // if we're doing continuous mode
    transmitter_setContinuousMode(true);
    // don't use if we're doing the trigger test
    transmitter_run();
    // initialize the switch value variable here so we don't constantly make a new one
    uint16_t switchValue;
    while (!(buttons_read() & BUTTONS_BTN1_MASK)) { // Run continuously until btn1 is pressed.
        switchValue = switches_read() % FILTER_FREQUENCY_COUNT; // Compute a safe number from the switches.
        transmitter_setFrequencyNumber(switchValue); // set the frequency number based upon switch value.
        // we use this run on the non-continuous mode, not needed for continuous mode transmitter_run();
        // use this on the non-continuous mode, not for continuous mode
        utils_msDelay(TRANSMITTER_TEST_LONG_DELAY);
    }
    // make a nice print out to the screen
    printf("exiting transmitter_runTest()\n\r");
}

// Runs the transmitter continuously.
// if continuousModeFlag == true, transmitter runs continuously, otherwise, transmits one waveform and stops.
// To set continuous mode, you must invoke this function prior to calling transmitter_run().
// If the transmitter is in currently in continuous mode, it will stop running if this function is
// invoked with continuousModeFlag == false. It can stop immediately or
// wait until the last 200 ms waveform is complete.
// NOTE: while running continuously, the transmitter will change frequencies at the end of each 200 ms waveform.
void transmitter_setContinuousMode(bool continuousModeFlag){
    continuous_mode = continuousModeFlag;
}

// This is provided for testing as explained in the transmitter section of the web-page. When enabled,
// debug prints are enabled to help to demonstrate the behavior of the transmitter.
void transmitter_enableTestMode(){
    //debug prints are printed
    test_mode_prints = !TRANSMITTER_SUPPRESS_PRINTS;
}

// This is provided for testing as explained in the transmitter section of the web-page. When disabled,
// debug prints that were previously enabled no longer appear.
void transmitter_disableTestMode(){
    // make sure that we don't print out debug statements
    test_mode_prints = TRANSMITTER_SUPPRESS_PRINTS;
}
