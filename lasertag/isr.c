
#include <stdint.h>
#include <stdio.h>

#include "hitLedTimer.h"
#include "interrupts.h"
#include "lockoutTimer.h"
#include "transmitter.h"
#include "trigger.h"

#define ADC_BUFFER_SIZE 20 // 100000
#define RESET_VALUE 0
#define INCRAMENT 1

// This implements a dedicated circular buffer for storing values
// from the ADC until they are read and processed by detector().
// adcBuffer_t is similar to a queue.
typedef struct {
  uint32_t indexIn;               // New values go here.
  uint32_t indexOut;              // Pull old values from here.
  uint32_t elementCount;          // Number of elements in the buffer.
  uint32_t data[ADC_BUFFER_SIZE]; // Values are stored here.
} adcBuffer_t;

// This is the instantiation of adcBuffer.
volatile static adcBuffer_t adcBuffer;

// Init adcBuffer.
void adcBufferInit() {
  // loop through adcBuffer.data and set all values to 0
  for (uint32_t i = RESET_VALUE; i < ADC_BUFFER_SIZE; i++) {
    adcBuffer.data[i] = RESET_VALUE;
  }
  adcBuffer.indexIn = RESET_VALUE;
  adcBuffer.indexOut = RESET_VALUE;
  adcBuffer.elementCount = RESET_VALUE;
}

// Init everything in isr.
void isr_init() {
  adcBufferInit(); // Init the local adcBuffer.
                   // Call state machine init functions
  lockoutTimer_init();
  trigger_init();
  transmitter_init();
  hitLedTimer_init();
}

// Implemented as a fixed-size circular buffer.
// indexIn always points to an empty location (by definition).
// indexOut always points to the oldest element.
// Buffer is empty if indexIn == indexOut.
// Buffer is full if incremented indexIn == indexOut.
void isr_addDataToAdcBuffer(uint32_t adcData) {

  if (adcBuffer.elementCount <
      (ADC_BUFFER_SIZE -
       1)) // Increment the element count unless you are already full.
    adcBuffer.elementCount++;
  adcBuffer.data[adcBuffer.indexIn] = adcData; // write,
  adcBuffer.indexIn =
      (adcBuffer.indexIn + 1) % ADC_BUFFER_SIZE; // then increment.
  if (adcBuffer.indexIn ==
      adcBuffer.indexOut) { // If you are now pointing at the out pointer,
    adcBuffer.indexOut =
        (adcBuffer.indexOut + 1) %
        ADC_BUFFER_SIZE; // move the out pointer up (essentially a pop).
  }
}

// Removes a single item from the ADC buffer.
// Does not signal an error if the ADC buffer is currently empty
// Simply returns a default value of 0 if the buffer is currently empty.
uint32_t isr_removeDataFromAdcBuffer() {
  uint32_t returnValue = 0;
  if (adcBuffer.indexIn == adcBuffer.indexOut) // Just return 0 if empty.
    return 0;
  else {
    returnValue =
        adcBuffer.data[adcBuffer.indexOut]; // Not empty, get the return value
                                            // from buffer.
    adcBuffer.indexOut =
        (adcBuffer.indexOut + 1) % ADC_BUFFER_SIZE; // increment the out index.
    adcBuffer.elementCount--;                       // One less element.
  }
  return returnValue;
}

// Functional interface to access element count.
uint32_t isr_adcBufferElementCount() { return adcBuffer.elementCount; }

// This function is invoked by the timer interrupt at 100 kHz.
void isr_function() {
  // Put latest ADC value in adcBuffer
  uint32_t adcData = interrupts_getAdcData();
  isr_addDataToAdcBuffer(adcData);
  // Call state machine tick functions
  lockoutTimer_tick();
  trigger_tick();
  transmitter_tick();
  hitLedTimer_tick();
  sound_tick();
}
