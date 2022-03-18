#ifndef __ADC_H__
#define __ADC_H__
#include "board.h"
//ADC definitions
#define N_CHANNELS 4
#define ADC_MAX 1024

typedef struct adc_cb {
	uint16_t adcLdI;
    uint16_t adcLdV;
	uint16_t adcTecTemp;
    uint16_t adcTecI;
} adc_cb;

adc_cb get_adc_values(void);

#endif