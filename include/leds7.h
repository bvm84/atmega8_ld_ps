#ifndef __LEDS7_H__
#define __LEDS7_H__
/*
 * При описании сегментов используется "прямая" логика: единичка означает свечение сегмента.
 * Для учета рода индикатора (с общим анодом или катодом) используются
 * макрос common_anode>, который должен быть описан в проекте, если индикатор с общим анодом.
 * для индикаторов с общим катодом этого макроса быть не должно.
 */

//Пин сегмента a
#define _a _BV(4) //PORTD
#define _b _BV(3) //PORTD
#define _c _BV(2) //PORTD
#define _d _BV(1) //PORTD
#define _e _BV(0) //PORTD
#define _f _BV(5) //PORTD
#define _g _BV(6) //PORTD
#define _h _BV(0) // PORTB, точка (dp)

#if (common_anode == 1)
/** макрос, осуществляющий необходимую коррекцию для индикаторов с общим анодом или катодом
 * @param x - битовая маска, подлежащая конвертированию
 */
#define _pref(x) ((uint8_t)~(x))
#else
/** макрос, осуществляющий необходимую коррекцию для индикаторов с общим анодом или катодом
 * @param x - битовая маска, подлежащая конвертированию
 */
#define _pref(x) (x)
#endif

/// символ "точка"
#define _comma _pref(_h)
/// символ "пробел"
#define _space _pref(0)
/// символ "минус"
#define _minus _pref((_g))
/// символ "квадратик"
#define _square_lo _pref((_c)|(_d)|(_e)|(_g))
/// символ "градус"
#define _square_hi _pref((_a)|(_b)|(_f)|(_g))

/// символ "1"
#define d_1 _pref((_b)|(_c))
/// символ "2"
#define d_2 _pref((_a)|(_b)|(_g)|(_e)|(_d))
/// символ "3"
#define d_3 _pref((_a)|(_b)|(_c)|(_d)|(_g))
/// символ "4"
#define d_4 _pref((_b)|(_c)|(_f)|(_g))
/// символ "5"
#define d_5 _pref((_a)|(_c)|(_d)|(_f)|(_g))
/// символ "6"
#define d_6 _pref((_a)|(_c)|(_d)|(_e)|(_f)|(_g))
/// символ "7"
#define d_7 _pref((_a)|(_b)|(_c))
/// символ "8"
#define d_8 _pref((_a)|(_b)|(_c)|(_d)|(_e)|(_f)|(_g))
/// символ "9"
#define d_9 _pref((_a)|(_b)|(_c)|(_d)|(_f)|(_g))
/// символ "0"
#define d_0 _pref((_a)|(_b)|(_c)|(_d)|(_e)|(_f))
/// символ "A"
#define d_A _pref((_a)|(_b)|(_c)|(_e)|(_f)|(_g))
/// символ "B"
#define d_B _pref((_c)|(_d)|(_e)|(_f)|(_g))
/// символ "C"
#define d_C _pref((_a)|(_d)|(_e)|(_f))
/// символ "D"
#define d_D _pref((_b)|(_c)|(_d)|(_e)|(_g))
/// символ "E"
#define d_E _pref((_a)|(_d)|(_e)|(_f)|(_g))
/// символ "F"
#define d_F _pref((_a)|(_e)|(_f)|(_g))

#define c_a (d_8 ^ _d)
#define c_A (d_A)
#define c_b (d_6 ^ _a)
#define c_B (d_8)
#define c_C (d_C)
#define c_c (_square_lo ^ _c)
#define c_d (d_D)
#define c_D (d_D)
#define c_e (d_E)
#define c_E (d_E)
#define c_f (d_F)
#define c_F (d_F)
#define c_g (d_0 ^ _b)
#define c_G (c_g)
#define c_h (c_b ^ _d)
#define c_H (d_8 ^ _d ^ _a)
#define c_i _pref(_c)
#define c_I (d_1)
#define c_j (d_0 ^ _a ^ _f)
#define c_J (c_j)
#define c_l _pref((_d)|(_e)|(_f))
#define c_L (c_l)
#define c_n (_square_lo ^ _d)
#define c_N (c_n)
#define c_o (_square_lo)
#define c_O (d_0)
#define c_p (d_8 ^ _c ^ _d)
#define c_P (c_p)
#define c_r (_square_lo ^ _c ^ _d)
#define c_R (c_r)
#define c_s (d_5)
#define c_S (c_s)
#define c_t (d_8 ^ _a ^ _b ^ _c)
#define c_T (c_t)
#define c_u (_square_lo ^ _g)
#define c_U (d_0 ^ _a)
#define c_y (d_9 ^ _a)
#define c_Y (c_y)
#define _equal _pref(_g | _a)

#endif