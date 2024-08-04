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

extern void I2C_LCD_Test();

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

void forward();
void backward();
void turn_left();
void turn_right();
void stop(void);

int current_speed = 0; // 속도
int current_mode = 0;

void (*current_location[])() =
{
	forward,
	backward,
	turn_left,
	turn_right
};
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
void forward(int *speed)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 2 | 1 << 0;  // 전진 모드로 설정
	
	OCR1A = *speed;  // PB5 PWM 출력 port left
	OCR1B = *speed;  // PB6 PWM 출력 port right
}

void backward(int *speed)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 3 | 1 << 1;  // 1010 후진 모드
	
	OCR1A = *speed;  // PB5 PWM 출력 port left
	OCR1B = *speed;  // PB6 PWM 출력 port right
}

void turn_left(int *speed)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 2 | 1 << 0;  // 전진 모드로 설정
	
	OCR1A = 0;  // PB5 PWM 출력 port left
	OCR1B = *speed;  // PB6 PWM 출력 port right
}

void turn_right(int *speed)
{
	MOTOR_DRIVER_DIRECTION_PORT &= ~(1 << 0 | 1 << 1 | 1 << 2 | 1 << 3);
	MOTOR_DRIVER_DIRECTION_PORT |= 1 << 2 | 1 << 0;  // 전진 모드로 설정
	
	OCR1A = *speed;  // PB5 PWM 출력 port left
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
			fnd_refreshrate = 0;
			fnd_display(&run_state);
		} // fnd와 lcd 표시

// 		volatile int gap1= ultrasonic_right_distance - ultrasonic_left_distance;
// 		volatile int gap2= ultrasonic_left_distance - ultrasonic_right_distance;

		//sprintf(sbuf,"%3d", run_state);
		
		 if (ultrasonic_right_distance <= 3 && ultrasonic_center_distance <= 3) // 오른쪽이 3cm 이하이고 중간 3cm 이하일 때 후진
		{
			run_state = BACKWARD;
			current_mode = BACKWARD;
			//backward(600);
			current_speed = 600;
			//sbuf[20] = "BACKWARD";
		}
		else if (ultrasonic_left_distance <= 3 && ultrasonic_center_distance <= 3) // 왼쪽이 3cm 이하이고 중간 3cm 이하일 때 후진
		{
			run_state = BACKWARD;
			current_mode = BACKWARD;
			//backward(600);
			current_speed = 600;
			//sbuf[20] = "BACKWARD";
		}
		else if (ultrasonic_left_distance <= 3 && ultrasonic_right_distance <= 3) // 완쪽이 3cm 이하이고 오른쪽 3cm 이하일 때 후진
		{
			run_state = BACKWARD;
			current_mode = BACKWARD;
			//backward(600);
			current_speed = 600;
			//sbuf[20] = "BACKWARD";
		}
		else if(ultrasonic_center_distance <= 4) // 중간이 4cm 이하일 때 후진
		{
			run_state = BACKWARD;
			current_mode = BACKWARD;
			//backward(600);
			current_speed = 600;
			//sbuf[20] = "BACKWARD";
		}
		else if (ultrasonic_right_distance <= 4) // 오른쪽이 4cm 이하일 때 후진
		{
			run_state = TURN_LEFT;
			current_mode = TURN_LEFT;
			//turn_left(500);
			current_speed = 500;
			//sbuf[20] = "BACKWARD";
		}
		else if (ultrasonic_left_distance <= 4) // 왼쪽이 4cm 이하일 때 후진
		{
			run_state = TURN_RIGHT;
			current_mode = TURN_RIGHT;
			//turn_right(500);
			current_speed = 500;
			//sbuf[20] = "BACKWARD";
		}
		else if (ultrasonic_left_distance <= 3 && ultrasonic_center_distance <= 3 && ultrasonic_right_distance <= 3) // 왼쪽, 중간, 오른쪽 모두 3cm이하일 때 후진
		{
			run_state = BACKWARD;
			current_mode = BACKWARD;
			//backward(600);
			current_speed = 600;
			//sbuf[20] = "BACKWARD";
		}
		else if(ultrasonic_right_distance - ultrasonic_left_distance < 4 && ultrasonic_left_distance - ultrasonic_right_distance < 4 && ultrasonic_center_distance >= 10)
		{ // 왼쪽과 오른쪽의 차이가 모두 4cm 미만이고 중간이 10cm 이상일 때 전진 
			run_state = FORWARD;
			current_mode = FORWARD;
			//forward(300);
			current_speed = 300;
			//sbuf[20] = "FORWARD";
		}
		else if (ultrasonic_center_distance >= 800 || ultrasonic_left_distance >= 800 || ultrasonic_right_distance >= 800)
		{ // 초음파 센서에 딱 붙으면 값이 튀어버려서 그때는 후진
			run_state = BACKWARD;
			current_mode = BACKWARD;
			//backward(400);
			current_speed = 400;
			//sbuf[20] = "BACKWARD";
		}
 		
		 ///좁은 길
		else if (ultrasonic_right_distance <= 7 && ultrasonic_center_distance <= 15 && ultrasonic_right_distance > 3 && ultrasonic_center_distance > 3)
		{ // 오른쪽이 3 초과 7 이하이고 중간이 3 초과 15 이하일 때 좌회전
			run_state = TURN_LEFT;
			current_mode = TURN_LEFT;
			//turn_left(400);
			current_speed = 400;
			//sbuf[20] = "TRUN LEFT";
		}
		else if (ultrasonic_left_distance <= 7 && ultrasonic_center_distance <= 15 && ultrasonic_left_distance > 3 && ultrasonic_center_distance > 3)
		{ // 왼쪽이 3 초과 7 이하이고 중간이 3 초과 15 이하일 때 우회전
			run_state = TURN_RIGHT;
			current_mode = TURN_RIGHT;
			//turn_right(400);
			current_speed = 400;
			//sbuf[20] = "TRUN RIGHT";
		}		
		
		 ///
		 
		 /// 넓은 길
		 else if (ultrasonic_right_distance <= 20 && ultrasonic_center_distance <= 30 && ultrasonic_right_distance > 3 && ultrasonic_center_distance > 3)
		 { // 오른쪽이 3 초과 20 이하이고 중간이 3 초과 30 이하일 때 좌회전
			 run_state = TURN_LEFT;
			 current_mode = TURN_LEFT;
			 //turn_left(500);
			 current_speed = 500;
			 //sbuf[20] = "TRUN LEFT";
		 }
		 else if (ultrasonic_left_distance <= 20 && ultrasonic_center_distance <= 30 && ultrasonic_left_distance > 3 && ultrasonic_center_distance > 3)
		 { // 왼쪽이 3 초과 20 이하이고 중간이 3 초과 30 이하일 때 좌회전
			 run_state = TURN_RIGHT;
			 current_mode = TURN_RIGHT;
			 //turn_right(500);
			 current_speed = 500;
			 //sbuf[20] = "TRUN RIGHT";
		 }
		 ///
		 else if (ultrasonic_right_distance <= 20)
		 { // 오른쪽이 20 이하일 때 좌회전
			 run_state = TURN_LEFT;
			 current_mode = TURN_LEFT;
			 //turn_left(500);
			 current_speed = 500;
			 //sbuf[20] = "TRUN LEFT";
		 }
		 else if (ultrasonic_left_distance <= 20)
		 { // 왼쪽이 20 이하일 때 우회전
			 run_state = TURN_RIGHT;
			 current_mode = TURN_RIGHT;
			 //turn_right(500);
			 current_speed = 500;
			 //sbuf[20] = "TRUN RIGHT";
		 }
		 
		
		else if ((ultrasonic_left_distance - ultrasonic_right_distance <= 2 && ultrasonic_right_distance - ultrasonic_left_distance <= 2) || ultrasonic_center_distance >= 20 && ultrasonic_center_distance <= 200)
		{ // 좌 우 차이가 2cm 이하이거나 중간이 20cm 이상 200cm 이하일 때 전진
			run_state = FORWARD;
			current_mode = FORWARD;
			//forward(350);
			current_speed = 350;
			//sbuf[20] = "FORWARD";
		}
		else if(ultrasonic_right_distance - ultrasonic_left_distance <= 4 && ultrasonic_center_distance <= 7 && ultrasonic_right_distance > 5)
		{ // 좌 우 차이가 4cm 이하이고 중간이 7cm 이하이고 오른쪽이 5cm 초과일 때 좌회전
			run_state = TURN_LEFT;
			current_mode = TURN_LEFT;
			//turn_left(500);
			current_speed = 500;
			//sbuf[20] = "TRUN LEFT";
		}
		else if(ultrasonic_left_distance - ultrasonic_right_distance <= 4 && ultrasonic_center_distance <= 7 && ultrasonic_left_distance > 5)
		{ // 좌 우 차이가 4cm 이하이고 중간이 7cm 이하이고 왼쪽이 5cm 초과일 때 우회전
			run_state = TURN_RIGHT;
			current_mode = TURN_RIGHT;
			//turn_right(500);
			current_speed = 500;
			//sbuf[20] = "TRUN RIGHT";
		}
		
		current_location[run_state](&current_speed);
		I2C_LCD_Test(&current_speed, &current_mode);
	}
}
