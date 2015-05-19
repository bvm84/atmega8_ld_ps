/** \file display.h
 * \brief ������������ ���� ������ �������
 * ��������� ������� � ��������� � ���������\n
 * \par
 * \author ARV
 * \note
 * \n �����:
 * \n \date	12.11.2008 ... 	__.__.2008
 * \par
 * \version v1.00	\n
 * Copyright 2008 � ARV. All rights reserved. </b>
 * \par
 * ��� ������ ������� ���������:
 * 	-# WinAVR-20080411 ��� ����� ����� ������
 *
 */


#ifndef DISPLAY_H_
#define DISPLAY_H_

/// ��� �����������
#define common_anode 0

#include "leds7.h"
#include "pt-1.4/pt.h"
#include "timer.h"

/// "������" �������
#define SCR_SIZE 3
/// ������ "�������� �������" ������ ������
#define LCD_PORT_1 PORTD
#define LCD_PORT_2 PORTB

PT_THREAD(SegDyn(struct pt *pt));
extern volatile uint8_t SCR_D[SCR_SIZE];

//extern uint32_t st_millis(void);

/// ���� ��� �������
//extern uint8_t blink;
/// ����� � ���������� ������� �� ������
//void print_dec(int data);
/// ����� � 16-����� �������
//static uint8_t digit(uint8_t d);


#endif /* DISPLAY_H_ */
