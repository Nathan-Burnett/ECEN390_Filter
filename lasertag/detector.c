

#include "detector.h"
#include "filter.h"
#include "hitLedTimer.h"
#include "interrupts.h"
#include "lockoutTimer.h"
#include <stdio.h>

#define ADC_SCALE_FACTOR 2047.5
#define ADC_SCALE_HALF 2047
#define ADC_SCALE_FULL 4095
#define MEDIAN_INDEX 4
#define SORTED_ARRAY_SIZE FILTER_FREQUENCY_COUNT

// debug stuff
const static double POWER_TEST_NO_HIT_VALS[] = {
    0.351741, 0.199679, 0.147244, 0.145901, 0.147286,
    0.324328, 0.240733, 0.712381, 0.384333, 0.989865};
const static double POWER_TEST_HIT_VALS[] = {
    0.351741, 0.199679, 0.147244,  150,       0.147286,
    0.324328, 0.240733, 0.0712381, 0.0384333, 0.0989865};
#define POWER_TEST_HIT 1
#define POWER_TEST_NO_HIT 2
#define DEBUG_OFF 0

static bool ignoredFreq[FILTER_FREQUENCY_COUNT];
static bool ignoreHits;
static bool hitDetected;
static uint32_t hitCounts[FILTER_FREQUENCY_COUNT];
static uint8_t lastHitFrequency;
static uint8_t fudgeFactorIndex;

static const uint16_t FUDGE_FACTORS[] = {1000, 20, 30};

// Always have to init things.
// bool array is indexed by frequency number, array location set for true to
// ignore, false otherwise. This way you can ignore multiple frequencies.
void detector_init(bool ignoredFrequencies[]) {
  // inits some arrays
  for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i) {
    // copies values from ignoredFrequencies
    ignoredFreq[i] = ignoredFrequencies[i];
    // sets all hitCounts to 0
    hitCounts[i] = 0;
  }
  fudgeFactorIndex = 0;
  hitLedTimer_enable();
  ignoreHits = false;
  hitDetected = false;
  filter_init();
  lastHitFrequency = 0;
}

// runs detection algorithm.
void detectHit(uint8_t debugMode) {
  double sortedPowerValues[FILTER_FREQUENCY_COUNT];
  double powerValues[FILTER_FREQUENCY_COUNT];

  uint32_t maxPowerFreqNumber;

  // if debugmode, use predefined values
  if (debugMode == POWER_TEST_HIT) {
    // copies array
    for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i) {
      powerValues[i] = POWER_TEST_HIT_VALS[i];
    }
  } else if (debugMode == POWER_TEST_NO_HIT) {
    // copies array
    for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i) {
      powerValues[i] = POWER_TEST_NO_HIT_VALS[i];
    }
  } else { // run power for non test array
    filter_getCurrentPowerValues(powerValues);
  }
  detector_sort(
      &maxPowerFreqNumber, powerValues,
      sortedPowerValues); // run power sort for the incoming adc values

  // PROBLEM : if the max frequency is an ignored frequency, but there's another
  // frequency that
  // isn't ignored that's high enough, we would miss that.
  // if the max power value is greater than the median times the fudgeFactor,
  // then it's a hit
  if (powerValues[maxPowerFreqNumber] >=
      sortedPowerValues[MEDIAN_INDEX] * FUDGE_FACTORS[fudgeFactorIndex]) {
    // it's a hit!!!
    lockoutTimer_start(); // start lockout
    hitLedTimer_start();  // start timer for led
    // printf("maxPowerFreqNumber = %u\n", maxPowerFreqNumber);
    ++hitCounts[maxPowerFreqNumber];
    hitDetected = true;
    lastHitFrequency = maxPowerFreqNumber;
  }
}

// Runs the entire detector: decimating fir-filter, iir-filters,
// power-computation, hit-detection. if interruptsNotEnabled = true, interrupts
// are not running. If interruptsNotEnabled = true you can pop values from the
// ADC queue without disabling interrupts. If interruptsNotEnabled = false, do
// the following:
// 1. disable interrupts.
// 2. pop the value from the ADC queue.
// 3. re-enable interrupts if interruptsNotEnabled was true.
// if ignoreSelf == true, ignore hits that are detected on your frequency.
// Your frequency is simply the frequency indicated by the slide switches
void detector(bool interruptsCurrentlyEnabled) {
  uint32_t elementCount = isr_adcBufferElementCount(); // add value to buffer

  // runs filter elementCount times
  for (uint32_t i = 0; i < elementCount; ++i) {

    isr_AdcValue_t rawAdcValue; // holds ADC output

    // gets ADC value and toggles interrupts off/on
    if (interruptsCurrentlyEnabled) {
      interrupts_disableArmInts();                 // disable int
      rawAdcValue = isr_removeDataFromAdcBuffer(); // get value
      interrupts_enableArmInts();                  // re-enable int

    }
    // else it just gets the ADC value
    else {
      rawAdcValue = isr_removeDataFromAdcBuffer(); // pop value from queue
    }

    // scales it from 0 to 4095 to -1.0 to 1.0
    double scaledAdcValue =
        detector_getScaledAdcValue(rawAdcValue); // scale value

    filter_addNewInput(scaledAdcValue); // add value to queue
    static uint8_t filterInputCount = 0;
    ++filterInputCount;

    // if we added 10 new values, time to filter! (thereby decimating it)
    if (filterInputCount ==
        filter_getDecimationValue()) { // if at decimation value roll over
      filterInputCount = 0;
      // runs firFilter
      filter_firFilter();
      // runs iir filters for each channel
      for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i)
        filter_iirFilter(i);
      // computes power for each channel
      for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i)
        filter_computePower(i, false, false);

      // if the lockout timer isn't running and we're not ignoring all hits,
      // run the hit detection algorithm
      if (!lockoutTimer_running() && !ignoreHits) {
        detectHit(DEBUG_OFF); // find if a hit
      }
    }
  }
}

// Returns true if a hit was detected.
bool detector_hitDetected() { return hitDetected; }

// Returns the frequency number that caused the hit.
uint16_t detector_getFrequencyNumberOfLastHit() { return lastHitFrequency; }

// Clear the detected hit once you have accounted for it.
void detector_clearHit() { hitDetected = false; }

// Ignore all hits. Used to provide some limited invincibility in some game
// modes. The detector will ignore all hits if the flag is true, otherwise will
// respond to hits normally.
void detector_ignoreAllHits(bool flagValue) { ignoreHits = flagValue; }

// Get the current hit counts.
// Copy the current hit counts into the user-provided hitArray
// using a for-loop.
void detector_getHitCounts(detector_hitCount_t hitArray[]) {
  // copies hitCounts into hitArray
  for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i) {
    hitArray[i] = hitCounts[i];
  }
}

// Allows the fudge-factor index to be set externally from the detector.
// The actual values for fudge-factors is stored in an array found in detector.c
void detector_setFudgeFactorIndex(uint32_t fudgeFactor) {
  fudgeFactorIndex = fudgeFactor;
}

// This function sorts the inputs in the unsortedArray and
// copies the sorted results into the sortedArray. It also
// finds the maximum power value and assigns the frequency
// number for that value to the maxPowerFreqNo argument.
// This function also ignores a single frequency as noted below.
// if ignoreFrequency is true, you must ignore any power from frequencyNumber.
// maxPowerFreqNo is the frequency number with the highest value contained in
// the unsortedValues. unsortedValues contains the unsorted values. sortedValues
// contains the sorted values. Note: it is assumed that the size of both of the
// array arguments is 10.
detector_status_t detector_sort(uint32_t *maxPowerFreqNo,
                                double unsortedValues[],
                                double sortedValues[]) {
  // Perform insertion sort

  double placeholder = 0.0;

  // copies unsortedValues into sortedValues
  for (uint8_t i = 0; i < FILTER_FREQUENCY_COUNT; ++i) {
    sortedValues[i] = unsortedValues[i];
  }

  // you have to set this at 0 as default else it will never be set in loops
  *maxPowerFreqNo = 0;

  // sorts in theory
  for (uint8_t i = 0; i < SORTED_ARRAY_SIZE; i++) { // marker for sorted array
    for (uint8_t j = i; j < SORTED_ARRAY_SIZE;
         j++) { // begin sorting by replacing values in each array
      if (sortedValues[j] >
          sortedValues[i]) { // run through second array checking each value
        placeholder = sortedValues[i];
        sortedValues[i] = sortedValues[j];
        sortedValues[j] = placeholder;
        if (i ==
            0) { // if there is no other values then the first is the greatest
          *maxPowerFreqNo = j;
        }
      }
    }
  }
  return DETECTOR_STATUS_OK;
}

// Encapsulate ADC scaling for easier testing.
double detector_getScaledAdcValue(isr_AdcValue_t adcValue) {
  return adcValue / ADC_SCALE_FACTOR - 1.0;
}

/*******************************************************
 ****************** Test Routines **********************
 ******************************************************/

// Students implement this as part of Milestone 3, Task 3.
void detector_runTest() {
  printf("Running test with hit values\n");
  detectHit(POWER_TEST_HIT); // detect hit

  // if there's a hit, it says hit detected.
  if (detector_hitDetected()) { // if hit
    printf("Hit detected!\n");
  } else { // if not hit
    printf("Hit not detected!\n");
  }
  detector_clearHit(); // clear hit

  printf("Running test with no hit values\n");

  detectHit(POWER_TEST_NO_HIT); // detect hit

  // if there's a hit, it says hit detected.
  if (detector_hitDetected()) { // if hit
    printf("Hit detected!\n");
  } else { // if not hit
    printf("Hit not detected!\n");
  }
  detector_clearHit(); // clear hit
}

// Returns 0 if passes, non-zero otherwise.
// if printTestMessages is true, print out detailed status messages.
// detector_status_t detector_testSort(sortTestFunctionPtr testSortFunction,
// bool printTestMessages);
detector_status_t detector_testAdcScaling() {
  printf("0 scaled: %f\n", detector_getScaledAdcValue(DEBUG_OFF));
  printf("2047 scaled: %f\n", detector_getScaledAdcValue(ADC_SCALE_HALF));
  printf("4095 scaled: %f\n", detector_getScaledAdcValue(ADC_SCALE_FULL));
}
