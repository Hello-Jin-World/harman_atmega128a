#include "fnd.h"
#include "button.h"
#include "def.h"
void init_fnd(void);
void fnd_display();
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

void init_fnd(void)
{
	FND_DATA_DDR = 0xff;
	//FND_DIGIT_DDR |= 0xf0; //자리 수 선택 핀 4 5 6 7
	FND_DIGIT_DDR |= 1 << FND_DIGIT_D1 | 1 << FND_DIGIT_D2 |
					 1 << FND_DIGIT_D3 | 1 << FND_DIGIT_D4;


	FND_DATA_PORT = 0x00; // FND를 all off
}



void fnd_display(int *run_state)
{
						  // 0      1      2      3      4      5      6      7      8      9      .
	uint8_t fnd_font[] = {~0xc0, ~0xf9, ~0xa4, ~0xb0, ~0x99, ~0x92, ~0x82, ~0xd8, ~0x80, ~0x90, ~0x7f
		, ~0x8e, ~0x83, ~0xc7, ~0xaf
		};

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
		
		FND_DATA_PORT = fnd_font[*run_state+11]; // 10단위 분
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}



