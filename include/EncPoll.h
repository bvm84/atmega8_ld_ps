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
#define BOUNCE 3
#define BUTTON_LONG_ON 2
#define BUT_MASK 16

extern uint16_t EncoderValue;
extern uint8_t ButtonState;


PT_THREAD(EncoderScan(struct pt *pt));
PT_THREAD(EncoderButton(struct pt *pt));



#endif /* ENCPOLL_H_ */