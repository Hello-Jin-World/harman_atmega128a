﻿#include "fnd.h"
#include "button.h"
#define CUSTOM_WASH 0
#define FAST_WASH 1
#define RINSE_AND_SPINDRY 2
#define ONLY_SPINDRY 3
#define MAIN_SCREEN 4


void init_fnd(void);
void fnd_display(void); // 숫자 표시
void fnd_loading_display(); // 로딩, 모드 표시
int fnd_main(void);

// 버튼
extern void init_button(void);
extern int get_button(int button_num, int button_pin);
// 모터
extern void washing_machine_fan_control(int *spin_strength);
// LED
extern void make_pwm_led_control();

// 메인 화면에서 선택하는 함수들
void custom_wash();
void fast_wash();
void rinse_and_spindry();
void only_spindry();
void main_screen();
void end_screen();

// 수동 세탁 모드에서 선탁하는 함수들
void water_temperature();
void rinse_frequency();
void spindry_strength();
void custom_wash_start();
void end_display();
void dumy_fanc();

uint32_t sec_count = 0; // 초를 재는 count 변수 unsigned int = uint32_t
volatile int led_shift_num = 0;

extern volatile uint32_t fnd_refreshrate; // fnd 잔상효과를 유지하기 위한 변수 2ms
extern volatile uint32_t msec_count;
extern volatile uint32_t check_timer; // 모터 회전 방향 반대로 하기위한 시각 체크 변수
extern volatile uint32_t loading_clock_change;

int select_wash_mode = 4; // 메인화면에서 모드 선택 변수
int custom_wash_mode = 5; // 커스텀 세탁 모드안에서 진행과정 선택 변수
int custom_wash_mode_toggle = 1; // 커스텀 세탁 모드 안에서 모든 과정을 마쳤는지 아는 토글 / 이게 0 되면 세탁을 시작함.
int total_wash_time = 90; // 총 세탁 시각 default : 60초 + default 탈수 시간 30초
int spin_strength_val = 0; // 1단계 115, 2단계 160, 3단계 205, 4단계 250
int loading_rot = 4; // 로딩  돌아가는거 보여주는 변수
int loading_clock_change_val = 1; // 시간이랑 로딩 창 번갈아 가는거 변수

void (*fp_wash_mode[])() =
{
	custom_wash, // 0 수동 세탁
	fast_wash, // 1 쾌속 세탁
	rinse_and_spindry, // 2 헹굼 + 탈수
	only_spindry, // 3 탈수 단독
	main_screen // 4 Idle 메인화면
};

void (*custom_wash_select[])() =
{
	water_temperature, // 물 온도 선택
	rinse_frequency, // 헹굼 횟수 선택
	spindry_strength, // 탈수 강도 선택
	custom_wash_start, // 세탁 시작
	end_display, // end 화면
	dumy_fanc // dumy
};

int fnd_main(void)
{
	DDRA = 0xff;	//PORTA에 연결된 PIN 8개를 모두 output으로 설정. 출력 : 1
	init_fnd(); // fnd 초기화
	init_button(); // button 초기화
	
	int main_ment_on = 0; // select mode 글씨가 fnd에 돌아가면서 표시되도록 하는 변수

	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN)) // 버튼 0을 받으면 수동 세탁 모드로 진입
		{
			select_wash_mode = CUSTOM_WASH;
		}
		
		if (get_button(BUTTON1, BUTTON1PIN)) // 버튼 1을 받으면 쾌속 세탁 모드로 진입
		{
			select_wash_mode = FAST_WASH;
		}
		
		if (get_button(BUTTON2, BUTTON2PIN)) // 버튼 2를 받으면 헹굼 + 탈수 모드로 진입
		{
			select_wash_mode = RINSE_AND_SPINDRY;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN)) // 버튼 3을 받으면 탈수 단독 모드로 진입
		{
			select_wash_mode = ONLY_SPINDRY;
		}
		
		fp_wash_mode[select_wash_mode]();
		
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			//fnd_display();
			fnd_loading_display(10,&main_ment_on);
		}
		if (msec_count >= 400)
		{
			msec_count = 0;
			main_ment_on++;
			main_ment_on %= 13;
		}
	}
}

////////////////////////////////////////////메인화면에서 동작하는 함수들////////////////////////////////////////////////

void custom_wash(void) // 수동 세탁
{

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
				custom_wash_mode = 0;
			}
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
		custom_wash_select[custom_wash_mode]();
	}
	// 여기까지 수동 세탁의 물온도, 헹굼횟수, 탈수 강도를 선택함 밑에서 지정한 시간만큼 모터가 돌아가고 탈수 강도에 맞게 30초 동안 강도에 맞게 진행
	
	//여기에 시간 만큼 회전하는 함수 추가.
	
}

void fast_wash(void) // 쾌속 세탁
{
	sec_count = 2;
	while (1)
	{
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			select_wash_mode = 4;
			break;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}

void rinse_and_spindry(void) // 헹굼 + 탈수
{
	sec_count = 3;
	while (1)
	{
		if (get_button(BUTTON2, BUTTON2PIN))
		{
			select_wash_mode = 4;
			break;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}

void only_spindry(void) // 탈수
{
	sec_count = 4;
	while (1)
	{
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			select_wash_mode = 4;
			break;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}

void main_screen(void) // 메인 화면
{
	PORTA = 0;
	loading_rot = 5; // NULL
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////수동 세척 모드에서 동작하는 함수들////////////////////////////////////////////
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
			custom_wash_mode = 1;
			water_tem_toggle = 0;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
		if (custom_wash_mode == 5)
		{
			break;
		}
	}
}

void rinse_frequency() // 헹굼 횟수 조절
{
	PORTA = 0x03;
	int rinse_frequency_toggle = 1; // 버튼1 누를 때 까지 반복
	
	sec_count = 2; // deflaut 헹굼 횟수
	
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
			custom_wash_mode = 2;
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
	
	sec_count = 3; // deflaut 탈수 강도 최대 4까지
	
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
		if (get_button(BUTTON1, BUTTON1PIN)) // 버튼 1 누르면 세팅한 강도에 맞는 회전값 반환
		{
			if(sec_count == 1)
			{
				spin_strength_val = 120;
			}
			else if (sec_count == 2)
			{
				spin_strength_val = 160;
			}
			else if (sec_count == 3)
			{
				spin_strength_val = 205;
			}
			else if (sec_count == 4)
			{
				spin_strength_val = 250;
			}
			PORTA = 0;
			spindry_strength_toggle = 0;
			custom_wash_mode = 3;
			sec_count = 0;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}


void custom_wash_start(void)
{
	int led_pwm_count; // LED 몇개 킬건지 정하는 변수
	
	sec_count = total_wash_time;
	
	while (sec_count > 0)
	{
		washing_machine_fan_control(&spin_strength_val);
		
		if (msec_count >= 250) // 1초마다 시간 1초씩 감소하고 로딩 회전이 됨.
		{
			msec_count = 0;
			sec_count--;
			loading_rot++;
			loading_rot %= 3;
		}
		if (loading_clock_change >= 3000) // 3초마다 로딩창이랑 시간 화면 뜨는거 토글
		{
			loading_clock_change = 0;
			loading_clock_change_val = !loading_clock_change_val;
		}
		
		/////여기에 일반세탁 60초 + 헹굼 횟수 * 10초 + 탈수 30초로 구분을 해야함
		if (total_wash_time - sec_count <= 30) // 일반 세탁 30초 경과
		{
			led_pwm_count = 255; // 0xff
		}
		
		else if (total_wash_time - sec_count <= 60 && total_wash_time - sec_count > 30) // 일반세탁 60초 완료
		{
			led_pwm_count = 126; // 0x7c
		}
		
		else if (total_wash_time - sec_count > 60 && sec_count >= 30) // 헹굼
		{
			led_pwm_count = 60; // 0x3c
		}
		
		else if (sec_count < 30)
		{
			led_pwm_count = 24; // 0x18
		}
		
		else if (sec_count == 0)
		{
			led_pwm_count = 0;
		}
		
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			make_pwm_led_control(&led_pwm_count);
			if (loading_clock_change_val)
			{
				fnd_display(); // 시간 보여주기
			}
			else
			{
				fnd_loading_display(&loading_rot,10); // 로딩 보여주기
			}
		}
	}
	OCR3C = 0;
	custom_wash_mode = 4; // end 화면으로 이동
	sec_count = 0; // 다 끝나면 끝
}

void end_display() // end 화면
{
	//더미 함수
	int end_toggle = 1;
	PORTA = 0;
	loading_rot = 4; // end fnd display
	while (end_toggle)
	{
		if (get_button(BUTTON0, BUTTON0PIN) || get_button(BUTTON1, BUTTON1PIN) || get_button(BUTTON2, BUTTON2PIN) || get_button(BUTTON3, BUTTON3PIN)) // 아무 버튼 누르면 다시 메인 화면으로
		{
			select_wash_mode = MAIN_SCREEN;
			end_toggle = !end_toggle;
			custom_wash_mode = 5; // 끝
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_loading_display(&loading_rot,10); // end 보여주기
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
			FND_DATA_PORT = fnd_font[sec_count / 60 % 10] | fnd_font[10]; // 1단위 분
		break;

		case 3 :
		FND_DIGIT_PORT = ~0x10;
		FND_DATA_PORT = fnd_font[sec_count / 600 % 6]; // 10단위 분
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}

void fnd_loading_display(int *loading_rot, int *main_ment) // 진행 로딩 상황 표시 
{
	int a, b, c, d;
				                // s     e        l      e     c     t               m     o     d        e
	uint8_t fnd_loading_font[] = {~0x92, ~0x86, ~0xc7, ~0x86, ~0xc6, ~0x87, ~0xff, ~0xaa, ~0xa3, ~0xa1, ~0x86, ~0xff, ~0xff,
		 ~0xce, ~0xf8, ~0xc7, ~0xf1, ~0xfe, ~0xf7, ~0xf6, ~0xff , ~0xab};
		//  |-     -|    |_     _|     -      _     -,_    꺼짐      n
	if (*loading_rot == 0)
	{
		d = 13; b = 18, c = 17, a = 16;	// loading 1
	}
	else if (*loading_rot == 1)
	{
		d = 15; b = 17, c = 18, a = 14; // loading 2
	}
	else if (*loading_rot == 2)
	{
		d = 17; b = 19, c = 19, a = 18; // loaing 3
	}
	else if (*loading_rot == 4) // end screen
	{
		a = 9; b = 21; c = 1; d = 6;
	}
	
	else // main screen
	{
		d = *main_ment;
		c = d + 1; c %= 13;
		b = c + 1; b %= 13;
		a = b + 1; a %= 13;
	}

	static int digit_select = 0; // 자리수 선택 변수 0~3   static : 전역변수처럼 작동

	switch(digit_select)
	{
		case 0 :
		FND_DIGIT_PORT = ~0x80;
		FND_DATA_PORT = fnd_loading_font[a];
		break;

		case 1 :
		FND_DIGIT_PORT = ~0x40;
		FND_DATA_PORT = fnd_loading_font[b];
		break;

		case 2 :
		FND_DIGIT_PORT = ~0x20;
		FND_DATA_PORT = fnd_loading_font[c];
		break;

		case 3 :
		FND_DIGIT_PORT = ~0x10;
		FND_DATA_PORT = fnd_loading_font[d];
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}






