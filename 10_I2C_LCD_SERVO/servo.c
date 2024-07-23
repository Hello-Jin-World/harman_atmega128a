/*
 * servo.c
 *
 * Created: 2024-07-23 오후 2:37:52
 *  Author: kccistc
 */ 
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include "button.h"

void init_timer1(void);
int servo_main(void);

extern void init_button(void);
extern int get_button(int button_num, int button_pin);

/*
SG90 사양
1. 0도 : 1ms
2. 90도 : 1.5ms
3. 180도 : 2ms 
*/

void init_timer1(void)
{
	//모드14 고속 PWM모드
	TCCR1A |= 1 << WGM11;
	TCCR1B |= 1 << WGM12 | 1 << WGM13;	

	// TOP : ICR1, 비교일치값 : OCR1A
	TCCR1A |= 1 << COM1A1;
	// 분주비 설정 8 16M/8 ==> 2Mhz
	TCCR1B |= 1 << CS11 | 1 << CS10; // 분주비 64   0.0000000625 * 64 = 0.000004sec 
	//TCCR1B |= 1 << CS11;
	// T = 1/f (1/16000000) => 0.0000000625sec * 8분주 ==> 0.0000005sec ==> 
	// 16bit timer이므로 최대 65535까지 count할 수 있으므로
	// 최대 65535 * 50us = 0.0327675초 (32.76ms)
	// 0.0000005 * 40000개 ==> 0.02sec(20ms)
	ICR1 = 5000; // -> 20ms 주기    0~39999
}

int servo_main(void)
{
	// servo 모터가 연결된 pin을 output mode로 설정
	DDRB |= 1 << 5;
	init_timer1();
	OCR1A = 200;
	
	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN))
		{
			OCR1A = 200;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			OCR1A = 400;
		}
		if (get_button(BUTTON2, BUTTON2PIN))
		{
			OCR1A = 650;
		}
		
		// 40000(20ms)이 100%, 2000(1ms)이 0도, 3000(1.5ms)이 90도, 4000(2ms)이 180도
// 		OCR1A = 1500;
// 		_delay_ms(1000);
// 		OCR1A = 3100;
// 		_delay_ms(1000);
// 		OCR1A = 5100;
// 		_delay_ms(1000);
	}
	return 0;
}
