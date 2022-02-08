
#include "filter.h"
#include <stdint.h>

#define FIR_COEF_COUNT 81
#define IIR_A_COEFF_COUNT 11
#define IIR_B_COEFF_COUNT 10
#define Z_QUEUE_SIZE 10 // 10 for the 10 different players
#define Y_QUEUE_SIZE 11 
#define X_QUEUE_SIZE 81
#define FILTER_IIR_FILTER_COUNT 10
#define OUTPUT_QUEUE_SIZE 2000
#define QUEUE_INIT_VALUE 0.0
#define DECIMATION_VALUE 10

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

    queue_init(&xQue,X_QUEUE_SIZE);
    for (uint32_t i = 0; i < X_QUEUE_SIZE; i++) //push 0's to all positions in queue
        queue_overwritePush(&(xQue),QUEUE_INIT_VALUE);
    
}


/*************************************************************************
 * function that will just init the queue and will fill the Yqueue with 0's
 **************************************************************************/
void yQueue_init(){

    queue_init(&yQue,Y_QUEUE_SIZE);
    for (uint32_t i = 0; i < Y_QUEUE_SIZE; i++) //push 0's to all positions in queue
        queue_overwritePush(&(yQue),QUEUE_INIT_VALUE);
}

/*************************************************************************
 * function that will just init the queue and will fill the Zqueue with 0's
 **************************************************************************/
void zQueue_init(){
    for (uint32_t i=0; i<FILTER_IIR_FILTER_COUNT; i++) { //push 0's to all positions in queue
        queue_init(&(zQue[i]), Z_QUEUE_SIZE);          // This case there will be 10 filters for the 10 players
        for (uint32_t j=0; j<Z_QUEUE_SIZE; j++)
            queue_overwritePush(&(zQue[i]), QUEUE_INIT_VALUE);
  }
}

/*************************************************************************
 * function that will just init the queue and will fill the Outqueue with 0's
 **************************************************************************/
void outQueue_init(){

    for (uint32_t i=0; i<FILTER_IIR_FILTER_COUNT; i++) { //push 0's to all positions in queue
        queue_init(&(outQue[i]), OUTPUT_QUEUE_SIZE);          // This case there will be 10 filters for the 10 players
        for (uint32_t j=0; j<OUTPUT_QUEUE_SIZE; j++)
            queue_overwritePush(&(outQue[i]), QUEUE_INIT_VALUE);
  }
}

/*************************************************************************
 * function that will just init the filter and will fill all queues with 0's
 **************************************************************************/

void filter_init(){
    xQueue_init();
    yQueue_init();  // seperate init functions for each queue
    zQueue_init();
    outQueue_init();
}

/************************************************************************************
 * function that will push the specified value to the xque and comes from the signal
 ************************************************************************************/

void filter_addNewInput(double x){
    queue_overwritePush(&(xQue),x); // push value onto the queue stack
}


/*********************************************************
 * function that will fill queue with the specified value 
 *********************************************************/
void filter_fillQueue(queue_t *q, double fillValue){
    for(uint32_t i = 0; i < queue_size(q);i++) // gets and starts a for loop through the queue
        queue_overwritePush(q,fillValue); // push the fill value
}


/************************************************************************************
 * This function starts the FIR-Filter process and pushes the data to the yQueue
 ************************************************************************************/
double filter_firFilter(){

}


/************************************************************************************
 * Function to start amd process a single iirFilter, input is the yqueue and output
 * is the zQueue
 ************************************************************************************/
double filter_iirFilter(uint16_t filterNumber){

}







/***********************************
 * Verification-assisting Functions
 ***********************************/

const double *filter_getFirCoefficientArray(){
    return fir_coeff; // returns the pointer to the coeff array
}

