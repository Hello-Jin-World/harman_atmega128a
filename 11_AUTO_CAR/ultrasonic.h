/*
 * ultrasonic.h
 *
 * Created: 2024-07-17 오전 9:32:29
 *  Author: kccistc
 */ 

#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#define F_CPU 16000000UL   // 16MHZ
#include <avr/io.h>   // PORTA PORTB 등의 IO관련 register들이 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h>   // for sei등
#include <stdio.h>   // printf scanf fgets등이 정의 되어 있다.

//------------ timer 3을 사용 ---------------//
#define US_TCNT_LEFT TCNT3 // timer3 16bit
#define TRIG_DDR_LEFT DDRA
#define TRIG_PORT_LEFT PORTA
#define TRIG_LEFT 0

#define US_TCNT_CENTER TCNT3 // timer3 16bit
#define TRIG_DDR_CENTER DDRA
#define TRIG_PORT_CENTER PORTA
#define TRIG_CENTER 1

#define US_TCNT_RIGHT TCNT3 // timer3 16bit
#define TRIG_DDR_RIGHT DDRA
#define TRIG_PORT_RIGHT PORTA
#define TRIG_RIGHT 2

// HW EXT interrupt 4 PE4
#define ECHO_DDR_LEFT DDRE
#define ECHO_PIN_LEFT PINE
#define ECHO_LEFT 4

#define ECHO_DDR_CENTER DDRE
#define ECHO_PIN_CENTER PINE
#define ECHO_CENTER 5

#define ECHO_DDR_RIGHT DDRE
#define ECHO_PIN_RIGHT PINE
#define ECHO_RIGHT 6


void init_ultrasonic();
void ultrasonic_trigger();
void ultrasonic_distance_check();

#endif /* ULTRASONIC_H_ */