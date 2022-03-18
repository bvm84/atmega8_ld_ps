#ifndef __BOARD_H__
#define __BOARD_H__
#include <avr/io.h>
#include <util/atomic.h>
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sleep.h>
#include <avr/pgmspace.h>
#include <stdint.h>
#include "leds7.h"
#include "display.h"
#include "encpoll.h"
#include "pid.h"
#include "adc.h"
//Timers definitions
#define ST_CTC_HANDMADE ((uint8_t) 255-10) // TImer0 settings
//PID parameters
#define P_FACTOR 2
#define I_FACTOR 0
#define D_FACTOR 0
//Macros definitions
#define B(bit_no)         (1 << (bit_no))
#define CB(reg, bit_no)   (reg) &= ~B(bit_no)
#define SB(reg, bit_no)   (reg) |= B(bit_no)
#define VB(reg, bit_no)   ( (reg) & B(bit_no) )
#define TB(reg, bit_no)   (reg) ^= B(bit_no)

//LD definitions
#define LD_CURRENT_MAX 128 //1.28A max curent
#define LD_CURRENT_MIN 0
#define LD_VOLTAGE_MAX 32 //3.2V max voltage
#define LD_VOLTAGE_MIN 0

#define CONTROL_LD_CURRENT      10
#define SET_MAX_CURENT          11
#define SET_MAX_VOLTAGE         12
#define LD_OVERCURRENT_ERROR    13
#define LD_OVERVOLTAGE_ERROR    14

typedef struct device_cb {
    uint8_t deviceState;
    uint16_t setLdI;
    uint16_t currentLdI;
	uint16_t currentLdV;
    uint16_t maxLdI;
    uint16_t maxLdV;
} device_cb;

uint32_t st_millis(void);
ISR(TIMER0_OVF_vect); 

#endif