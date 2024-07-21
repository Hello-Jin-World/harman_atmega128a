#include "button.h"
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

void hw_pwm_fan_control();
void init_timer3_pwm(void);
void init_n289n(void);
void n298n_dcmotor_pwm_control(void);
void washing_machine_fan_control();

extern int get_button(int button_num, int button_pin);
extern void shift_left_keep_ledon();
extern void shift_right_keep_ledon();

extern uint32_t sec_count; // 초를 재는 count 변수 unsigned int = uint32_t
volatile uint32_t check_timer;

int fan_forward = 1;

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
	OCR3C = 0; // 비교일치값 (Output Compare Register : PWM 값)
}

void init_n289n(void)
{
	PORTF &= ~(1 << 6 | 1 << 7);  // 6, 7 reset
	PORTF |= 1 << 6; // 정회전
}

void washing_machine_fan_control(int *spin_strength , int *forward_state)
{
	OCR3C = *spin_strength;
	
	if (check_timer >= 4500) // 4.5초마다 방향 바뀜
	{
		PORTF &= ~(1 << 6 | 1 << 7);  // 6, 7 reset
		fan_forward = !fan_forward;
		*forward_state = !*forward_state;
		check_timer = 0;
	}
	
	if (fan_forward == 1 /*&& pause_toggle == 0*/)
	{
		PORTF |= 1 << 6; // 정회전
	}
	else if (fan_forward == 0 /*&& pause_toggle == 0*/)
	{
		PORTF |= 1 << 7; // 역회전
	}
}