#ifndef ENCPOLL_H_
#define ENCPOLL_H_

#define ENCPOLL_PORT B
#define ENCPOLL_A_PIN 4
#define ENCPOLL_B_PIN 3
#define ENCBUT_PORT B
#define ENCBUT_PIN 5

#define PIN_(port)  PIN  ## port
#define PIN(port)  PIN_(port)

#define BUTTON_ADC 0
#define BUTTON_ENC 1
#define BUTTON_PID 2
#define BOUNCE 3.
#define BUTTON_NO_PRESS 0
#define BUTTON_SHORT_PRESS 1
#define BUTTON_LONG_PRESS 2
#define BUT_MASK 16

uint8_t encoder_scan(void);
uint8_t encoder_button(void)



#endif /* ENCPOLL_H_ */