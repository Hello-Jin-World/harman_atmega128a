
/*
 * led.c
 *
 * Created: 2024-07-11 오전 9:31:04
 *  Author: kccistc
 */ 

#include "led.h" //현재 directory에 들어있는 led.h를 불러온다.

int led_main(void); // 선언

void shift_left_ledon();
void shift_right_ledon();
void shift_left_keep_ledon();
void shift_right_keep_ledon();
void floweron(void);
void floweroff(void);
void led_all_on(void);
void led_all_off(void);

int led_main(void) // 정의
{
	DDRA = 0xff;	//PORTA에 연결된 PIN 8개를 모두 output으로 설정. 출력 : 1
	
	while(1)
	{
		floweron();
		floweroff();
	}
}

void shift_left_keep_ledon(void)
{
	for (int i = 0; i < 8; i++)
	{
		PORTA += 0b00000001 << i;
	  //(i = 0)          1
	  //(i = 1)         1
	  //(i = 2)        1
		_delay_ms(50);
	}
}

void shift_right_keep_ledon(void)
{
	for (int i = 0; i < 8; i++)
	{
		PORTA -= 0b10000000 >> i;
		_delay_ms(50);
	}
}

void shift_left_ledon(void)
{
	for (int i = 0; i < 8; i++)
	{
		PORTA = 0b00000001 << i;
		//(i = 0)          1
		//(i = 1)         1
		//(i = 2)        1
		_delay_ms(50);
	}
}

void shift_right_ledon(void)
{
	for (int i = 1; i < 9; i++)
	{
		PORTA = 0b10000000 >> i;
		_delay_ms(50);
	}
}

void led_all_on(void)
{
	PORTA = 0xff;
	_delay_ms(50);
}

void led_all_off(void)
{
	PORTA = 0x00;
	_delay_ms(50);
}

void floweron(void)
{
	int j, i;
	for (i = 0; i < 4; i++)
	{
		for (j = i; j < i + 1; j++)
		{
			PORTA += 0b00001000 >> i;
		}
		PORTA += 0b00010000 << i;
		_delay_ms(50);
	}
	
}

void floweroff(void)
{
	int j, i;
	for (i = 0; i < 4; i++)
	{
		for (j = i; j < i + 1; j++)
		{
			PORTA -= 0b00000001 << i;
		}
		PORTA -= 0b10000000 >> i;
		_delay_ms(50);
	}
}