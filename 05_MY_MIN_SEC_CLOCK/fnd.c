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

uint32_t sec_count = 0; // 초를 재는 count 변수 unsigned int = uint32_t

extern volatile uint32_t fnd_refreshrate; // fnd 잔상효과를 유지하기 위한 변수 2ms
extern volatile uint32_t msec_count;

int state = 0;
int temp = 0, temp1 = 0;

void (*fp[])() =
{
	display_clock, // 0
	stopwatch, // 1
	pause_stopwatch, // 2
	clear_stopwatch // 3
};

int fnd_main(void)
{
	DDRA = 0xff;
	int button0_state = 0;
	int button1_state = 0;
	int reset_active = 0;
	int restart_stopwatch = 0;
	
	init_fnd();
	init_button();
	
	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN))
		{
			button0_state = !button0_state;
			
			if (button0_state)
			{
				temp = sec_count;
				sec_count = 0;
				state = 1;
			}
			else
			{
				sec_count = temp;
				state = 0;
			}
		}
		
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			button1_state = !button1_state;
			
			if (button1_state)
			{
				state = 2;
				reset_active = 1;
			}
			if (button1_state == 0 || restart_stopwatch == 1)
			{
				state = 1;
				reset_active = 0;
				restart_stopwatch = 0;
			}
		}
		if (get_button(BUTTON2, BUTTON2PIN)) // 삭제
		{
			if (reset_active)
			{
				clear_stopwatch();
				restart_stopwatch = 1;
			}
		}
		
		fp[state]();
	}
}

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

void init_fnd(void)
{
	FND_DATA_DDR = 0xff; // 출력모드로 설정
	//FND_DIGIT_DDR |= 0xf0; //자리 수 선택 핀 4 5 6 7
	FND_DIGIT_DDR |= 1 << FND_DIGIT_D1 | 1 << FND_DIGIT_D2 |
					 1 << FND_DIGIT_D3 | 1 << FND_DIGIT_D4;

#if 0 // comm 애노우드
	FND_DATA_PORT = ~0x00; // FND를 all off  ~0x00 = 0xff
#else // comm 캐소우드
	FND_DATA_PORT = 0x00; // FND를 all off
#endif
}



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