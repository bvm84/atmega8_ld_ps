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
uint8_t EncoderValue;
struct pt_sem button_sem;
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

PT_THREAD(EncoderScan(struct pt *pt))
{
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, (PIN(ENCPOLL_PORT) & (1<<ENCPOLL_A_PIN)) ||(PIN(ENCPOLL_PORT) & (1<<ENCPOLL_B_PIN)));
	//Поток не запустится пока не сработает энкодер
	uint8_t Enc=0;
	Enc += ((PIN(ENCPOLL_PORT) & ((1<<ENCPOLL_A_PIN)|(1<<ENCPOLL_B_PIN)))>>3);
	EncoderValue -= pgm_read_byte(&(EncState[Enc]));
	Enc <<= 2;
	Enc &= 0b00001111;
	if (EncoderValue < 1) EncoderValue = 1;
	if (EncoderValue > 254) EncoderValue = 254; //600 = 6A current
	PT_END(pt);
}
PT_THREAD(EncoderButton(struct pt *pt)) {
	static uint16_t val=0;
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, ((PIN(ENCBUT_PORT)&(_BV(ENCBUT_PIN))))); //поток не запустится пока не будет зфиксировано нажатие
	while (((PIN(ENCBUT_PORT)&(_BV(ENCBUT_PIN)))==0)&&(val<=1000)) {
		val++;
	}
	if (val>900) {
		//ButtonState=BUTTON_LONG_ON; - это сейчас не нужно, вдруг пригодится обрабатывать долгие нажатия
		PT_SEM_SIGNAL(pt, &button_sem);
		PT_SEM_SIGNAL(pt, &button_sem);
	}
	else if (val>=5) {
		PT_SEM_SIGNAL(pt, &button_sem);
	}
	val=0;
	PT_END(pt);
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
