#include "button.h"
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

#define STOP 0
#define START 1
#define SETTING 2

void hw_pwm_fan_control();
void init_timer3_pwm(void);
void fan_start(void);
void fan_stop(void);
void time_setting_mode(void);

extern void init_button(); // button 초기화
extern int get_button(int button_num, int button_pin);
extern void fan_time_fnd_display(void);

extern volatile uint32_t fnd_refreshrate; // fnd 잔상효과를 유지하기 위한 변수 2ms
extern volatile uint32_t msec_count; // 1초를 세기 위한 변수
extern uint32_t sec_count; // 초를 재는 count 변수 unsigned int = uint32_t

void (*fan_fp[])() =
{
	fan_stop,
	fan_start,
	time_setting_mode
};

int fan_state = 0;
int return_enable = 1;

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
	init_button(); // button 초기화
	
	int button0_state = 0;
	int button3_state = 0;

	init_timer3_pwm();
	
	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN)) // start/stop
		{
			button0_state = !button0_state; // 반전 toggle
			if (button0_state)
			{
				fan_state = START;
				return_enable = 1;
			}
			else
			{
				fan_state = STOP;
			}
		}
		if (get_button(BUTTON2, BUTTON2PIN)) // reset
		{
			sec_count = 0;
			fan_state = STOP;
		}
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			button0_state = 0;
			fan_state = SETTING;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fan_time_fnd_display();
		}
		(*fan_fp[fan_state])();
	}
}

void fan_start(void)
{
	OCR3C = 127;
	if (sec_count == 0)
	{
		fan_state=STOP;
		OCR3C=0;
	}
	if (msec_count >= 1000)
	{
		msec_count = 0;
		sec_count--;	
	}

}

void fan_stop(void)
{
	OCR3C = 0; // 중지
}

void time_setting_mode(void)
{
	while(return_enable)
	{
		if (get_button(BUTTON0, BUTTON0PIN))
		{
			sec_count++;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			sec_count += 60;
		}
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			return_enable = 0;
			fan_state = STOP;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fan_time_fnd_display();
		}
	}
}