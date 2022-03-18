#include "board.h"
extern uint8_t SCR_D[SCR_SIZE];
extern uint16_t LdCurrent;
extern uint16_t LdVoltage;
extern uint16_t TecTemp;
extern uint16_t TecCurrent;
static volatile uint8_t PWM_value=0; 
static struct PID_DATA *pid_reg_st; //структура ПИД
static device_cb deviceCb = {
	.deviceState = CONTROL_LD_CURRENT,
	.setLdI = 0,
	.currentLdI = 0,
	.currentLdV = 0,
	.maxLdI = 1024,
	.maxLdV = 0
};
void display_out(uint16_t value) {
	uint8_t *ptr = &SCR_D[0];
	ptr = (volatile uint8_t *)utoa_fast_div((uint32_t)value, (uint8_t *)ptr);
	seg_dyn(ptr);
}
void curent_voltage_calc(void) {
	uint8_t *ptr;
	LdCurrent = AdcValues[0] >> 3;
	LdVoltage = AdcValues[1] >> 5;
	TecTemp = AdcValues[2] >> 5;
	TecCurrent = AdcValues[3] >> 5;
}
void lcd_switch(void) {
	static volatile uint8_t lcd_switch_timer=0; 
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
}
int main(void)
{
	uint8_t encValue = 1, encButton = BUTTON_NO_PRESS;
	adc_cb adcCurrentValues;
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
	pid_Init(P_FACTOR, I_FACTOR, D_FACTOR, pid_reg_st);
	wdt_reset(); //сбрасываем собаку на всякий пожарный
	wdt_enable(WDTO_2S); //запускаем собаку с перидом 2с
	sei();
	//ADCSRA|=(_BV(7))|(_BV(6)); //запускаем АЦП в одиночном режиме, перезапуск в обработчике прерывания
	//ADCSRA|=(_BV(7));
	//UDR = ADC_values[0];		// Отправляем байт
	//UCSRB|=(1<<UDRIE);	// Разрешаем прерывание UDRE
    while(1)
    {
		encButton = encoder_button();
		encValue = encoder_scan();
		adcCurrentValues = get_adc_values();
		deviceCb.currentLdI = adcCurrentValues.adcLdI;
		deviceCb.currentLdV = adcCurrentValues.adcLdI;
		if (deviceCb.currentLdI >= deviceCb.maxLdI) {
			//deviceCb.setLdI = 0;
			PWM_output = 0;
			deviceCb.deviceState = LD_OVERCURRENT_ERROR;
		}
		if (deviceCb.currentLdV >= deviceCb.maxLdI) {
			//deviceCb.setLdI = 0;
			PWM_output = 0;
			deviceCb.deviceState = LD_OVERVOLTAGE_ERROR;
		}
		deviceCb.setLdI = encValue; //convert encValue to something
		switch (deviceCb.deviceState) {
			case CONTROL_LD_CURRENT: {
				deviceCb.setLdI = encValue; //convert encValue to something
				deviceCb.currentLdI = adcCurrentValues.adcLdI;
				pwm_value = pid_Controller((int16_t)deviceCb.setLdI, (int16_t)deviceCb.currentLdI, pid_reg_st);
				pwm_output(pwm_value);
				display();
				break;
			}
			case SET_MAX_CURENT: {
				while (!encoder_button()) {
					deviceCb.maxLdI = encoder_scan();
					display();
				}
				deviceCb.deviceState = CONTROL_LD_CURRENT;
				break;
			}
			case SET_MAX_VOLTAGE: {
				while (!encoder_button()) {
					deviceCb.maxLdV = encoder_scan();
					display();
				}
				deviceCb.deviceState = CONTROL_LD_CURRENT;
				break;
			}
			case LD_OVERCURRENT_ERROR: {
				//PWM_output = 0;
				//overcurent_error_display
				deviceCb.deviceState = SET_MAX_CURENT; 
				while (!encoder_button()) ;
				break;
			}
			case LD_OVERVOLTAGE_ERROR: {
				//PWM_output = 0;
				//overvolatge_error_display
				deviceCb.deviceState = SET_MAX_VOLTAGE; 
				while (!encoder_button()) ;
				break;
			}
		}
		wdt_reset(); //переодически сбрасываем собаку чтобы не улетететь в ресет
	}
}
