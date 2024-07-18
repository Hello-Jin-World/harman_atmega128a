#include "fnd.h"
#include "button.h"
void init_fnd(void);
void fnd_display(void);
int fnd_main(void);
extern void init_button(void);
extern int get_button(int button_num, int button_pin);

void display_clock();
void stopwatch();
void pause_stopwatch();
void clear_stopwatch();
void final_fan_display();

uint32_t sec_count = 0; // 초를 재는 count 변수 unsigned int = uint32_t
uint32_t min_count = 0; // 분을 재는 count 변수

extern volatile uint32_t fnd_refreshrate; // fnd 잔상효과를 유지하기 위한 변수 2ms
extern volatile uint32_t msec_count;

int temp1 = 0;

void (*fp_clock[])() =
{
	display_clock, // 0 시계 모드
	stopwatch, // 1 스톱워치 모드
	pause_stopwatch, // 2 스톱워치 일시정지
	clear_stopwatch // 3 스톱워치 리셋
};

int fnd_main(void)
{
	int temp = 0;
	int state_mod = 0; // 모드 선택 변수
	
	DDRA = 0xff;
	int button0_state = 0; // 버튼0번 상태 변수
	int button1_state = 0; // 버튼1번 상태 변수
	int reset_active = 0; // 리셋이 가능한지 알려주는 토글
	int restart_stopwatch = 0; // 재시작이 가능한지 알려주는 토글
	
	init_fnd(); // fnd 초기화
	init_button(); // button 초기화
	
	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN))
		{
			button0_state = !button0_state;
			
			if (button0_state) // 버튼 0을 처음 누르면 지나간 시간을 임시 변수에 저장하고 시간 리셋 후 스톱워치로 진입
			{
				button1_state = 0;
				temp = sec_count;
				sec_count = 0;
				state_mod = 1;
			}
			else // 다시 버튼0을 누르면 임시 변수에 저장된 시간을 가져오고 시계로 진입
			{
				sec_count = temp;
				state_mod = 0;
			}
		}
		
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			button1_state = !button1_state;
			
			if (button1_state) // 버튼 1을 처음 누르면 일시정지 모드로 진입하고 리셋을 할 수 있도록 토글 활성화
			{
				state_mod = 2;
				reset_active = 1;
			}
			if (button1_state == 0 || restart_stopwatch == 1) // 다음 버튼 1을 누르면 스톱워치 재시작 후 리셋 토글 비활성화.
			{
				state_mod = 1;
				reset_active = 0;
				restart_stopwatch = 0;
			}
		}
		if (get_button(BUTTON2, BUTTON2PIN)) // 버튼 2를 누르면 리셋 토글이 활성화가 되었는지 확인 후 리셋
		{
			if (reset_active)
			{
				clear_stopwatch();
				restart_stopwatch = 1; // 재시작이 가능하도록 재시작 토글 활성화
			}
		}
		
		fp_clock[state_mod]();
	}
}
/*
void display_clock(void)
{
	if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
	{
		fnd_refreshrate = 0;
		fnd_display();
	}
	if (msec_count >= 1000)
	{
		msec_count = 0;
		sec_count++;
	}
}

void stopwatch(void)
{
	if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
	{
		fnd_refreshrate = 0;
		fnd_display();
	}
	if (msec_count >= 16)
	{
		msec_count = 0;
		sec_count++;
	}
}

void pause_stopwatch(void)
{
	if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
	{
		fnd_refreshrate = 0;
		fnd_display();
	}
	temp1 = sec_count;	
}

void clear_stopwatch(void)
{
	sec_count = 0;
	if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
	{
		fnd_refreshrate = 0;
		fnd_display();
	}
}
*/
void init_fnd(void)
{
	FND_DATA_DDR = 0xff; // 출력모드로 설정
	//FND_DIGIT_DDR |= 0xf0; //자리 수 선택 핀 4 5 6 7
	FND_DIGIT_DDR |= 1 << FND_DIGIT_D1 | 1 << FND_DIGIT_D2 |
					 1 << FND_DIGIT_D3 | 1 << FND_DIGIT_D4;

	FND_DATA_PORT = 0x00; // FND를 all off
}

/*

void fnd_display(void)
{
#if 0 // common anode
						 // 0     1     2     3     4     5     6     7     8     9     .
	uint8_t fnd_font[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90, 0x7f};
#else // common cathod
						  // 0      1      2      3      4      5      6      7      8      9      .
	uint8_t fnd_font[] = {~0xc0, ~0xf9, ~0xa4, ~0xb0, ~0x99, ~0x92, ~0x82, ~0xd8, ~0x80, ~0x90, ~0x7f};
#endif

	static int digit_select = 0; // 자리수 선택 변수 0~3   static : 전역변수처럼 작동

	switch(digit_select)
	{
		case 0 :
#if 0 // common anode
			FND_DIGIT_PORT = 0x80;
#else // common cathod
			FND_DIGIT_PORT = ~0x80;
#endif
			FND_DATA_PORT = fnd_font[sec_count % 10];   // 0~9초
		break;

		case 1 :
#if 0 // common anode
		FND_DIGIT_PORT = 0x40;
#else // common cathod
		FND_DIGIT_PORT = ~0x40;
#endif
		FND_DATA_PORT = fnd_font[sec_count / 10 % 6]; // 10단위 초
		break;

		case 2 :
#if 0 // common anode
		FND_DIGIT_PORT = 0x20;
		if (sec_count % 2 == 1)
			FND_DATA_PORT = fnd_font[sec_count / 60 % 10] & fnd_font[10]; // 1단위 분
		else
			FND_DATA_PORT = fnd_font[sec_count / 60 % 10]; // 1단위 분
		break;
#else // common cathod
		FND_DIGIT_PORT = ~0x20;
		if (sec_count % 2 == 1)
			FND_DATA_PORT = fnd_font[sec_count / 60 % 10] | fnd_font[10]; // 1단위 분
		else
			FND_DATA_PORT = fnd_font[sec_count / 60 % 10]; // 1단위 분
		break;
#endif

		case 3 :
#if 0 // common anode
		FND_DIGIT_PORT = 0x10;
#else // common cathod
		FND_DIGIT_PORT = ~0x10;
#endif
		FND_DATA_PORT = fnd_font[sec_count / 600 % 6]; // 10단위 분
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}
*/

//////////선풍기 시각 설정 함수//////////////
void fan_time_fnd_display(void)
{
						  // 0      1      2      3      4      5      6      7      8      9      .
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
		FND_DATA_PORT = fnd_font[min_count % 10] | fnd_font[10]; // 1단위 분
		break;

		case 3 :
		FND_DIGIT_PORT = ~0x10;
		FND_DATA_PORT = fnd_font[min_count / 10 % 6]; // 10단위 분
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}
