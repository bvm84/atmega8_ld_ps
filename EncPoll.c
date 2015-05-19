/*
 * CFile1.c
 *
 * Created: 03.12.2013 12:03:47
 *  Author: Вадим
 */ 
#include "EncPoll.h"

uint16_t EncoderValue=0;
uint8_t ButtonState=BUTTON_OFF;

void inline button_change_state(void);

const	int8_t	EncState[] PROGMEM =
{
	0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0
};

void inline button_change_state(void)
{
	if ((ButtonState==BUTTON_ON)||(ButtonState==BUTTON_LONG_ON)) ButtonState=BUTTON_OFF;
	else ButtonState=BUTTON_ON;
}

PT_THREAD(EncoderScan(struct pt *pt))
{
	static	uint8_t	EncVal=0;
	static	uint8_t	Enc=0;
	static uint8_t enc_timer=0;
	
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, st_millis()-enc_timer>=2);
	enc_timer=st_millis();
	
	Enc += ((PIN(ENCPOLL_PORT) & ((1<<ENCPOLL_A_PIN)|(1<<ENCPOLL_B_PIN)))>>3);
	PT_WAIT_UNTIL(pt, st_millis()-enc_timer>=1);
	EncVal += pgm_read_byte(&(EncState[Enc]));
	Enc <<= 2;
	Enc &= 0b00001111;
	
	//if (EncVal > 0xfe) EncVal = 0;
	if (EncVal < 1) EncVal = 1;
	if (EncVal > 600) EncVal = 600; //600 = 6A current
	
	EncoderValue=EncVal;
	
	PT_END(pt);
}
PT_THREAD(EncoderButton(struct pt *pt))
{
	static uint32_t but_timer=0;
	static uint16_t val=0;
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, (st_millis()-but_timer)>=1);
	but_timer=st_millis();
	if (((PIN(ENCBUT_PORT)&(_BV(ENCBUT_PIN)))==0)&&(val<=1000))
	{
		val++;
	}
	else
	{
		if (val>900)
		{
			ButtonState=BUTTON_LONG_ON;
			PT_WAIT_UNTIL(pt,(st_millis()-but_timer)>=1000);
		}
		else if (val>=5)
		{
			button_change_state();
		}
		val=0;
	}
	PT_END(pt);
}

/* проверить сосотояние кнопки
если нажата, увеличиваем счетчик и ждем 100мсек
если через 100мсек все еще нажата, увеличиваем счетчик еще
и так до 5 раз

Запускаем цикл с уловием есть 

 меняем состояние переменой кнопки
 
 */
/*int8_t	EncPollDelta( void )
{
	static	uint8_t	Enc=0;
	Enc <<= 2;
	Enc &= 0b00001111;
	Enc += PIN(ENCPOLL_PORT) & ((1<<ENCPOLL_A_PIN)|(1<<ENCPOLL_B_PIN));
	return pgm_read_byte(&(EncState[Enc]));
}
*/
