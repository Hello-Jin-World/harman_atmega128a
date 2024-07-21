#include "led.h" //현재 directory에 들어있는 led.h를 불러온다.

extern volatile int led_shift_num;

int dim = 70; // LED 밝기 조절 변수
int direction = 1; // 1: 밝기 증가 모드, -1: 밝기 감소 모드

void make_pwm_led_control();
void led_on_pwm(int dim);

extern volatile uint32_t msec_count;


void make_pwm_led_control(int *led_pwm_count)
{
	DDRA = 0xff; // led 모두 출력 모드
	PORTA = *led_pwm_count; // 입력받은 카운트 만큼 LED on
	//PORTA = "0xff"; // led all on
	
	// dark --> bright --> dark 순서로 진행되는 코드
	
	led_on_pwm(dim); // 지속적으로 on할거면 이 함수만 while 반복
	dim += direction;
	
	if (dim == 255) // 다 밝아지면 다시 어두워지도록 함
	{
		direction = -1;
	}
	if (dim == 0) // 다 어두워지면 다시 밝아지도록 함
	{
		direction = 1;
	}
}

void led_on_pwm(int dim) // dim의 반복을 이용해서 밝기를 제어 (PWM의 HIGH 비율을 제어)
{
	for (int i = 0; i < 256; i++)
	{
		if (i > dim)
		{
			PORTA = 0; // duty cycle이 넘어가면 LED all off
		}
		_delay_us(20);
	}
}