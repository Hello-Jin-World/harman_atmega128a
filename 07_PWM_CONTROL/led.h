
/*
 * led.h
 *
 * Created: 2024-07-11 오전 9:28:53
 *  Author: kccistc
 */ 

//분할 컴파일 : 기능이 같은 것끼리 파일을 만들어서 파일 이름을 보면 무슨 기능이 들어있는지 식별이 가능하도록 한다.

#ifndef LED_H_
#define LED_H_ // 1을 갖고 있음.
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

#define LED_ALL_ON 0
#define LED_ALL_OFF 1
#define SHIFT_LEFT_LEDON 2
#define SHIFT_RIGHT_LEDON 3
#define SHIFT_LEFT_KEEP_LEDON 4
#define SHIFT_RIGHT_KEEP_LEDON 5
#define FLOWERON 6
#define FLOWEROFF 7


#endif /* LED_H_ */