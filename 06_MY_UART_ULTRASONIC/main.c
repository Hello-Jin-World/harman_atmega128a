#define F_CPU 16000000UL   // 16MHZ
#include <avr/io.h>   // PORTA PORTB 등의 IO관련 register들이 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h>   // for sei등
#include <stdio.h>   // printf scanf fgets등이 정의 되어 있다. 

#include "def.h"
#include "ultrasonic.h"

volatile uint32_t msec_count=0;   
volatile uint32_t fnd_dis=0;  // fnd의 잔상 효과를 유지 하기 위한 변수 2ms
volatile uint32_t ultrasonic_check_timer = 0; // 1000ms에 한번씩 

extern int led_main(void);   // 선언

extern int fnd_main(void);
extern void fnd_init(); // fnd 초기화
extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern void init_ultrasonic();
//extern void ultrasonic_distance_check();
extern void ultrasonic_led();

// for printf
FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)
{
	TCNT0=6;  // 6~256 : 250(1ms) 그래서 TCNT0를 6으로 설정
	msec_count++;  // 1ms마다 ms_count가 1씩 증가
	fnd_dis++;   // fnd 잔상효과 유지 하기 위한 timer 2ms  	
	ultrasonic_check_timer++;
}

int main(void)
{
	init_fnd();
    init_timer0();
	init_uart0();
	init_ultrasonic();
	stdout = &OUTPUT;  // printf가 동작 될 수 있도록 stdout에 OUTPUT화일 포인터 assign
	                 				 
	DDRA=0xff;   // led를 출력 모드로 
	PORTA = 0x00;
    sei();     // 전역적으로 interrupt 허용
	
    while (1) 
    {
		ultrasonic_led();
    }
}

void init_timer0()
{
	TCNT0 = 6;  

	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;	

	TIMSK |= 1 << TOIE0;  // TIMSK |= 0x01;
	//sei();
}