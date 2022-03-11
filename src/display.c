#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "display.h"
uint8_t SCR_D[SCR_SIZE];
uint8_t ValuueToShow = SHOW_LD_CURRENT;
extern uint16_t LdCurrent;
extern uint16_t LdVoltage;
extern uint16_t TecTemp;
extern uint16_t TecCurrent;
static struct pt SegDyn_pt; //выводит цифры на индикатор
/// Если нужно мерцание цифр (нужен таймер)
//uint8_t blink = 0;
inline static struct divmod10_t divmodu10(uint32_t n);
const uint8_t digits[] PROGMEM = {d_0,d_1,d_2,d_3,d_4,d_5,d_6,d_7,d_8,d_9};
static uint8_t digit(uint8_t d) {
	return pgm_read_byte(&digits[d]);	
}
PT_THREAD(DisplayOut(struct pt *pt)) {
	PT_BEGIN(pt);
	PT_INIT(&SegDyn_pt);
	uint8_t *ptr = &SCR_D[0];
	PT_SEM_WAIT(pt, )
	if (ValuueToShow == SHOW_LD_CURRENT) ptr = (volatile uint8_t *)utoa_fast_div((uint32_t)LdCurrent, (uint8_t *)ptr);
	else if (ValuueToShow == SHOW_LD_VOLTAGE) ptr = (volatile uint8_t *)utoa_fast_div((uint32_t)LdVoltage, (uint8_t *)ptr);
	else if (ValuueToShow == SHOW_TEC_TEMP) ptr = (volatile uint8_t *)utoa_fast_div((uint32_t)TecTemp, (uint8_t *)ptr);
	else if (ValuueToShow == SHOW_TEC_TEMP) ptr = (volatile uint8_t *)utoa_fast_div((uint32_t)TecCurrent, (uint8_t *)ptr);
	PT_SPAWN();
	PT_END(pt);
}
PT_THREAD(SegDyn(struct pt *pt)) {
	static volatile uint32_t last_timer=0;
	uint8_t cathode=0;
	PT_BEGIN(pt);
	while (cathode<SCR_SIZE) {
		PT_WAIT_UNTIL(pt, (st_millis()-last_timer)>=5); //5мсек на свечение одного разряда
		last_timer = st_millis();
		LCD_PORT_1 = 0;//segments off, CC0 off
		//LCD_PORT_2&=(~(_BV(6))); //CC2, CC3 off
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
inline static struct divmod10_t divmodu10(uint32_t n) {
	struct divmod10_t res;
	// умножаем на 0.8
	res.quot = n >> 1;
	res.quot += res.quot >> 1;
	res.quot += res.quot >> 4;
	res.quot += res.quot >> 8;
	res.quot += res.quot >> 16;
	uint32_t qq = res.quot;
	res.quot >>= 3; // делим на 8
	res.rem = (uint8_t)(n - ((res.quot << 1) + (qq & ~7ul))); // вычисляем остаток
	// корректируем остаток и частное
	if(res.rem > 9) {
		res.rem -= 10;
		res.quot++;
	}
	return res;
}
char * utoa_fast_div(uint32_t value, char * buffer) {	
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