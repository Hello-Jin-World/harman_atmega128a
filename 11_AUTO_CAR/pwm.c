#include "button.h"
#include "pwm.h"
#include "fnd.h"
#include "def.h"
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

extern int get_button(int button_num, int button_pin);
extern void init_button();

extern void fnd_display();

extern volatile uint32_t msec_count;
extern uint32_t sec_count;
extern volatile uint32_t fnd_refreshrate;
extern volatile uint32_t read_distance;

extern volatile int ultrasonic_left_distance;
extern volatile int ultrasonic_center_distance;
extern volatile int ultrasonic_right_distance;

void init_timer1_pwm(void);
void init_n289n(void);
void washing_machine_fan_control(void);

void forward(int speed);
void backward(int speed);
void turn_left(int speed);
void turn_right(int speed);
void stop(void);

/*
	16bit 1번 timer/counter를 사용
	
	PWM출력 신호
	============
	PB5 : OC1A     왼쪽 바퀴
	PB6 : OC1B   오른쪽 바퀴

	BTN0 : auto / manual

	방향 설정
	=========
	1. LEFT MOTOR (DC MOTOR DRIVER)
	PORTF0 : IN1
	PORTF1 : IN2
	
	2. RIGHT MOTOR (DC MOTOR DRIVER)
	PORTF2 : IN3
	PORTF3 : IN3
	
	IN1/IN3   IN2/IN4
	=======   =======
	   0         1   : 역회전
	   1         0   : 정회전
	   1         1   : STOP
*/

void init_n289n(void)
{
	MOTOR_PWM_DDR |= 1 << MOTOR_LEFT_PORT_DDR | 1 << MOTOR_RIGHT_PORT_DDR; // DDR설정
	MOTOR_DRIVER_DIRECTION_PORT_DDR |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3; // PF1234 출력으로 설정
	
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 0 | 1 << 2; // 자동차를 전진모드로 
}

void init_timer1_pwm(void)
{
	// 분주비 : 64 16000000HZ/64 ==> 250000HZ(250kHZ)
	// T=1/f 1/250000HZ ==> 0.000004sec (4us)
	// 250000HZ에서 256개의 펄스를 count하면 소요시간 : 1.02ms
	//              127                             : 0.5ms
	//              0x3ff(1023) --> 4ms
	TCCR1B |= 1 << CS11 | 1 << CS10;   // 분주비 64  P318 표14-1
	
	// 모드 14: 고속 PWM timer1사용  (P327 표14-5)
	TCCR1A |= 1 << WGM11;   // TOP --> ICR1에 설정
	TCCR1B |= 1 << WGM13 | 1 << WGM12;
	
	// 비반전모드 top: ICR1 비교일치값(PWM) 지정 : OCR1A, OCR1B P350 표15-7
	// 비교일치 발생시 OCR1A, OCR1B의 출력 핀은 LOW로 바뀌고 BOTTOM에서 HIGH로 바뀐다.
	TCCR1A |= 1 << COM1A1;
	TCCR1A |= 1 << COM1B1;

	ICR1 = 0x3ff;  // 1023 ==> 4ms TOP : PWM 값
}

///////////////////////////////           수동모드          //////////////////////////////////////
void forward(int speed)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 2 | 1 << 0;  // 전진 모드로 설정
	
	OCR1A = speed;  // PB5 PWM 출력 port left
	OCR1B = speed;  // PB6 PWM 출력 port right
}

void backward(int speed)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 3 | 1 << 1;  // 1010 후진 모드
	
	OCR1A = speed;  // PB5 PWM 출력 port left
	OCR1B = speed;  // PB6 PWM 출력 port right
}

void turn_left(int speed)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 2 | 1 << 0;  // 전진 모드로 설정
	
	OCR1A = 0;  // PB5 PWM 출력 port left
	OCR1B = speed;  // PB6 PWM 출력 port right
}

void turn_right(int speed)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 2 | 1 << 0;  // 전진 모드로 설정
	
	OCR1A = speed;  // PB5 PWM 출력 port left
	OCR1B = 0;  // PB6 PWM 출력 port right
}

void stop(void)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3;  // stop 모드로 설정
	
	OCR1A = 0;  // PB5 PWM 출력 port left
	OCR1B = 0;  // PB6 PWM 출력 port right
}
////////////////////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////         자동모드            //////////////////////////////////
void auto_start(void)
{
	sec_count = 4;
	while (sec_count > 0)
	{
		if (msec_count >= 1000)
		{
			msec_count = 0;
			sec_count--;
		} // 1초씩 감소
		if (fnd_refreshrate >= 2)
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}

	int run_state;	
	sec_count = 120;
	while(sec_count > 0)
	{
		ultrasonic_trigger();
		volatile int gap1 = ultrasonic_right_distance - ultrasonic_left_distance;
		volatile int gap2 = ultrasonic_left_distance - ultrasonic_right_distance;


////////////new//////////////////////////
		if (msec_count >= 1000)
		{
			msec_count = 0;
			sec_count--;
		} // 1초씩 감소
		
		if (ultrasonic_center_distance >= 10)
		{
			if (ultrasonic_left_distance >= 7 && ultrasonic_right_distance >= 7)
			{
				run_state = FORWARD;
				forward(400);
			}
		}
		else
		{
			set_car_location(&run_state);
		}
		
		if (fnd_refreshrate >= 2)
		{
			fnd_refreshrate = 0;
			fnd_display(&run_state);
		} // fnd 표시
	}
#if 0
////////////////////////////////////////

		if (msec_count >= 1000)
		{
			msec_count = 0;
			sec_count--;
		} // 1초씩 감소
		
		if (fnd_refreshrate >= 2)
		{
			fnd_refreshrate = 0;
			fnd_display();
		} // fnd 표시
	
		if (ultrasonic_center_distance <= 25)
		{
			
			if (gap1 > 3)
			{
				if (ultrasonic_left_distance <= 3)
				{
					backward(500);
				}
				else
				{
					turn_right(400);
				}
			}
			else if (gap2 > 3)
			{
				if (ultrasonic_right_distance <= 3)
				{
					backward(500);
				}
				else
				{
					turn_left(400);
				}
			}
			else if (ultrasonic_left_distance <= 3  || ultrasonic_right_distance <= 3)
			{
				backward(600);
			}
			else
			{
				forward(400);
			}
		}
// 		else if (ultrasonic_center_distance <= 15 && ultrasonic_left_distance <= 10)
// 		{
// 			turn_right(450);
// 		}
// 		else if (ultrasonic_center_distance <= 15 && ultrasonic_right_distance <= 10)
// 		{
// 			turn_left(450);
// 		}
		else
		{
			forward(400);
		}
	}
#endif
}

void set_car_location(int *run_state)
{
	int set_comp = 1;
	while (set_comp)
	{
		if (msec_count >= 1000)
		{
			msec_count = 0;
			sec_count--;
		} // 1초씩 감소
		
		ultrasonic_trigger();
		volatile int l_gap = ultrasonic_right_distance - ultrasonic_left_distance; // 왼쪽에 가까울 때
		volatile int r_gap = ultrasonic_left_distance - ultrasonic_right_distance; // 오른쪽에 가까울 때
		
		if (r_gap >= 4)
		{
			run_state = TURN_LEFT;
			turn_left(400);
		}
		
		else if(l_gap >= 4)
		{
			run_state = TURN_RIGHT;
			turn_right(400);
		}
		
		else if (l_gap < 3 && r_gap < 3)
		{
			set_comp = 0;
		}
		
		else if (ultrasonic_left_distance < 7)
		{
			run_state = TURN_RIGHT;
			backward(400);
		}
		
		else if (ultrasonic_right_distance < 7)
		{
			run_state = TURN_LEFT;
			backward(400);
		}
		
		else
		{
			run_state = BACKWARD;
			backward(400);
		}
		
		if (fnd_refreshrate >= 2)
		{
			fnd_refreshrate = 0;
			fnd_display(&run_state);
		}
	}
}