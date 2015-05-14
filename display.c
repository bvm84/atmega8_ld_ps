/** \file display.c
 * \brief ������ ������ � ��������
 * ����� �� �������������� 3-��������� ���������
 * (������������ ������������ ���������)
 * \par
 * \author ARV
 * Copyright 2008 � ARV. All rights reserved. </b>
 * \par
 * ��� ������ ������� ���������:
 * 	-# WinAVR-20080411 ��� ����� ����� ������
 *
 */

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "display.h"


/// ����-������� ������� ��������� 1-�� ����������
uint8_t blink = 0;
/// ������-�����
extern volatile uint8_t SCR_D[SCR_SIZE];
/// ������ ��������-����
const uint8_t digits[] PROGMEM = {d_0,d_1,d_2,d_3,d_4,d_5,d_6,d_7,d_8,d_9};
static uint8_t digit(uint8_t d) 
{
	return pgm_read_byte(&digits[d]);	
}
	
PT_THREAD(SegDyn(struct pt *pt))
{
	static volatile uint32_t last_timer=0;
	static volatile uint8_t cathode=0;
	
	PT_BEGIN(pt);
	while (cathode<=SCR_SIZE-1)
	{
		PT_WAIT_UNTIL(pt, (st_millis()-last_timer)>=5);
		last_timer=st_millis();
		LCD_PORT_1&=~(_BV(7)); //CC1 off
		LCD_PORT_2&=~(_BV(6)); //CC2, CC3 off (���������� ���� ����)
		LCD_PORT_2&=~(_BV(7));
		//PT_WAIT_UNTIL(pt,1);
		//�������� �����
		LCD_PORT_1=((digit(SCR_D[cathode]))&(~(_BV(7)))); //�������� ����� ������ ����� ��� �������� ���������, �� ������
		//����� ����� ��������, �������, ��������� � ��������� ���������
		if (cathode==0) LCD_PORT_1|=(_BV(7));
		if (cathode==1) LCD_PORT_2|=(_BV(6));
		if (cathode==2) LCD_PORT_2|=(_BV(7));
		cathode++;
	}
	cathode=0;
	PT_END(pt);
}
	
