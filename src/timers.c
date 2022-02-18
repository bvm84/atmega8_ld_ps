#include "board.h"
volatile static uint32_t st_timer0_millis;

ISR(TIMER0_OVF_vect) {
	st_timer0_millis++;
	TCNT0 = ST_CTC_HANDMADE;
}
ISR(TIMER1_COMPA_vect) {
	OCR1AL=EncoderValue;
}
uint32_t st_millis(void) {
	uint32_t m;
	ATOMIC_BLOCK(ATOMIC_RESTORESTATE)
	{
		m = st_timer0_millis;
	}
	return m;
}
