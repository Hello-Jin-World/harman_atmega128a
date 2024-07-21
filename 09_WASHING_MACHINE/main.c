#define F_CPU 16000000UL   // 16MHZ
#include <avr/io.h>   // PORTA PORTB 등의 IO관련 register들이 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h>   // for sei등
#include <stdio.h>   // printf scanf fgets등이 정의 되어 있다.

#include "def.h"
#include "ultrasonic.h"

volatile uint32_t msec_count=0;   // 인터럽드 서비스 루틴에서 쓰는 변수 type앞에는
// volatile이라고 선언
// 이는 최적화를 방지 하기 위함이다.
volatile uint32_t fnd_refreshrate=0;  // fnd의 잔상 효과를 유지 하기 위한 변수 2ms
volatile uint32_t check_timer = 0; // 5000ms에 한번씩
volatile uint32_t loading_clock_change = 0;
volatile uint32_t loading_refreshrate = 0;
volatile uint32_t closed_check_timer = 0;

int led_main(void);   // 선언

extern int fnd_main(void);
extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern void init_uart1(void);
extern void UART1_transmit(uint8_t data);
extern void init_ultrasonic();
extern void ultrasonic_distance_check();
extern void pc_command_processing(void);
extern void bt_command_processing(void);
extern void make_pwm_led_control(void);
extern void init_button(void);
extern void hw_pwm_fan_control(void);
extern void n298n_dcmotor_pwm_control(void);

extern void init_timer3_pwm();
extern void init_n289n();

extern void buzzer_main();

FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)
{
	TCNT0=6;
	msec_count++;
	fnd_refreshrate++;
	check_timer++;
	loading_clock_change++;
	loading_refreshrate++;
	closed_check_timer++;
}

int main(void)
{
	init_timer0();
	init_uart0();
	init_uart1();
	init_ultrasonic();
	init_button();
	init_timer3_pwm();
	init_n289n();
	
	stdout = &OUTPUT;  // printf가 동작 될 수 있도록 stdout에 OUTPUT화일 포인터 assign
	
	DDRA=0xff;   // led를 출력 모드로
	sei();     // 전역적으로 interrupt 허용
	
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
}