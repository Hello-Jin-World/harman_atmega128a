{#define F_CPU 16000000L
	#include <avr/io.h>
	#include <util/delay.h>
	#include <avr/interrupt.h> // sei ��
	#include "fnd.h"
	#include "led.h"

	extern int fnd_main(void);

	volatile uint32_t msec_count = 0;
	volatile uint32_t fnd_refreshrate = 0;
	ISR(TIMER0_OVF_vect) // interrupt service routine :routine�� interrupt ���� �Լ�
	{
		TCNT0 = 6; // 6 ~ 256 : 250�� (1ms) �׷��� TCNT0�� 6���� ����
		msec_count++; // 1ms���� msec_count�� 1�� ����
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
