/*
 * 01_LED_CONTROL.c
 *
 * Created: 2024-07-10 오후 3:17:14
 * Author : kccistc
 */ 
#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>
#include "button.h"

extern int led_main(void); // extern : led_main함수는 다른 파일에 있는 함수이다.라고 컴파일러한테 신고하는 것
extern void init_button(void);
extern int get_button(int button_num, int button_pin);
extern void led_all_on(void);
extern void led_all_off(void);

int main(void)
{	
	int button0_state = 0;
	init_button();
	//led_main();
	DDRA = 0xff; // PORTA를 출력으로 설정
	PORTA = 0x00; // led를 all off
	
	while(1)
	{
		// -- 1버튼 처리 (토글)
		// button0을 1번 눌렀다 떼면 led를 all on
		// button1을 1번 눌렀다 떼면 led를 all off
		if (get_button(BUTTON0, BUTTON0PIN))
		{
			button0_state = !button0_state;
			if (button0_state)
			{
				led_all_on();
			}
			else
				led_all_off();
		}
	}
}

#if 0
int main(void)
{
    DDRA = 0b11111111; // portA가에 LED가 8개 연결되어 있으므로 all 1
					   // DDR(Data Direction Register) : 방향 설정
					   // register : 1: 출력, 0: 입력
	//DDRD = 0b00000000;
	
	//unsigned char led[] = {0b00000001, 0b00000010, 0b00000100, 0b00001000, 0b00010000, 0b00100000, 0b01000000, 0b10000000};
	
	while(1)
	{
		/*if((PIND&0x10)==0){
			PORTA=0x01;
			PORTA=0x10;
		}
		if((PIND&0x20)==0) {
			PORTA=0x02;
			PORTA=0x20;
		}
		if((PIND&0x40)==0) {
			PORTA=0x04;
			PORTA=0x40;
		}
		if((PIND&0x80)==0) {
			PORTA=0x08;
			PORTA=0x80;
		}
	}*/
		for (int i = 0; i < 8; i++)
		{
			PORTA = 0x01 << i;
			_delay_ms(300);
		}
		for (int i = 1; i < 7; i++)
		{
			PORTA = 0x80 >> i;
			_delay_ms(300);
		}
	}
}
#endif

