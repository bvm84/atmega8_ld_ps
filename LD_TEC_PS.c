/*
 * LD_TEC_PS.c
 *
 * Created: 25.11.2013 18:45:47
 *  Author: Вадим
 */ 

#include <avr/io.h>
#include "display.h"
#include <util/atomic.h>
#include <pt.h>
#include "timer.h"
#include "EncPoll.h"
#include "pid.h"
#include <util/delay.h>
#include <avr/wdt.h>
#include <avr/sleep.h>


//#define F_CPU 8000000 -defined in project symbols

#define UART_OUT 1
#ifndef UART_OUT
#define UART_OUT 0
#endif

//#define LCD_BUF_SIZE SCR_SIZE+1
#define P_FACTOR 2
#define I_FACTOR 0
#define D_FACTOR 0

#define B(bit_no)         (1 << (bit_no))
#define CB(reg, bit_no)   (reg) &= ~B(bit_no)
#define SB(reg, bit_no)   (reg) |= B(bit_no)
#define VB(reg, bit_no)   ( (reg) & B(bit_no) )
#define TB(reg, bit_no)   (reg) ^= B(bit_no)
//Бит за номером, очистка, установка, проверка, переключение


#define baudrate 9600L
#define bauddivider (F_CPU/(16*baudrate)-1)
#define HI(x) ((x)>>8)
#define LO(x) ((x)& 0xFF)
#define ABS(x) ((x) < 0 ? -(x) : (x))

struct divmod10_t
{
	uint32_t quot;
	uint8_t rem;
};
static volatile uint8_t ADC_values[]={0,0,0,0}; //8 бит АЦП пока
static volatile uint8_t ADC_counter=0;
static volatile uint16_t ADC0_value=0;
static volatile uint8_t PWM_value=0; 



static struct pt SegDyn_pt; //указатель на структуру протопотока который выводит цифры на индикатор
static struct pt EncoderScan_pt;
static struct pt CurrentCalc_pt;
static struct pt EncoderButton_pt;
static struct pt PidCurr_pt;
static struct pt Adc_pt;
static struct pt LcdSwitch_pt;



char * utoa_fast_div(uint32_t value, uint8_t *buffer);
inline static struct divmod10_t divmodu10(uint32_t n);
volatile uint8_t SCR_D[SCR_SIZE];
static struct PID_DATA *pid_reg_st;


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

char * utoa_fast_div(uint32_t value, uint8_t *buffer)
{
	
	uint8_t i=0;
	for(i=0;i<SCR_SIZE;i++)
	{
		*buffer=0;
		buffer++;
	} //кладем нули во все места, чтобы корректно отображались еденицы и десятки
	//buffer += LCD_BUF_SIZE;//указывает на младший разряд +1
	//*--buffer = 0;//вычитаем из адреса 1 и кладем в младший разряд 0
	do
	{
		struct divmod10_t res = divmodu10(value);
		*--buffer = res.rem;
		//*buffer = res.rem + '0';
		//buffer--;
		value = res.quot;
		//i++;
	}
	while (value != 0);
	//if (i==1) {*(buffer-3)=0; *(buffer-2)=0;}
	//else if (i==2) {*(buffer-2)=0;}
	
	return (char *)buffer; 
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
PT_THREAD(CurrentCalc(struct pt *pt))
{
	PT_BEGIN(pt);
	volatile uint8_t *ptr;
	ptr=&SCR_D[0]; //берем указатель на слепок экрана
	ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)EncoderValue, (uint8_t *)ptr);
	PT_END(pt);
}


PT_THREAD(PidCurr(struct pt *pt))
{
	static uint8_t pid_timer=0;
	static volatile int16_t PWM_calc=0;
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, (st_millis()-pid_timer)>=20);
	pid_timer=st_millis();
	
	PWM_calc=pid_Controller((int16_t)EncoderValue, (int16_t)ADC_values[0], pid_reg_st);
	
	PT_END(pt);
		
}
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
PT_THREAD(LcdSwitch(struct pt *pt))
{
	static volatile uint8_t lcd_switch_timer=0; 
	char * ptr=0;
	uint16_t pid_value=0;
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt,(st_millis()-lcd_switch_timer)>=100);
	ptr=&SCR_D[0];
	pid_value=ABS(pid_Controller((int16_t)EncoderValue, (int16_t)ADC0_value, pid_reg_st));
	if (ButtonState==BUTTON_ADC) ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)ADC0_value, (uint8_t *)ptr); //включить dot
	if (ButtonState==BUTTON_ENC) ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)EncoderValue, (uint8_t *)ptr);
	if (ButtonState==BUTTON_PID) ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)pid_value, (uint8_t *)ptr);
	pid_Reset_Integrator(pid_reg_st);
	PWM_value=EncoderValue;
	/*
	Для корректной работы данной реализации ПИДа в режиме П или ПД необходимо сбрасывать накопительную ошибку
	иначе ПИД со временем улетит в оверфлоу
	*/
	lcd_switch_timer=st_millis();
	PT_END(pt);
}



/*
усреднение АЦП -> суммируем 32 раза и сдвигаем на 5 (типа деление на 32).
Скользящее: убираем послденее, сдвигаем все и записываем новое.
Табуляция значений 8-бит АЦП в тупую требует 256 байт флэша, второй вариант - тупо пересчет
(математика, либо сдвиги, либо придумывать чнго)
*/

ISR(TIMER1_COMPA_vect)
{
	OCR1AL=EncoderValue;
}

ISR(ADC_vect)
{
	ADC_values[ADC_counter]=ADCH;
	ADMUX|=0b00001111; //все каналы АЦП сажаем на землю, перобразование окончено
	ADC_counter++;
	if (ADC_counter<4) 
	{
		ADMUX=(ADMUX&(~15))|ADC_counter; //подключаем нужный канал
	}
	else
	{
		ADC_counter=0; //сбрасываем счетчик
		ADMUX=(ADMUX&(~15));//подключаем нулевой канал АЦП
	}
	ADCSRA|=(_BV(6)); //так рабоатет в режиме фри  ранинг моде херня со смещением данных с каналов в массиве
}
/*
ISR(USART_UDRE_vect)
{
	//UDR=ADC_values[0];
	//UCSRB &=~(1<<UDRIE); //запрещаем прерывание от UART, передача щаа	
}
*/

int main(void)
{
	//volatile uint8_t *ptr;
	//uint8_t noise_level_value=0;
	//initiate ports
	DDRD=255; //all pins on portd are outputs
	DDRB=0b11000111;
	DDRC=0b11110000;//ADC pins inputs
	//DDRA='0b11010000';
	PORTD=0;
	PORTB=0b00111000;
	PORTC=0;
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
	//TIMSK |= _BV(TOIE0) | _BV(OCIE1A);
	// Set default value
	TCNT0 = ST_CTC_HANDMADE; //1ms tiks on 8mhz CPU clock
	
	//set timer 1 for PWM
	TCCR1A=0b10000001; //переключение oc1A по событие на таймере, oc1b льключен, WGM10=1, 8 бит таймер
	TCCR1B=0b00001001; //clocked from CLK=8MHZ WGM12=1
	OCR1AH=0;
	OCR1AL=127;//50% ШИМ 
	TCNT1=0;
	TIMSK=0;
	TIMSK |= _BV(TOIE0) | _BV(OCIE1A);
	//инициализация АЦП
	ADMUX=0b11100000; //опорное напряжение от внутреннего ИОН (2,56V), выравнивание по левому краю (читаем только ADCH), присоединить АЦП к входу ADC0;
	ADCSRA=0b00001111; //резрешить прерывание от АЦП, Установить делитель частоты 128
	MCUCR|=0b00010000;//установить ADC_noise canceling mode
	
	/* //Если включить этот блок сломается индикация, не включать в текущей аппаратной реализации
	UBRRL = LO(bauddivider); //грузим скокрость передачи 9600бод
	UBRRH = HI(bauddivider);
	UCSRA = 0; //обнуляем флаги приема передачи
	UCSRB = 1<<RXEN|1<<TXEN|0<<RXCIE|0<<TXCIE; //разрешаем работы USART, подключаем приемопередатчик к ногам, пока запрещаем перрывания
	UCSRC = 1<<URSEL|1<<UCSZ0|1<<UCSZ1;//передача 8 бит, стандартная (четность там, топ биты)
	*/
	
	//noise_level_value=ADC_init();
	pid_Init(P_FACTOR,I_FACTOR,D_FACTOR, pid_reg_st);
		
	PT_INIT(&SegDyn_pt);
	PT_INIT(&EncoderButton_pt);
	PT_INIT(&EncoderScan_pt);
	PT_INIT(&Adc_pt);
	PT_INIT(&LcdSwitch_pt);
	//PT_INIT(&CurrentCalc_pt);
	//PT_INIT(&PID_LD_CURR_pt);
	
	wdt_reset(); //сбрасываем собаку на всякий пожарный
	wdt_enable(WDTO_2S); //запускаем собаку с перидом 2с
	
	sei();
	//ADCSRA|=(_BV(7))|(_BV(6)); //запускаем АЦП в одиночном режиме, перезапуск в обработчике прерывания
	//ADCSRA|=(_BV(7));
	//UDR = ADC_values[0];		// Отправляем байт
	//UCSRB|=(1<<UDRIE);	// Разрешаем прерывание UDRE
    while(1)
    {
		SegDyn(&SegDyn_pt);
		EncoderButton(&EncoderButton_pt);
		EncoderScan(&EncoderScan_pt);
		Adc(&Adc_pt);
		LcdSwitch(&LcdSwitch_pt);
		wdt_reset(); //переодически сбрасываем собаку чтобы не улетететь в ресет
	 }
}
