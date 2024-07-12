/*
 * button.h
 *
 * Created: 2024-07-11 오전 11:33:19
 *  Author: kccistc
 */ 


#ifndef BUTTON_H_
#define BUTTON_H_ // 1을 갖고 있음.
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

#define LED_DDR DDRA  // 이렇게 하는 이유는 LED PORT가 변경되면 이 #define만 바꿔주면 compiler가 이후의 logic을 찾아서 쉽게 변경을 해주기 위함이다.
#define LED_PORT PORTA

#define BUTTON_DDR DDRD
#define BUTTON_PIN PIND // PORTD를 읽는 register 5v:1, 0v:0

#define BUTTON0PIN 4 // PORTD의 4번이다.
#define BUTTON1PIN 5 // PORTD의 5번이다.
#define BUTTON2PIN 6 // PORTD의 6번이다.
#define BUTTON3PIN 7 // PORTD의 7번이다.

#define BUTTON0 0 // PORTD.4의 가상 index (sw 번호)
#define BUTTON1 1 // PORTD.4의 가상 index (sw 번호)
#define BUTTON2 2 // PORTD.4의 가상 index (sw 번호)
#define BUTTON3 3 // PORTD.4의 가상 index (sw 번호)

#define BUTTON_PRESS 1 // 버튼을 누르면 high (active-high)
#define BUTTON_RELEASE 0 // 버튼을 떼면 low
#define BUTTON_NUMBER 4 // 버튼 개수

#endif /* BUTTON_H_ */