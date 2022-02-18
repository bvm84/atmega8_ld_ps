#include "board.h"
extern uint8_t SCR_D[SCR_SIZE];
static volatile uint8_t PWM_value=0; 
static struct PID_DATA *pid_reg_st; //структура ПИД
static struct pt_sem display_sem;
//extern struct pt_sem button_sem;
extern struct pt_sem button_sem;
extern uint8_t EncoderValue;
//Указатели на структуру протопотоков
static struct pt SegDyn_pt; //выводит цифры на индикатор
static struct pt EncoderScan_pt; //сканирует поворот ручки экнкодер
static struct pt CurrentCalc_pt; //вычисляет ток черех ЛД
static struct pt EncoderButton_pt; //сканирует кнопку энкодера
static struct pt PidCurr_pt; //PID, выдает значение ШИМ
static struct pt Adc_pt; //Получает данные от АЦП
static struct pt LcdSwitch_pt; //переключает отображаемый параметр на семисегнтнике

PT_THREAD(CurrentCalc(struct pt *pt)) {
	PT_BEGIN(pt);
	volatile uint8_t *ptr;
	ptr=&SCR_D[0]; //берем указатель на слепок экрана
	ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)EncoderValue, (uint8_t *)ptr);
	PT_END(pt);
}
PT_THREAD(PidCurr(struct pt *pt)) {
	static uint8_t pid_timer=0;
	static volatile int16_t PWM_calc=0;
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt, (st_millis()-pid_timer)>=20);
	pid_timer=st_millis();
	//PWM_calc=pid_Controller((int16_t)EncoderValue, (int16_t)ADC_values[0], pid_reg_st);
	PT_END(pt);		
}
PT_THREAD(LcdSwitch(struct pt *pt)) {
	static volatile uint8_t lcd_switch_timer=0; 
	char * ptr=0;
	uint16_t pid_value=0;
	PT_BEGIN(pt);
	PT_WAIT_UNTIL(pt,(st_millis()-lcd_switch_timer)>=100);
	ptr=&SCR_D[0];
	/*
	pid_value=pid_Controller((int16_t)EncoderValue, (int16_t)ADC0_value, pid_reg_st);
	if (ButtonState==BUTTON_ADC) ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)ADC0_value, (uint8_t *)ptr); //включить dot
	if (ButtonState==BUTTON_ENC) ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)EncoderValue, (uint8_t *)ptr);
	if (ButtonState==BUTTON_PID) ptr=(volatile uint8_t *)utoa_fast_div((uint32_t)pid_value, (uint8_t *)ptr);
	pid_Reset_Integrator(pid_reg_st);
	PWM_value=EncoderValue;
	*/
	/*
	Для корректной работы данной реализации ПИДа в режиме П или ПД необходимо сбрасывать накопительную ошибку
	иначе ПИД со временем улетит в оверфлоу
	*/
	lcd_switch_timer=st_millis();
	PT_END(pt);
}
int main(void)
{
	//volatile uint8_t *ptr;
	//uint8_t noise_level_value=0;
	//initiate ports
	DDRD=255; //all pins on portd are outputs, 7-seg indicator anodes, PD7 - CA1
	DDRB=0b11000111; 
	//PB0 - dp (7-seg ), PB1 - PWM, PB2 - Error, PB3 - MOSI(Enc_left), PB4 - MISO (Enc_rigth), PB5 - SCK(Enc_but), PB6 - CA2, PB7 - CA3
	DDRC=0b11110000;//ADC pins inputs
	//PA0 - CUR_MON, PA1 - V_MON, PA2 - NTC1, PA3 - NTC2, PA4 - DS_IN_A/LD_PWM_OUT_A, PA5 - DS_IN_B/LD_PWM_OUT_B
	//AREF - 5v, or internal 2.56V
	//DDRA='0b11010000';
	PORTD=0; // Ставим нули на семисегментнике
	PORTB=0b00111000;// Ставим нули на семисегментнике
	PORTC=0;
	
	SB(TCCR0, CS02); // инициализируем таймер0, как системный. Set prescaler to 256
	TCNT0 = ST_CTC_HANDMADE; //80ms tiks on 2Mhz CPU clock, 2000000/256=7812,5 тиков в секунду. 1/7812,5*10 = 0,00128с = 1.28мс
	SB(TIMSK, TOIE0); // Enable timer0 intterrupt
	
	TCCR1A=0b10000001; //set timer 1 for PWM переключение oc1A по событие на таймере, oc1b льключен, WGM10=1, 8 бит таймер
	TCCR1B=0b00001001; //clocked from CLK=8MHZ WGM12=1
	OCR1AH=0;
	OCR1AL=127;//50% ШИМ 
	TCNT1=0;
	TIMSK=0;
	TIMSK |= _BV(TOIE0) | _BV(OCIE1A);

	//noise_level_value=ADC_init();
	pid_Init(P_FACTOR,I_FACTOR,D_FACTOR, pid_reg_st);
		
	PT_INIT(&SegDyn_pt);
	PT_INIT(&EncoderButton_pt);
	PT_INIT(&EncoderScan_pt);
	PT_INIT(&Adc_pt);
	PT_INIT(&LcdSwitch_pt);
	PT_SEM_INIT(&display_sem, 0); //отображаем ток по умолчанию
	PT_SEM_INIT(&button_sem, 0); //отображаем ток по умолчанию
	EncoderValue = 1;
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
