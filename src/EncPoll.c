/*
 * CFile1.c
 *
 * Created: 03.12.2013 12:03:47
 *  15.12.2015
 Внес изменения в EncoderScan:
 static	uint8_t	EncVal=1; вместо EncVal=0;
 EncVal -= pgm_read_byte, а не EncVal += pgm_read_byte
 Иначе при вращении вправо значения уменьшались, а надо чтобы увеличивались.
 Также  EncoderButton теперь переключает режим отображения

 */ 
#include "board.h"
static uint8_t EncoderValue = 1;
//extern struct pt_sem button_sem;
//extern uint8_t EncoderValue;
// uint8_t ButtonState=BUTTON_ADC;

//void inline button_change_state(void);

const int8_t EncState[] PROGMEM =
{
	0, -1, 1, 0, 1, 0, 0, -1, -1, 0, 0, 1, 0, 1, -1, 0
};

/*void inline button_change_state(void)
{
	if ((ButtonState==BUTTON_ON)||(ButtonState==BUTTON_LONG_ON)) ButtonState=BUTTON_OFF;
	else ButtonState=BUTTON_ON;
}
*/
uint8_t encoder_scan(void) {
	uint8_t Enc=0;
	Enc += ((PIN(ENCPOLL_PORT) & ((1<<ENCPOLL_A_PIN)|(1<<ENCPOLL_B_PIN)))>>3);
	EncoderValue -= pgm_read_byte(&(EncState[Enc]));
	Enc <<= 2;
	Enc &= 0b00001111;
	if (EncoderValue < 1) EncoderValue = 1;
	if (EncoderValue > 254) EncoderValue = 254; //600 = 6A current
	return EncoderValue;
}
uint8_t encoder_button(void) {
	static uint16_t val=0;
	while (((PIN(ENCBUT_PORT)&(_BV(ENCBUT_PIN)))==0)&&(val<=1000)) {
		val++;
	}
	if (val>900) {
		return BUTTON_LONG_PRESS;
	}
	else if (val>=5) {
		return BUTTON_SHORT_PRESS;
	}
	else return BUTTON_NO_PRESS;
}

/* проверить состояние кнопки
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
