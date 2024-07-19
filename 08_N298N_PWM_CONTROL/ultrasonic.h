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

#define TRIG_DDR DDRG
#define TRIG_PORT PORTG
#define TRIG 4

#define ECHO_DDR DDRE
#define ECHO_PIN PINE // INT4
#define ECHO 4

void init_ultrasonic();
void ultrasonic_trigger();
void ultrasonic_distance_check();

#endif /* ULTRASONIC_H_ */