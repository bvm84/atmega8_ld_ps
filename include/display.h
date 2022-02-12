/** \file display.h
 * \brief Заголовочный файл модуля дисплея
 * прототипы функций и константы с макросами\n
 * \par
 * \author ARV
 * \note
 * \n Схема:
 * \n \date	12.11.2008 ... 	__.__.2008
 * \par
 * \version v1.00	\n
 * Copyright 2008 © ARV. All rights reserved. </b>
 * \par
 * Для сборки проекта требуется:
 * 	-# WinAVR-20080411 или более новая версия
 *
 */


#ifndef DISPLAY_H_
#define DISPLAY_H_

/// тип индикаторов
#define common_anode 0

#include "leds7.h"
#include "pt-1.4/pt.h"
#include "timer.h"

/// "размер" дисплея
#define SCR_SIZE 3
/// массив "экранной области" слепок экрана
#define LCD_PORT_1 PORTD
#define LCD_PORT_2 PORTB

PT_THREAD(SegDyn(struct pt *pt));
extern volatile uint8_t SCR_D[SCR_SIZE];

//extern uint32_t st_millis(void);

/// флаг для мигания
//extern uint8_t blink;
/// Вывод в десятичном формате со знаком
//void print_dec(int data);
/// Вывод в 16-ичном формате
//static uint8_t digit(uint8_t d);


#endif /* DISPLAY_H_ */
