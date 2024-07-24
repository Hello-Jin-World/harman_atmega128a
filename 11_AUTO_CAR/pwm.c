#include "button.h"
#include "pwm.h"

#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

extern int get_button(int button_num, int button_pin);
extern void init_button();

void init_timer1_pwm(void);
void init_n289n(void);

/*
	16bit 1번 timer/counter를 사용
	
	PWM출력 신호
	============
	PB5 : OC1A     왼쪽 바퀴
	PB6 : OC1B   오른쪽 바퀴

	BTN0 : auto / manual

	방향 설정
	=========
	1. LEFT MOTOR (DC MOTOR DRIVER)
	PORTF0 : IN1
	PORTF1 : IN2
	
	2. RIGHT MOTOR (DC MOTOR DRIVER)
	PORTF2 : IN3
	PORTF3 : IN3
	
	IN1/IN3   IN2/IN4
	=======   =======
	   0         1   : 역회전
	   1         0   : 정회전
	   1         1   : STOP
*/

void init_n289n(void)
{
	MOTER_PWM_DDR |= 1 << MOTER_LEFT_PORT_DDR | 1 << MOTER_RIGHT_PORT_DDR; // DDR설정
	MOTER_DRIVER_DIRECTION_PORT_DDR |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3; // PF1234 출력으로 설정
	
	MOTER_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTER_DRIVER_DIRECTION_PORT |= 1 << 0 | 1 << 2; // 자동차를 전진모드로 
}

void init_timer1_pwm(void)
{
	// 모드 14 : 고속 pwm 
	TCCR1A |= 1 << WGM11; // TOP --> ICR1에 설정
	TCCR1B |= 1 << WGM12 | 1 << WGM13;
	// 비반전모드 top : ICR1  비교일치값(pwm)지정 : OCR1A, OCR1B
	// TCCR1A레지스터의 COM1A1을 1로 set
	// 비교일치 발생시 OCR1A, OCR1B의 출력 핀은 LOW로 바뀌고 BOTTOM에서 HIGH로 바뀐다.
	TCCR1A |= 1 << COM1A1;
	TCCR1B |= 1 << COM1B1;
	// 분주비 : 64   16,000,000hz -> 250,000hz        
	// 주기 : 1 / 250,000 -> 4us
	
	TCCR1B |= 1 << CS10 | 1 << CS11; // 분주비 64로 세팅
	
	ICR1 = 0x3ff; // 약 4ms  TOP : PWM값
}