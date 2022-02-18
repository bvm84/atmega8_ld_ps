#include "board.h"

static volatile uint8_t ADC_values[]={0,0,0,0}; //8 бит АЦП пока
static volatile uint8_t ADC_counter=0;
static volatile uint16_t ADC0_value=0;
ISR(ADC_vect) {
	ADC_values[ADC_counter]=ADCH;
	ADMUX|=0b00001111; //все каналы АЦП сажаем на землю, перобразование окончено
	ADC_counter++;
	if (ADC_counter<4) {
		ADMUX=(ADMUX&(~15))|ADC_counter; //подключаем нужный канал
	}
	else {
		ADC_counter=0; //сбрасываем счетчик
		ADMUX=(ADMUX&(~15));//подключаем нулевой канал АЦП
	}
	ADCSRA|=(_BV(6)); //так рабоатет в режиме фри  ранинг моде херня со смещением данных с каналов в массиве
}
/*
усреднение АЦП -> суммируем 32 раза и сдвигаем на 5 (деление на 32).
Скользящее: убираем последнее, сдвигаем все и записываем новое.
Табуляция значений 8-бит АЦП в тупую требует 256 байт флэша, второй вариант - тупо пересчет
(математика, либо сдвиги, либо придумывать чнго)
*/
PT_THREAD(Adc(struct pt *pt))
{
	static volatile uint8_t adc_lcd_timer=0, averaging=0;
	static volatile uint32_t aver_value=0; 
	//volatile uint8_t *ptr;
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt,(st_millis()-adc_lcd_timer)>=10);
	/*sleep_enable();
	ADCSRA|=(_BV(7))|(_BV(6));//заупск преобразования АЦП
	sleep_cpu();
	sleep_disable();
	//этот кусок вроде как повышает стабильность АЦП, но с ним начинаются пропуски импульсов ШИМ
	*/
	ADCSRA|=(_BV(7))|(_BV(6));//заупск преобразования АЦП
	if (averaging<32) {
		averaging++;
		aver_value+=ADC_values[0];
		}
	else {
		averaging=0;
		ADC0_value=aver_value>>5;//&&((st_millis()-adc_lcd_timer)>=1)
		//ptr=&SCR_D[0];
		//ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)ADC0_value, (uint8_t *)ptr);
		aver_value=0;
		}
	adc_lcd_timer=st_millis();
	PT_END(pt);
}
/* функция инициализации АЦП и нахождения уровня шума, передумал делать, может в будущем пригодится

uint8_t ADC_init(void)
{
	uint16_t ADC_SUMM=0, prev_timer=st_millis();
	ADMUX=0b01101111; //опорное напряжение с AVCC (5V), выравнивание по левому краю (читаем только ADCH), 
	//0  -  не используется в atmega8,1111 - посадить вход АЦП на землю, чтобы найти шумы
	ADCSR=0b10000011;//enable ФВс+prescaler 128=>ADC freq =8MHz/128=62.5kHz
	for (uint8_t i=0;i<32;i++)
	{
		ADCSR|=(_BV(6)); //запустить однократное преобразование АЦП
		while(((ADCSR&(_BV(4)))>>4)!=1)
		{
			;
		}
		ADCSR|=(_BV(4));//сбрасываем флаг окончания преобразования АЦП запись 1 в бит 4
		ADC_SUMM+=ADCH;
		
	}
	ADMUX=0b01100001; //присоединить АЦП к входу ADC0;
	ADCSR=0b10001011; //частота преобразоания 62,5кГц, запустить АЦП, включить прерывания от АЦП
	return ADC_SUMM>>5; //усреднение результата шума, деление на 32 пятью сдвигами враво
}
*/