#include "button.h"
#include "pwm.h"
#include "fnd.h"
#include "def.h"
#include "I2C.h"
#include "I2C_LCD.h"
#include <avr/interrupt.h>
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

extern int get_button(int button_num, int button_pin);
extern void init_button();

extern void I2C_LCD_clear(void);
extern void I2C_write_byte(uint8_t address, uint8_t data);

extern void ultrasonic_trigger();

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
	int run_state;
	sec_count = 120;
	char sbuf[20];

	while(sec_count > 0)
	{
		ultrasonic_trigger();
		if (msec_count >= 1000)
		{
			msec_count = 0;
			sec_count--;
		} // 1초씩 감소

		if (fnd_refreshrate >= 2)
		{
			I2C_LCD_clear();
			fnd_refreshrate = 0;
			fnd_display(&run_state);
		} // fnd와 lcd 표시

// 		volatile int gap1= ultrasonic_right_distance - ultrasonic_left_distance;
// 		volatile int gap2= ultrasonic_left_distance - ultrasonic_right_distance;

		sprintf(sbuf,"%3d", run_state);
		
		 if (ultrasonic_right_distance <= 3 && ultrasonic_center_distance <= 3)
		{
			run_state = BACKWARD;
			backward(600);
		}
		else if (ultrasonic_left_distance <= 3 && ultrasonic_center_distance <= 3)
		{
			run_state = BACKWARD;
			backward(600);
		}
		else if (ultrasonic_left_distance <= 3 && ultrasonic_right_distance <= 3)
		{
			run_state = BACKWARD;
			backward(600);
		}
		else if (ultrasonic_left_distance <= 3 && ultrasonic_center_distance <= 3 && ultrasonic_right_distance <= 3)
		{
			run_state = BACKWARD;
			backward(600);
		}
		else if(ultrasonic_right_distance - ultrasonic_left_distance < 4 && ultrasonic_left_distance - ultrasonic_right_distance < 4 && ultrasonic_center_distance >= 10)
		{
			run_state = FORWARD;
			forward(300);
		}
		else if (ultrasonic_center_distance >= 800 || ultrasonic_left_distance >= 800 || ultrasonic_right_distance >= 800)
		{
			run_state = BACKWARD;
			backward(400);
		}
 		else if(ultrasonic_center_distance <= 4)
 		{
 			run_state = BACKWARD;
 			backward(600);
 		}
 		else if (ultrasonic_right_distance <= 4)
 		{
 			run_state = BACKWARD;
 			turn_left(400);
 		}
 		else if (ultrasonic_left_distance <= 4)
 		{
 			run_state = BACKWARD;
 			turn_right(400);
 		}
		 ///좁은 길
		else if (ultrasonic_right_distance <= 7 && ultrasonic_center_distance <= 15 && ultrasonic_right_distance > 3 && ultrasonic_center_distance > 3)
		{
			run_state = TURN_LEFT;
			turn_left(400);
		}
		else if (ultrasonic_left_distance <= 7 && ultrasonic_center_distance <= 15 && ultrasonic_left_distance > 3 && ultrasonic_center_distance > 3)
		{
			run_state = TURN_RIGHT;
			turn_right(400);
		}		
		
		 ///
		 
		 /// 넓은 길
		 else if (ultrasonic_right_distance <= 20 && ultrasonic_center_distance <= 30 && ultrasonic_right_distance > 3 && ultrasonic_center_distance > 3)
		 {
			 run_state = TURN_LEFT;
			 turn_left(400);
		 }
		 else if (ultrasonic_left_distance <= 20 && ultrasonic_center_distance <= 30 && ultrasonic_left_distance > 3 && ultrasonic_center_distance > 3)
		 {
			 run_state = TURN_RIGHT;
			 turn_right(400);
		 }
		 ///
		 else if (ultrasonic_right_distance <= 20)
		 {
			 run_state = TURN_LEFT;
			 turn_left(400);
		 }
		 else if (ultrasonic_left_distance <= 20)
		 {
			 run_state = TURN_RIGHT;
			 turn_right(400);
		 }
		 
		
		else if ((ultrasonic_left_distance - ultrasonic_right_distance <= 2 && ultrasonic_right_distance - ultrasonic_left_distance <= 2) || ultrasonic_center_distance >= 20 && ultrasonic_center_distance <= 200)
		{
			run_state = FORWARD;
			forward(350);
		}
		else if(ultrasonic_right_distance - ultrasonic_left_distance <= 4 && ultrasonic_center_distance <= 7 && ultrasonic_right_distance > 5)
		{
			run_state = TURN_LEFT;
			turn_left(400);
		}
		else if(ultrasonic_left_distance - ultrasonic_right_distance <= 4 && ultrasonic_center_distance <= 7 && ultrasonic_left_distance > 5)
		{
			run_state = TURN_RIGHT;
			turn_right(400);
		}
		I2C_LCD_write_string_XY(1,0,sbuf);
// 		else
// 		{
// 			run_state = BACKWARD;
// 			forward(350);
// 		}
	}
}
