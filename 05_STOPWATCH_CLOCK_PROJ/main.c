#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> // sei 등
#include "fnd.h"
#include "led.h"

extern int fnd_main(void);

volatile uint32_t msec_count = 0;
volatile uint32_t fnd_refreshrate = 0;
ISR(TIMER0_OVF_vect) // interrupt service routine :routine은 interrupt 서비스 함수
{
	TCNT0 = 6; // 6 ~ 256 : 250개 (1ms) 그래서 TCNT0를 6으로 설정
	msec_count++; // 1ms마다 msec_count가 1씩 증가
	fnd_refreshrate++;
}

int main(void)
{
	init_timer0();
	init_fnd();
	
	fnd_main();
	while (1)
	{
	}
}

void init_timer0()
{
	TCNT0 = 6;

	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;

	TIMSK |= 1 << TOIE0;

	sei();

}
