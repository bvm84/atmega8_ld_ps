#ifndef __ADC_H__
#define __ADC_H__
#include "board.h"
//ADC definitions
#define N_CHANNELS 4
#define ADC_MAX 1024

//extern uint16_t AdcValues[N_CHANNELS]={0,0,0,0}; //10 бит значения АЦП
PT_THREAD(Adc(struct pt *pt));

#endif