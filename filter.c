#include "filter.h"
#include <stdint.h>

#define FIR_COEF_COUNT 81
#define IIR_COEF_COUNT 10
#define FILTER_IIR_FILTER_COUNT 10
#define QUEUE_INIT_VALUE 0.0

//Global Variables

const static queue_t xQue,yQue;
const static queue_t zQue[FILTER_IIR_FILTER_COUNT];
const static queue_t outQue[FILTER_IIR_FILTER_COUNT];
const static double fir_coeff[FIR_COEF_COUNT];
const static double iir_coeff[FILTER_FREQUENCY_COUNT][IIR_COEF_COUNT];


/*************************************************************************
 * function that will just init the queue and will fill the Xqueue will 0's
 **************************************************************************/
void xQueue_init(){
    queue_init(&xQue,)
}


/*************************************************************************
 * function that will just init the queue and will fill the Yqueue will 0's
 **************************************************************************/
void yQueue_init(){

}

/*************************************************************************
 * function that will just init the queue and will fill the Zqueue will 0's
 **************************************************************************/
void zQueue_init(){

}

/*************************************************************************
 * function that will just init the filter and will fill all queues will 0's
 **************************************************************************/

void filter_init(){

}