/*
 * LD_TEC_PS.c
 *
 * Created: 25.11.2013 18:45:47
 *  Author: Вадим
 */ 

#include <avr/io.h>
#include "display.h"
#include <util/atomic.h>
#include <pt-1.4/pt.h>
#include "timer.h"
#include "EncPoll.h"

#define LCD_BUF_SIZE SCR_SIZE+1

struct divmod10_t
{
	uint32_t quot;
	uint8_t rem;
};

static struct pt SegDyn_pt; //указатель на структуру протопотока который выводит цифры на индикатор
static struct pt EncoderScan_pt;
static struct pt CurrentCalc_pt;

char * utoa_fast_div(uint32_t value, char *buffer);
inline static struct divmod10_t divmodu10(uint32_t n);
volatile uint8_t SCR_D[SCR_SIZE];


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

char * utoa_fast_div(uint32_t value, char *buffer)
{
	buffer += LCD_BUF_SIZE;
	*--buffer = 0;
	do
	{
		struct divmod10_t res = divmodu10(value);
		*--buffer = res.rem + '0';
		value = res.quot;
	}
	while (value != 0);
	return buffer;
}


PT_THREAD(CurrentCalc(struct pt *pt))
{
/*	PT_BEGIN(pt);
	static char *ptr;
	ptr=(char *)&SCR_D[0];
	ptr=utoa_fast_div((uint32_t)EncoderValue, ptr);
	PT_END(pt);
*/
	SCR_D[0]=(uint8_t)EncoderValue;
}

/*
усреднение АЦП -> суммируем 32 раза и сдвигаем на 5 (типа деление на 32).
Скользящее: убираем послденее, сдвигаем все и записываем новое.
Табуляция значений 8-бит АЦП в тупую требует 256 байт флэша, второй вариант - тупо пересчет
(математика, либо сдвиги, либо придумывать чнго)
*/



int main(void)
{

	//initiate ports
	DDRD=255;
	DDRB=0b11000011;
	//DDRA='0b11010000';
	PORTD=PORTB=0;
/*
to do: DDRB PIN6,PIN7 - ouputs (CA2,CA3)
	   DDRB PIN0 - ouputs (dot)
	   DDRD PIN7 - otput (CA1)
	   
	   PORTB PIN1 - out PWM TEC, PIN2- input ERROR_IN (RED_BUTTON), PIN3,PIN4 - inputs encoder, PIN5 - input encoder button  
	   PortA - Pin0 - Cur_Mon, Pin1 - V_mon, Pin2 - NTC1, PIN3 - NTC2, PIN4 - LD_PWM_OUT, PIN5 - DS raiator sensor

*/
	// Set prescaler to 64
	TCCR0 |= (_BV(CS01) | _BV(CS00));
	// Enable interrupt
	TIMSK |= _BV(TOIE0);
	// Set default value
	TCNT0 = ST_CTC_HANDMADE; //1ms tiks on 8mhz CPU clock
		
	PT_INIT(&SegDyn_pt);
	PT_INIT(&EncoderScan_pt);
	PT_INIT(&CurrentCalc_pt);
	sei();

    while(1)
    {
		SCR_D[0]=7;
		SCR_D[1]=9;
		SCR_D[2]=2;
		
		//EncoderScan(&EncoderScan_pt);
		//CurrentCalc(&CurrentCalc_pt);
		SegDyn(&SegDyn_pt);
		EncoderScan(&EncoderScan_pt);
		CurrentCalc(&CurrentCalc_pt);
   }
	
}
