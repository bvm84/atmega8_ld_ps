/*
 * CFile1.c
 *
 * Created: 03.12.2013 12:03:47
 *  Author: Вадим
 */ 
#include "EncPoll.h"
#include "timer.h"

//static uint16_t EncoderValue=0;

const	int8_t	EncState[] PROGMEM =
{
	0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0
};
PT_THREAD(EncoderScan(struct pt *pt))
{
	static	uint16_t	EncVal=0;
	static	uint8_t	Enc=0;
	static uint8_t enc_timer=0;
	
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, (st_millis()-enc_timer)>=1);
	enc_timer=st_millis();
	Enc += PIN(ENCPOLL_PORT) & ((1<<ENCPOLL_A_PIN)|(1<<ENCPOLL_B_PIN));
	EncVal += (uint16_t)pgm_read_byte(&(EncState[Enc]));
	Enc <<= 2;
	Enc &= 0b00001111;
	
	//if (EncVal > 0xfe) EncVal = 0;
	if (EncVal > 600) EncVal = 600; //600 = 6A current
	
	EncoderValue=EncVal;
	
	PT_END(pt);
}
/*int8_t	EncPollDelta( void )
{
	static	uint8_t	Enc=0;
	Enc <<= 2;
	Enc &= 0b00001111;
	Enc += PIN(ENCPOLL_PORT) & ((1<<ENCPOLL_A_PIN)|(1<<ENCPOLL_B_PIN));
	return pgm_read_byte(&(EncState[Enc]));
}
*/
