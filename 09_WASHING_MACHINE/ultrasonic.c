/*
 * ultrasonic.c
 *
 * Created: 2024-07-17 오전 9:33:26
 *  Author: kccistc
 */ 

#include "ultrasonic.h"
extern volatile uint32_t closed_check_timer;

volatile int ultrasonic_distance = 0;
char scm[50];

// PE4 : 외부 INT4 초음파 센서 상승, 하강 edge 둘 다 이곳으로 들어온다.
//결국은 상승edge에서 1번, 하강edge에서 1번씩 이곳으로 들어온다.
ISR(INT4_vect)
{
	// 1. 상승 edge
	if (ECHO_PIN & (1 << ECHO))
	{
		TCNT1 = 0;
	}
	// 2. 하강 edge
	else
	{
		// ECHO 핀에 들어온 펄스 길이를 us로 환산
		ultrasonic_distance = 1000000.0 * TCNT1 * 1024 / F_CPU;
		
		// 예) TCNT1에 10이 들어 있다고 가정하자
		// 15.625khz의 1주기가 64us이다.
		// 64us * 10 == 640us
		// 640us / 58us (58us는 초음파 센서에서 1cm이동하는데 58us가 소요됨). ==> 11cm 
		sprintf(scm, "dis : %dcm\n", ultrasonic_distance / 58); // sprintf는 버퍼에 찍는 것
	}
}

void init_ultrasonic()
{
	TRIG_DDR |= 1 << TRIG; // output mode로 설정
	ECHO_DDR &= ~(1 << ECHO); // input mode로 설정
	
	// 0 1 : 상승, 하강 edge에 둘 다 INT를 띄우도록 설정
	// INT0~3은 EICRA, INT4~7은 EICRB 레지스터
	EICRB |= 0 << ISC41 | 1 << ISC40; // INT4니까 EICRB에서 ISC41, 40
	
	// 16bit timer1을 설정해서 사용 16비트는 0~65535(0xffff)가 최대
	// 16Mhz를 1024로 분주 16000000/1024 --> 15625hz --> 15.625khz
	// 1주기 (1개의 펄스 소요시간) 1/15625 = 0.000064sec = 64us	
	TCCR1B |=  1 << CS12 | 1 << CS10; // 1024로 분주
	EIMSK |= 1 << INT4; // 외부 인터럽트4번 (ECHO) 사용
}

void ultrasonic_trigger()
{
	TRIG_PORT &= ~(1 << TRIG); // 해당되는 포트만 LOW로 만듦
	_delay_us(1);
	TRIG_PORT |= 1 << TRIG; // HIGH
	_delay_us(15); // 규격에는 10us인데 reduance
	TRIG_PORT &= ~(1 << TRIG); // LOW
}

void ultrasonic_distance_check()
{
	if (closed_check_timer >= 1000) // 1초가 되면
	{
		closed_check_timer = 0;
		ultrasonic_trigger();
	}
}