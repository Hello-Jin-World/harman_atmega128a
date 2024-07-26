#include "button.h"
#include "def.h"
#include "led.h"

void init_button(void);
void auto_mode_check(void);
int get_button(int button_num, int button_pin);

extern int func_state; // pfunction을 찾아가는 인덱스
extern void stop();
extern void auto_start(); // 자율주행 시작

int button0_state = 0;


void init_button(void)
{
	BUTTON_DDR &= ~(1 << BUTTON0PIN /*| 1 << BUTTON1PIN | 1 << BUTTON2PIN | 1 << BUTTON3PIN*/);
}

int get_button(int button_num, int button_pin)
{
	int current_state;

	static unsigned char button_status[BUTTON_NUMBER] =
	{BUTTON_RELEASE/*, BUTTON_RELEASE, BUTTON_RELEASE, BUTTON_RELEASE*/}; // 지역변수에 static을 선언하면 전역변수처럼 활용가능
	//
	
	current_state = BUTTON_PIN & (1 << button_pin); // 버튼을 읽는다.
	if (current_state && button_status[button_num] == BUTTON_RELEASE) // 버튼이 처음 눌려진 상태.
	{
		_delay_ms(60); // noise가 지나가기를 기다린다.
		button_status[button_num] = BUTTON_PRESS; // 처음 눌려진 상태가 아니다
		return 0; // 아직은 완전히 눌렀다 땐 상태가 아니다.
		
	}
	else if (button_status[button_num] == BUTTON_PRESS && current_state == BUTTON_RELEASE)
	{	// 버튼이 이전에 눌려진 상태였으며 현재는 떼어진 상태이면
		button_status[button_num] = BUTTON_RELEASE; // 다음 버튼을 체크 하기위해서 초기화
		_delay_ms(60); // 뗀 상태의 noise가 지나가기를 기다린다.
		return 1; // 완전히 1번 눌렀다 땐 상태로 인정
	}
	
	return 0; // 버튼이 open 상태
}

void auto_mode_check(void)
{
	if (get_button(BUTTON0, BUTTON0PIN))
	{
		button0_state = !button0_state;
	}
	
	if (button0_state)
	{
		AUTO_RUN_LED_PORT |= 1 << AUTO_RUN_LED_PIN; // LED ON
		func_state = AUTO_MODE;
	}
	else
	{
		AUTO_RUN_LED_PORT &= ~(1 << AUTO_RUN_LED_PIN); // LED OFF
		stop();
	}
}