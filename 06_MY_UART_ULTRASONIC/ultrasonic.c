#include "ultrasonic.h"
#include "fnd.h"

extern int led_main(void);
extern void init_fnd(void);
extern void fnd_display(void);
extern void led_ultrasonic();
extern void fnd_display_us();

extern volatile uint32_t ultrasonic_check_timer;

volatile int ultrasonic_distance = 0;

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
	}
}

void init_ultrasonic()
{
	TRIG_DDR |= 1 << TRIG; // output mode로 설정
	ECHO_DDR &= ~(1 << ECHO); // input mode로 설정
	
	EICRB |= 0 << ISC41 | 1 << ISC40; // INT4니까 EICRB에서 ISC41, 40
	
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

void ultrasonic_led()
{
	if  (ultrasonic_check_timer >= 2)
	{
		ultrasonic_check_timer = 0;
		
		if (ultrasonic_distance / 58 >= 0 && ultrasonic_distance / 58 <= 2) // 0~2cm led 1개 on
		{
			PORTA = 0x01;
		}
		else if (ultrasonic_distance / 58 > 2 && ultrasonic_distance / 58 <= 4) // 2~4cm led 2개 on
		{
			PORTA = 0x03;
		}
		else if (ultrasonic_distance / 58 > 4 && ultrasonic_distance / 58 <= 6) // 4~6cm led 3개 on
		{
			PORTA = 0x07;
		}
		else if (ultrasonic_distance / 58 > 6 && ultrasonic_distance / 58 <= 8) // 6~8cm led 4개 on
		{
			PORTA = 0x0f;
		}
		else if (ultrasonic_distance / 58 > 8 && ultrasonic_distance / 58 <= 10) // 8~10cm led 5개 on
		{
			PORTA = 0x1f;
		}
		else if (ultrasonic_distance / 58 > 10 && ultrasonic_distance / 58 <= 12) // 10~12cm led 6개 on
		{
			PORTA = 0x3f;
		}
		else if (ultrasonic_distance / 58 > 12 && ultrasonic_distance / 58 <= 14) // 12~14cm led 7개 on
		{
			PORTA = 0x7f;
		}
		else if (ultrasonic_distance / 58 > 14 && ultrasonic_distance / 58 <= 16) // 14~16cm led 8개 on
		{
			PORTA = 0xff;
		}
		else
		{
			PORTA = 0; // 그 외엔 모든 led off
		}
		fnd_display_us(&ultrasonic_distance); // 실시간으로 fnd에 거리 display
		ultrasonic_trigger();		
	}
}