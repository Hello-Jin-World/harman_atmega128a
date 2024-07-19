#include "fnd.h"
#include "button.h"
#define AUTO_WASH 0
#define FAST_WASH 1
#define RINSE_AND_SPINDRY 2
#define ONLY_SPINDRY 3
#define MAIN_SCREEN 4


void init_fnd(void);
void fnd_display(void);
int fnd_main(void);

// 버튼
extern void init_button(void);
extern int get_button(int button_num, int button_pin);

// 메인 화면에서 선택하는 함수들
void auto_wash();
void fast_wash();
void rinse_and_spindry();
void only_spindry();
void main_screen();

//자동 세척 모드에서 선탁하는 함수들
void water_temperature();
void rinse_frequency();
void spindry_strength();
void dumy_fanc();

uint32_t sec_count = 0; // 초를 재는 count 변수 unsigned int = uint32_t
volatile int led_shift_num = 0;
// uint32_t  = 0; 

extern volatile uint32_t fnd_refreshrate; // fnd 잔상효과를 유지하기 위한 변수 2ms
extern volatile uint32_t msec_count;
extern volatile uint32_t check_timer;

int select_wash_mode = 4; // 메인화면에서 모드 선택 변수
int auto_wash_mode = 3; // 자동 세탁 모드안에서 진행과정 선택 변수
int auto_wash_mode_toggle = 1; // 자동 세탁 모드 안에서 모든 과정을 마쳤는지 아는 토글 / 이게 0 되면 세탁을 시작함.
int total_wash_time = 30; // 총 세탁 시각 default : 30초
int spin_strength_val = 160; // 1단계 70, 2단계 115, 3단계 160, 4단계 205, 5단계 250

void (*fp_wash_mode[])() =
{
	auto_wash, // 0 자동 세탁
	fast_wash, // 1 쾌속 세탁
	rinse_and_spindry, // 2 헹굼 + 탈수
	only_spindry, // 3 탈수 단독
	main_screen // 4 Idle 메인화면
};

void (*auto_wash_select[])() =
{
	water_temperature,
	rinse_frequency,
	spindry_strength,
	dumy_fanc
};

int fnd_main(void)
{
	DDRA = 0xff;	//PORTA에 연결된 PIN 8개를 모두 output으로 설정. 출력 : 1
	init_fnd(); // fnd 초기화
	init_button(); // button 초기화
	

	int button1_state = 0;
	int button2_state = 0;
	int button3_state = 0;
	
	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN)) // 버튼 0을 받으면 자동 세탁 모드로 진입
		{
			select_wash_mode = AUTO_WASH;
		}
		
		if (get_button(BUTTON1, BUTTON1PIN)) // 버튼 1을 받으면 쾌속 세탁 모드로 진입
		{
			button1_state = !button1_state;
			
			if (button1_state) //
			{
				select_wash_mode = FAST_WASH;
			}
			else //
			{
				select_wash_mode = MAIN_SCREEN;
			}
		}
		
		if (get_button(BUTTON2, BUTTON2PIN)) // 버튼 2를 받으면 헹굼 + 탈수 모드로 진입
		{
			button2_state = !button2_state;\
			
			if (button2_state)
			{
				select_wash_mode = RINSE_AND_SPINDRY;
			}
			else
			{
				select_wash_mode = MAIN_SCREEN;	
			}
		}
		
		if (get_button(BUTTON3, BUTTON3PIN)) // 버튼 3을 받으면 탈수 단독 모드로 진입
		{
			button3_state = !button3_state;
			
			if (button3_state) //
			{
				select_wash_mode = ONLY_SPINDRY;
			}
			else //
			{
				select_wash_mode = MAIN_SCREEN;
			}
		}
		fp_wash_mode[select_wash_mode]();
		//auto_wash_led_on(&led_shift_num);
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}

////////////////////////////////////////////메인화면에서 동작하는 함수들////////////////////////////////////////////////

void auto_wash(void) // 자동 세탁
{
	PORTA = 0xff;
	sec_count = 1;
	
	int auto_step_led = 0;
	
	while (auto_step_led < 4)
	{
		if (get_button(BUTTON0, BUTTON0PIN))
		{
			select_wash_mode = 4;
			break;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			if (auto_step_led == 0) // 물 온도 선택하기  --- 물온도 변수 필요하고 
			{
				auto_wash_mode = 0;
			}
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
		auto_wash_select[auto_wash_mode]();
	}
	// 여기까지 자동 세탁의 물온도, 헹굼횟수, 탈수 강도를 선택함 밑에서 지정한 시간만큼 모터가 돌아가고 탈수 강도에 맞게 30초 동안 강도에 맞게 진행
	
	//여기에 시간 만큼 회전하는 함수 추가.
	
}

void fast_wash(void) // 쾌속 세탁
{
	sec_count = 2;
}

void rinse_and_spindry(void) // 헹굼 + 탈수
{
	sec_count = 3;
}

void only_spindry(void) // 탈수
{
	sec_count = 4;
}

void main_screen(void) // 메인 화면
{
	PORTA = 0;
	sec_count = 754; // 1234 표시 12 * 60 + 34
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////자동 세척 모드에서 동작하는 함수들////////////////////////////////////////////
void water_temperature() // 물 온도 선택
{
	PORTA = 0x07;
	int water_tem_toggle = 1;
	
	sec_count = 30;  // deflaut 물 온도
	
	while (water_tem_toggle)
	{
		if (get_button(BUTTON2, BUTTON2PIN))
		{
			sec_count -= 5;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			sec_count += 5;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{	
			sec_count = 0;
			auto_wash_mode = 1;
			water_tem_toggle = 0;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}

void rinse_frequency() // 헹굼 횟수 조절
{
	PORTA = 0x03;
	int rinse_frequency_toggle = 1; // 버튼1 누를 때 까지 반복
	
	sec_count = 5; // deflaut 헹굼 횟수
	
	while (rinse_frequency_toggle)
	{
		if (get_button(BUTTON2, BUTTON2PIN)) // 버튼 2 누르면 횟수 1 감소
		{
			sec_count -= 1;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN)) // 버튼 3 누르면 횟수 1 증가
		{
			sec_count += 1;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			total_wash_time += sec_count * 10; // 버튼 1누르면 횟수 * 10초를 전역변수에 저장하고 다음 단계로 간 다음 탈출
			sec_count = 0;
			auto_wash_mode = 2;
			rinse_frequency_toggle = 0;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}

void spindry_strength()
{
	PORTA = 0x01;
	int spindry_strength_toggle = 1; // 버튼1 누를 때 까지 반복
	
	sec_count = 3; // deflaut 탈수 강도
	
	while (spindry_strength_toggle)
	{
		if (get_button(BUTTON2, BUTTON2PIN)) // 버튼 2 누르면 횟수 1 감소
		{
			sec_count -= 1;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN)) // 버튼 3 누르면 횟수 1 증가
		{
			sec_count += 1;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			if(sec_count == 1)
			{
				spin_strength_val = 70;
			}
			else if (sec_count == 2)
			{
				spin_strength_val = 115;
			}
			else if (sec_count == 3)
			{
				spin_strength_val = 160;
			}
			else if (sec_count == 4)
			{
				spin_strength_val = 205;
			}
			else if (sec_count == 5)
			{
				spin_strength_val = 250;
			}
			PORTA = 0;
			sec_count = 0;
			auto_wash_mode_toggle = 0;
			spindry_strength_toggle = 0;
			
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}

void dumy_fanc()
{
	
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////fnd 초기화 및 fnd 표시 함수들 ///////////////////////////////////////////////////////////

void init_fnd(void)
{
	FND_DATA_DDR = 0xff; // 출력모드로 설정

	FND_DIGIT_DDR |= 1 << FND_DIGIT_D1 | 1 << FND_DIGIT_D2 |
					 1 << FND_DIGIT_D3 | 1 << FND_DIGIT_D4;

	FND_DATA_PORT = 0x00; // FND를 all off
}



void fnd_display(void)
{
						//  0      1       2      3      4      5      6     7      8       9      .
	uint8_t fnd_font[] = {~0xc0, ~0xf9, ~0xa4, ~0xb0, ~0x99, ~0x92, ~0x82, ~0xd8, ~0x80, ~0x90, ~0x7f};

	static int digit_select = 0; // 자리수 선택 변수 0~3   static : 전역변수처럼 작동

	switch(digit_select)
	{
		case 0 :
		FND_DIGIT_PORT = ~0x80;
		FND_DATA_PORT = fnd_font[sec_count % 10];   // 0~9초
		break;

		case 1 :
		FND_DIGIT_PORT = ~0x40;

		FND_DATA_PORT = fnd_font[sec_count / 10 % 6]; // 10단위 초
		break;

		case 2 :
		FND_DIGIT_PORT = ~0x20;
		if (sec_count % 2 == 1)
			FND_DATA_PORT = fnd_font[sec_count / 60 % 10] | fnd_font[10]; // 1단위 분
		else
			FND_DATA_PORT = fnd_font[sec_count / 60 % 10]; // 1단위 분
		break;

		case 3 :
		FND_DIGIT_PORT = ~0x10;
		FND_DATA_PORT = fnd_font[sec_count / 600 % 6]; // 10단위 분
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}



