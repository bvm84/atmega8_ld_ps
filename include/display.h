#ifndef __DISPLAY_H__
#define __DISPLAY_H__

#include "board.h"
//Маска для обнуления катодов на порте B
#define CC2_CC3_MASK 0xC0
//Определяем тип семисегментного индикатора
#define common_anode 0
//Маска для транзисторов катодов на PORTB
#define CC2_CC3_MASK 0xC0
//Количество цифр в индикаторе
#define SCR_SIZE 3
//Определяем порты к которым пподключен семисегментный индиктор
#define LCD_PORT_1 PORTD
#define LCD_PORT_2 PORTB
struct divmod10_t {
	uint32_t quot;
	uint8_t rem;
};
void seg_dyn(void);
char * utoa_fast_div(uint32_t value, char *buffer);

//Если нужно мигания
//extern uint8_t blink;
//Вывод десятичного?
//void print_dec(int data);
//Массив цифр
//static uint8_t digit(uint8_t d);
#endif
