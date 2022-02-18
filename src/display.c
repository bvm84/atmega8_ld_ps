#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "display.h"

/// Если нужно мерцание цифр (нужен таймер)
//uint8_t blink = 0;
inline static struct divmod10_t divmodu10(uint32_t n);
const uint8_t digits[] PROGMEM = {d_0,d_1,d_2,d_3,d_4,d_5,d_6,d_7,d_8,d_9};
static uint8_t digit(uint8_t d) {
	return pgm_read_byte(&digits[d]);	
}
PT_THREAD(SegDyn(struct pt *pt)) {
	static volatile uint32_t last_timer=0;
	uint8_t cathode=0;
	PT_BEGIN(pt);
	while (cathode<SCR_SIZE) {
		PT_WAIT_UNTIL(pt, (st_millis()-last_timer)>=5);
		last_timer = st_millis();
		LCD_PORT_1 = 0;//segments off, CC0 off
		//LCD_PORT_2&=(~(_BV(6))); //CC2, CC3 off (���������� ���� ����)
		//LCD_PORT_2&=(~(_BV(7)));
		LCD_PORT_2 &= CC2_CC3_MASK;
		PT_WAIT_UNTIL(pt, (st_millis()-last_timer) >= 1); //задержка после погашения индикаторов
		LCD_PORT_1 |= (digit((uint8_t)SCR_D[cathode]));
		if (cathode==0) LCD_PORT_1|=(_BV(7));
		if (cathode==1) LCD_PORT_2|=(_BV(6));
		if (cathode==2) LCD_PORT_2|=(_BV(7));
		//LCD_PORT_1|=(digit((uint8_t)SCR_D[cathode]));
		cathode++;
	}
	PT_END(pt);
}
inline static struct divmod10_t divmodu10(uint32_t n)
{
	struct divmod10_t res;
	// умножаем на 0.8
	res.quot = n >> 1;
	res.quot += res.quot >> 1;
	res.quot += res.quot >> 4;
	res.quot += res.quot >> 8;
	res.quot += res.quot >> 16;
	uint32_t qq = res.quot;
	// делим на 8
	res.quot >>= 3;
	// вычисляем остаток
	res.rem = (uint8_t)(n - ((res.quot << 1) + (qq & ~7ul)));
	// корректируем остаток и частное
	if(res.rem > 9)
	{
		res.rem -= 10;
		res.quot++;
	}
	return res;
}

char * utoa_fast_div(uint32_t value, uint8_t *buffer) {	
	uint8_t i=0;
	buffer += SCR_SIZE;
    // 3 байт достаточно, т.к. у нас индиатор с тремя цифрами
    //*--buffer = 0; //Завершающий нуль не нужен
	while ((value != 0) || i >= SCR_SIZE) {
		struct divmod10_t res = divmodu10(value);
		*--buffer = res.rem;
		//*buffer = res.rem + '0'; // Индикатор конца строки не нужен
		//buffer--;
		value = res.quot;
		i++;
	}
	//if (i==1) {*(buffer-3)=0; *(buffer-2)=0;}
	//else if (i==2) {*(buffer-2)=0;}
	return buffer; 
}