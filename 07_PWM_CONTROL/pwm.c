#include "button.h"
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

void hw_pwm_fan_control();
void init_timer3_pwm(void);

extern int get_button(int button_num, int button_pin);

/*
	16bit 3번 timer/counter를 사용
	pwm출력 신호
	============
	PE3 : OC3A
	PE4 : OC3B    현재 초음파센서 INT4가 연결되어 있음
	PE5 : OC3C <-- 모터 연결
	BTN0 : start/stop
	BTN1 : speed-up (OCR3C : 20증가 최대 250)
	BTN2 : speed-down (OCR3C : 20감소 최소 60)
*/
void init_timer3_pwm(void)
{
	// DDRE |= 1 << 3 | 1 << 4 | 1 << 5; 현재 4번에 초음파가 연결되어 있다.
	DDRE |= 1 << 3 | 1 << 5;
	// 모드 5 : 8비트 고속 pwm    TCCR3A레지스터 WGM30과 TCCR3B레지스터 WGM32를 1로 만든다.
	TCCR3A |= 1 << WGM30;
	TCCR3B |= 1 << WGM32;
	// 비반전모드 top : 0x00ff  비교일치값(pwm)지정 : OCR3C
	// TCCR3A레지스터의 COM3C1을 1로 set
	// 비교일치 발생시 OC3C의 출력 핀은 LOw로 바뀌고 BOTTOM에서 HIGH로 바뀐다.
	TCCR3A |= 1 << COM3C1;
	// 분주비 : 64   16,000,000hz -> 250,000hz         TCCR3B의 CS30, CS31을 1로 set
	// 주기 : 1 / 250,000 -> 4us
	// 250000hz에서 256개의 펄스를 count하면 소요시간 : 1.02ms
	//			    127개의 펄스를 count하면 소요시간 : 0.51ms
	TCCR3B |= 1 << CS31 | 1 << CS30;
	OCR3C = 0; // 비교일치값 (Output Compare Register : PWM 값
}

void hw_pwm_fan_control(void)
{
	int start_button = 0;
	
	init_timer3_pwm();
	
	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN)) // start/stop
		{
			start_button = !start_button; // 반전 toggle
			if (start_button)
			{
				OCR3C = 127; // 시작
			}
			else
			{
				OCR3C = 0; // 중지
			}
		}
		else if (get_button(BUTTON1, BUTTON1PIN)) // speed up
		{
			if (OCR3C >= 250)
			{
				OCR3C = 250;
			}
			else
			{
				OCR3C += 20; // 20씩 증가
			}
		}
		else if (get_button(BUTTON2, BUTTON2PIN)) // spead down
		{
			if (OCR3C <= 70)
			{
				OCR3C = 70;
			}
			else
			{
				OCR3C -= 20; // 20씩 감소
			}
		}
	}
}