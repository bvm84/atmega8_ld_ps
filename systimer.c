/*
 * CFile1.c
 *
 * Created: 26.11.2013 12:24:12
 *  Author: Вадим
 */ 
#include <interrupt.h>

#define ST_CTC_HANDMADE 255-125

volatile static uint32_t st_timer0_millis;

ISR(TIMER0_OVF_vect)
{
	st_timer0_millis++;
	TCNT0 = ST_CTC_HANDMADE;
}

uint32_t st_millis(void)
{
	uint32_t m;

	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		m = st_timer0_millis;
	}

	return m;
}

void inline St_Init(void)
{
	// Set prescaler to 64
	TCCR0 |= (_BV(CS01) | _BV(CS00));

	// Enable interrupt
	TIMSK |= _BV(TOIE0);

	// Set default value
	TCNT0 = ST_CTC_HANDMADE;
}