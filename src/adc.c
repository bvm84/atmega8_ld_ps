#include "board.h"
static adc_cb adcCb = {
	.adcLdI =0,
	.adcLdV = 0,
	.adcTecTemp = 0,
	.adcTecI = 0
};
static adcMutex = 0;
static uint16_t AdcValues[N_CHANNELS]={0,0,0,0}; //10 бит значения АЦП
static volatile uint8_t AdcCounter=0;
ISR(ADC_vect) {
	AdcValues[AdcCounter] = ADCW;
	ADMUX |= 0b00001111; //все каналы АЦП сажаем на землю, перобразование окончено
	AdcCounter++;
	if (AdcCounter < 4) {
		ADMUX = (ADMUX &(~15)) | AdcCounter; //подключаем нужный канал
	}
	else {
		AdcCounter = 0; //сбрасываем счетчик
		ADMUX = (ADMUX &(~15));//подключаем нулевой канал АЦП
		adcMutex = 0;
	}
	ADCSRA |= (_BV(6)); //так рабоатет в режиме фри  ранинг моде херня со смещением данных с каналов в массиве
}
/*
усреднение АЦП -> суммируем 32 раза и сдвигаем на 5 (деление на 32).
Скользящее: убираем последнее, сдвигаем все и записываем новое.
Табуляция значений 8-бит АЦП в тупую требует 256 байт флэша, второй вариант - тупо пересчет
(математика, либо сдвиги, либо придумывать чнго)
*/
adc_cb get_adc_values(void) {
	uint8_t AveragingCounter=0;
	uint32_t AdcSums[N_CHANNELS] = {0, 0, 0, 0}; 
	//volatile uint8_t *ptr;
    //инициализация АЦП
	ADMUX = 0b11100000; //опорное напряжение от внутреннего ИОН (2,56V), присоединить АЦП к входу ADC0;
	ADCSRA = 0b00001111; //резрешить прерывание от АЦП, Установить делитель частоты 128
	MCUCR |= 0b00010000;//установить ADC_noise canceling mode
	/*sleep_enable();
	ADCSRA|=(_BV(7))|(_BV(6));//заупск преобразования АЦП
	sleep_cpu();
	sleep_disable();
	//этот кусок вроде как повышает стабильность АЦП, но с ним начинаются пропуски импульсов ШИМ
	*/
	while(AveragingCounter < 32) {
		adcMutex = 1; 
		ADCSRA |= (_BV(7))|(_BV(6));//запуск преобразования АЦП
		while(!adcMutex) ; //ждем окончания преобразования
        for(uint8_t i=0; i<N_CHANNELS; i++) AdcSums[i] += AdcValues[i];
		AveragingCounter++;
    }
	adcCb.adcLdI = AdcSums[0]>>5;
	adcCb.adcLdV = AdcSums[1]>>5;
	adcCb.adcTecTemp = AdcSums[2]>>5;
	adcCb.adcTecI = AdcSums[3]>>5;
	for(uint8_t i=0; i<N_CHANNELS; i++) AdcSums[i] = 0;
	return adcCb;
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