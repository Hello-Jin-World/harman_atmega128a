#define F_CPU 16000000UL   // 16MHZ
#include <avr/io.h>   // PORTA PORTB 등의 IO관련 register들이 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h>   // for sei등
#include <stdio.h>   // printf scanf fgets등이 정의 되어 있다.
#include "def.h"

volatile uint32_t msec_count = 0;
volatile uint32_t fnd_refreshrate = 0;
volatile uint32_t ultrasonic_check_timer = 0;

extern volatile uint8_t bt_data;
extern int button0_state;

extern void init_led(); // led 초기화
extern void init_button(); // 버튼 초기화
extern void init_timer1_pwm();
extern void init_n289n();
extern void pc_command_processing();
extern void washing_machine_fan_control();
extern void UART0_transmit();
extern void distance_check();
extern void forward();
extern void backward();
extern void turn_left();
extern void turn_right();
extern void stop();
extern void auto_mode_check();
extern void I2C_LCD_init();
extern void auto_start();

void manual_mode();
void auto_mode_check();
void auto_mode();

int func_state = 0; // pfunction을 찾아가는 인덱스값
extern int current_speed;

void (*pfunc[])() =
{
	manual_mode, // bt_command run
	distance_check, // 초음파 거리 측정
	auto_mode_check, // button0 체크
	auto_start // 자율 주행
};

// for printf
FILE OUTPUT = FDEV_SETUP_STREAM(UART0_transmit, NULL, _FDEV_SETUP_WRITE);

ISR(TIMER0_OVF_vect)
{
	TCNT0 = 6;  // 6~256 : 250(1ms) 그래서 TCNT0를 6으로 설정
	msec_count++;  // 1ms마다 ms_count가 1씩 증가
	fnd_refreshrate++;   // fnd 잔상효과 유지 하기 위한 timer 2ms
	ultrasonic_check_timer++;
}

int main(void)
{
	I2C_LCD_init();
	init_led();
	init_fnd();
	init_button();
	init_timer0();
	init_uart0();
	init_uart1();
	init_n289n();
	init_timer1_pwm();
	init_ultrasonic(); // timer 3
	
	stdout = &OUTPUT;  // printf가 동작 될 수 있도록 stdout에 OUTPUT화일 포인터 assign
	
	sei();     // 전역적으로 interrupt 허용

	while (1)
	{
		//I2C_LCD_Test(&current_speed);
		pfunc[func_state] ();
	}
}

void init_timer0()
{
	// 16MHZ --> 1/64로 down (분주: divider/prescable)
	// 1. 분주비 계산
	// 16000000HZ/64 ==> 250,000HZ
	// 2. T(주기) 1가 잡아 먹는 시간 : 1/f = 1/250,000 ==> 0.000004sec(4us) : 0.004ms
	// 3. 8bit timer OV(OVflow) : 0.004ms x 256 = 0.001024sec --> 1.024ms
	// 그러면 정확히 1ms 를 재고 싶다면 0.004ms x 250개 = 0.001sec ==> 1ms
	TCNT0 = 6;   // TCNT : 0~256 ==> 1ms마다 TIMER0_OVF_vect로 진입한다.
	// TCNT0 = 6로 설정을 한이유: 6~256 : 250개를 count(정확히 1MS를맞추기 위해서)
	//
	// 4 분주비를 설정 (250khz)
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;
	// 5. TIMER0 OVERFLOW를 허용(enable)
	TIMSK |= 1 << TOIE0;  // TIMSK |= 0x01;
	//sei();    // 전역적(대문)으로 interrupt 허용
}

void manual_mode(void)
{
	switch (bt_data)
	{
		case 'F':
		case 'f':
			forward(500);
			break;
			
		case 'B':
		case 'b':
			backward(500);
			break;
			
		case 'L':
		case 'l':
			turn_left(700);
			break;
			
		case 'R':
		case 'r':
			turn_right(700);
			break;
			
		case 'S':
		case 's':
			stop();
			break;
			
		default:
			break;
	}
	func_state = DISTANCE_CHECK;
}