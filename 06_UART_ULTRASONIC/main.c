/*
 * 03.TIMER_CONTROL.c
 *
 * Created: 2024-07-12 오후 3:45:57
 * Author : KCCISTC
 */ 

#define F_CPU 16000000UL   // 16MHZ
#include <avr/io.h>   // PORTA PORTB 등의 IO관련 register들이 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h>   // for sei등
#include <stdio.h>   // printf scanf fgets등이 정의 되어 있다. 

#include "def.h"

volatile uint32_t msec_count=0;   // 인터럽드 서비스 루틴에서 쓰는 변수 type앞에는
                                // volatile이라고 선언
								// 이는 최적화를 방지 하기 위함이다. 
volatile uint32_t fnd_dis=0;  // fnd의 잔상 효과를 유지 하기 위한 변수 2ms
int led_main(void);   // 선언
extern int fnd_main(void);
extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);

// for printf
FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)
{
	TCNT0=6;  // 6~256 : 250(1ms) 그래서 TCNT0를 6으로 설정
	msec_count++;  // 1ms마다 ms_count가 1씩 증가
	fnd_dis++;   // fnd 잔상효과 유지 하기 위한 timer 2ms  	
}

int main(void)
{
    init_timer0();
	init_uart0();
	stdout = &OUTPUT;  // printf가 동작 될 수 있도록 stdout에 OUTPUT화일 포인터 assign
	                 				 
	DDRA=0xff;   // led를 출력 모드로 
    sei();     // 전역적으로 interrupt 허용
printf("Hello SEONGJIN\n");	
    while (1) 
    {
		pc_command_processing();
    }
}

void init_timer0()
{
// 16MHZ --> 1/64로 down (분주: divider/prescable)
// 1. 분주비 계산 
// 16000000HZ/64 ==> 250,000HZ
// 2. T(주기) 1가 잡아 먹는 시간 : 1/f = 1/250,000 ==> 0.000004sec(4us) : 0.004ms
// 3. 8bit timer OV(OVflow) : 0.004ms x 256 = 0.001024sec --> 1.024ms
// 그러면 정확히 1ms 를 재고 싶다면 0.004ms x 250개 = 0.001sec ==> 1ms
	TCNT0 = 6;   // TCNT : 0~256 ==> 1ms마다 TIMER0_OVF_vect로 진입한다.
	             // TCNT0 = 6로 설정을 한이유: 6~256 : 250개를 count(정확히 1MS를맞추기 위해서)
	             //
// 4 분주비를 설정 (250khz)
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;	
// 5. TIMER0 OVERFLOW를 허용(enable)
	TIMSK |= 1 << TOIE0;  // TIMSK |= 0x01;
	sei();    // 전역적(대문)으로 interrupt 허용 
}