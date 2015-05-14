/*
* EncPoll.h
*
* Created: 03.12.2013 13:47:53
* Author: Вадим
*/


#ifndef ENCPOLL_H_
#define ENCPOLL_H_

#include <inttypes.h>
#include <avr/pgmspace.h>
#include "timer.h"
#include <pt-1.4/pt.h>

#define ENCPOLL_PORT B
#define ENCPOLL_A_PIN 3
#define ENCPOLL_B_PIN 4
#define ENCBUT_PORT B
#define ENCBUT_PIN 5

#define PIN_(port) PIN ## port
#define PIN(port) PIN_(port)

#define BUTTON_ON 1
#define BUTTON_OFF 0
#define BOUNCE 3
#define BUTTON_LONG_ON 2
#define BUT_MASK 16

extern uint16_t EncoderValue;
extern uint8_t ButtonState;


PT_THREAD(EncoderScan(struct pt *pt));
PT_THREAD(EncoderButton(struct pt *pt));



#endif /* ENCPOLL_H_ */