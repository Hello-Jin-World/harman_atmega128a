#include "led.h" //현재 directory에 들어있는 led.h를 불러온다.
#include "button.h"
extern void init_button(void);
extern int get_button(int button_num, int button_pin);


int led_main(void); // 선언

void shift_left_ledon();
void shift_right_ledon();
void shift_left_keep_ledon();
void shift_right_keep_ledon();
void floweron(void);
void floweroff(void);
void led_all_on(void);
void led_all_off(void);

extern volatile uint32_t msec_count;

int state = 0;

void (*fp[])() =
{
	shift_left_ledon, // 0
	shift_right_ledon, // 1
	shift_left_keep_ledon, // 2 
	shift_right_keep_ledon, // 3
	floweron, // 4
	floweroff // 5
};

int led_main(void) // 정의
{
	
	init_button();
	
	DDRA = 0xff;	//PORTA에 연결된 PIN 8개를 모두 output으로 설정. 출력 : 1
	
	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN))
		{
			state++;
			state %= 6; // 버튼을 눌렀을 때 상태처리
		}
		fp[state]();
	}
}

void shift_left_keep_ledon(void)
{
#if 1
	static int i = 0;

	if (msec_count >= 100)
	{
		msec_count = 0;
		if (i >= 8)
		{
			i = 0;
			PORTA = 0x00;
		}
		else
		{
			PORTA |= 0b00000001 << i++;
		}
	}
#else
	for (int i = 0; i < 8; i++)
	{
		PORTA += 0b00000001 << i;
	  //(i = 0)          1
	  //(i = 1)         1
	  //(i = 2)        1
		_delay_ms(50);
	}
#endif
}

void shift_right_keep_ledon(void)
{
#if 1
	static int i = 0;

	if (msec_count >= 100)
	{
		msec_count = 0;
		if (i >= 8)
		{
			i = 0;
			PORTA = 0x00;
		}
		else
		{
			PORTA |= 0b10000000 >> i++;
		}
	}
#else
	for (int i = 0; i < 8; i++)
	{
		PORTA -= 0b10000000 >> i;
		_delay_ms(50);
	}
#endif
}

void shift_left_ledon(void)
{
#if 1 // for문 제거, _delay_ms 제거
	static int i = 0;
	
	if (msec_count >= 100)
	{
		msec_count = 0;
		if (i >= 8)
		{
			i = 0;
			PORTA = 0x00;
		}
		else
		{
			PORTA = 0b00000001 << i++;
		}
	}
#else
	for (int i = 0; i < 8; i++)
	{
		PORTA = 0b00000001 << i;
		//(i = 0)          1
		//(i = 1)         1
		//(i = 2)        1
		_delay_ms(50);
	}
#endif
}

void shift_right_ledon(void)
{
#if 1
	static int i = 0;

	if (msec_count >= 100)
	{
		msec_count = 0;
		if (i >= 8)
		{
			i = 0;
			PORTA = 0x00;
		}
		else
		{
			PORTA = 0b10000000 >> i++;
		}
	}
#else
	for (int i = 1; i < 9; i++)
	{
		PORTA = 0b10000000 >> i;
		_delay_ms(50);
	}
#endif
}

void led_all_on_off(void)
{
	static int led_toggle = 0;
	
	if (msec_count >= 500) // if (mscount == 500) : interrupt에서는 == 주의
	{
		msec_count = 0; // 다음 500ms를 만나기 위해 0으로
		led_toggle = !led_toggle;
		if (led_toggle)
			led_all_on();
		else
			led_all_off();
	}
}

void led_all_on(void)
{
	PORTA = 0xff;
}

void led_all_off(void)
{
	PORTA = 0x00;
}

void floweron(void)
{

#if 1
	static int i = 0;

	if (msec_count >= 100)
	{
		msec_count = 0;
		if (i >= 4)
		{
			i = 0;
			PORTA = 0x00;
		}
		else
		{
			PORTA |= 0x10 << i | 0x08 >> i;
			i++;
		}
	}
#else
	int j, i;
	for (i = 0; i < 4; i++)
	{
		PORTA |= 0b00001000 >> i | 0b00010000 << i;
		_delay_ms(50);
	}
#endif
}

void floweroff(void)
{
	unsigned char h=0xf0;
	unsigned char l=0x0f;
#if 1
	static int i = 0;

	if (msec_count >= 100)
	{
		msec_count = 0;
		if (i >= 4)
		{
			i = 0;
			PORTA = 0x00;
		}
		else
		{
			PORTA = ( ((h >> i) & 0xf0) | ((l << i) & 0x0f));
			i++;
		}
	}
#else
	int j, i;
	for (i = 0; i < 4; i++)
	{
		PORTA = ( ((h >> i) & 0xf0) | ((l << i) & 0x0f));
		_delay_ms(300);
	}
#endif
}