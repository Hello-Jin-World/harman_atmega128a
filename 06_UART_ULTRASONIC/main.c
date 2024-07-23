/*
 * 03.TIMER_CONTROL.c
 *
 * Created: 2024-07-12 ���� 3:45:57
 * Author : KCCISTC
 */ 

#define F_CPU 16000000UL   // 16MHZ
#include <avr/io.h>   // PORTA PORTB ���� IO���� register���� ��� �ִ�.
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h>   // for sei��
#include <stdio.h>   // printf scanf fgets���� ���� �Ǿ� �ִ�. 

#include "def.h"
#include "ultrasonic.h"

volatile uint32_t msec_count=0;   // ���ͷ��� ���� ��ƾ���� ���� ���� type�տ���
                                // volatile�̶�� ����
								// �̴� ����ȭ�� ���� �ϱ� �����̴�. 
volatile uint32_t fnd_dis=0;  // fnd�� �ܻ� ȿ���� ���� �ϱ� ���� ���� 2ms
volatile uint32_t ultrasonic_check_timer = 0; // 1000ms�� �ѹ��� 

int led_main(void);   // ����

extern int fnd_main(void);
extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern void init_uart1(void);
extern void UART1_transmit(uint8_t data);
extern void init_ultrasonic();
extern void ultrasonic_distance_check();
extern void pc_command_processing(void);
extern void bt_command_processing(void);

// for printf
FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)
{
	TCNT0=6;  // 6~256 : 250(1ms) �׷��� TCNT0�� 6���� ����
	msec_count++;  // 1ms���� ms_count�� 1�� ����
	fnd_dis++;   // fnd �ܻ�ȿ�� ���� �ϱ� ���� timer 2ms  	
	ultrasonic_check_timer++;
}

int main(void)
{
    init_timer0();
	init_uart0();
	init_uart1();
	init_ultrasonic();
	stdout = &OUTPUT;  // printf�� ���� �� �� �ֵ��� stdout�� OUTPUTȭ�� ������ assign
	                 				 
	DDRA=0xff;   // led�� ��� ���� 
    sei();     // ���������� interrupt ���
	
//printf("Hello SEONGJIN\n");	

#if 0
	while (1)
	{
		printf("0525\n"); // 0x41 0x12
		_delay_ms(100);
	}
#else
    while (1) 
    {
		pc_command_processing();
		//bt_command_processing();
		//ultrasonic_distance_check();
    }
#endif
}

void init_timer0()
{
// 16MHZ --> 1/64�� down (����: divider/prescable)
// 1. ���ֺ� ��� 
// 16000000HZ/64 ==> 250,000HZ
// 2. T(�ֱ�) 1�� ��� �Դ� �ð� : 1/f = 1/250,000 ==> 0.000004sec(4us) : 0.004ms
// 3. 8bit timer OV(OVflow) : 0.004ms x 256 = 0.001024sec --> 1.024ms
// �׷��� ��Ȯ�� 1ms �� ��� �ʹٸ� 0.004ms x 250�� = 0.001sec ==> 1ms
	TCNT0 = 6;   // TCNT : 0~256 ==> 1ms���� TIMER0_OVF_vect�� �����Ѵ�.
	             // TCNT0 = 6�� ������ ������: 6~256 : 250���� count(��Ȯ�� 1MS�����߱� ���ؼ�)
	             //
// 4 ���ֺ� ���� (250khz)
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;	
// 5. TIMER0 OVERFLOW�� ���(enable)
	TIMSK |= 1 << TOIE0;  // TIMSK |= 0x01;
	//sei();    // ������(�빮)���� interrupt ��� 
}