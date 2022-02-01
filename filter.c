
#include "filter.h"
#include <stdint.h>

#define FIR_COEF_COUNT 81
#define IIR_A_COEFF_COUNT 11
#define IIR_B_COEFF_COUNT 10
#define Z_QUEUE_SIZE IIR_A_COEFF_COUNT // dont really know how much there is supposed to be I think 11
#define FILTER_IIR_FILTER_COUNT 10
#define QUEUE_INIT_VALUE 0.0

//Global Variables

const static queue_t xQue,yQue; // x and y queue declaration
const static queue_t zQue[FILTER_IIR_FILTER_COUNT]; // the 10 z queues deleration 
const static queue_t outQue[FILTER_IIR_FILTER_COUNT]; // the 10 output queues declaration
const static double fir_coeff[FIR_COEF_COUNT]; //array of FIR filter coefficients
const static double iir_a_coeff[FILTER_FREQUENCY_COUNT][IIR_A_COEFF_COUNT]; //array of IIR filter A coefficients
const static double iir_b_coeff[FILTER_FREQUENCY_COUNT][IIR_B_COEFF_COUNT]; //array of IIR filter B coefficients


/*************************************************************************
 * function that will just init the queue and will fill the Xqueue with 0's
 **************************************************************************/
void xQueue_init(){
    queue_init(&xQue,)
}


/*************************************************************************
 * function that will just init the queue and will fill the Yqueue with 0's
 **************************************************************************/
void yQueue_init(){

}

/*************************************************************************
 * function that will just init the queue and will fill the Zqueue with 0's
 **************************************************************************/
void zQueue_init(){
    for (uint32_t i=0; i<FILTER_IIR_FILTER_COUNT; i++) {
        queue_init(&(zQueue[i]), Z_QUEUE_SIZE);
        for (uint32_t j=0; j<Z_QUEUE_SIZE; j++)
            queue_overwritePush(&(zQueue[i]), QUEUE_INIT_VALUE);
  }
}

/*************************************************************************
 * function that will just init the queue and will fill the Outqueue with 0's
 **************************************************************************/
void outQueue_init(){
    
}

/*************************************************************************
 * function that will just init the filter and will fill all queues with 0's
 **************************************************************************/

void filter_init(){

}