#define F_CPU 16000000UL   // 16MHZ
#include <avr/io.h>   // PORTA PORTB 등의 IO관련 register들이 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h>   // for sei등
#include <string.h>

#include "def.h"
#include "uart1.h"
#include "led.h"

void init_uart1(void);
void led_main(void);
void UART1_transmit(uint8_t data);
void bt_command_processing(void);

extern void shift_left_ledon(void);
extern void shift_right_ledon(void);
extern void shift_left_keep_ledon(void);
extern void shift_right_keep_ledon(void);
extern void floweron(void);
extern void floweroff(void);
extern void led_all_on(void);
extern void led_all_off(void);

void (*fp_led1[])() =
{
	NULL,
	led_all_on, // 1
	led_all_off, // 2 
	floweron, // 3 
	floweroff, // 4
	shift_left_ledon, // 5
	shift_right_ledon,  // 6
	shift_left_keep_ledon, // 7
	shift_right_keep_ledon // 8
};


// BT로 부터 1 byte가 들어올때 마다 이곳으로 들어온다 (RX INT)
// 예) led_all_on\n --> 이곳 11번이 들어온다 
//     led_all_off\n
ISR(USART1_RX_vect)
{
	volatile static int i=0;
	volatile uint8_t data;
	
	data = UDR1;  // uart0의 H/W register(UDR1)로 부터 1 byte를 읽어 간다. 
	              // data = UDR1를 실행 하면 UDR1의 내용이 빈다(empty)
	if (data == '\r' || data == '\n')
	{
		rx1_buff[rear1][i] = '\0';   // \n --> \0(문장의 끝을 알리는 indicator)
		i=0;   // 다음 string을 저장 하기 위해 index값을 0으로 만든다. 
		rear1++;
		rear1 %= COMMAND_NUMBER;   // 0 ~ 9 if (rear1 >= 9) rear1 =0;
		// !!!! 이곳에 queue full (rx1_buff) 상태를 check하는 로직이 들어 가야 한다. !!!!!
	}
	else
	{
		// !!!!! COMMAND_LENGTH를 check 하는 로직 추가 !!!!
		rx1_buff[rear1][i++] = data;
	}
		  
}

/*
1. 전송속도: 9600bps : 총글자수 : 9600bit/10bits ==> 960자
  (1글자를 전송하는데 소요 되는시간 : 약 1ms)
2. 비동기방식(uart의 경우 clock신호에 의지 하지 않고(비동기) 별도의 부호
   비트(start/stop bit)로 데이터를 구분 하는 방식 
   data 8bit/none parity방식
3. RX(수신) : interrupt 방식 
*/
void init_uart1(void)
{
	// 1. 9600bps 설정
	UBRR1H = 0x00;
	UBRR1L = 207;   // 9600bps 표9-9
	// 2배속 통신
	UCSR1A |= 1 << U2X1; // 2배속 통신
	UCSR1C |= 0x06; // 비동기(ASYNC)/data8bits/none parity
	// RXEN1 : UART1으로 부터 수신이 가능 하도록
	// TXEN1 : UART1으로 부터 송신이 가능 하도록 한다. 
	// RXCIE1 : UART1로 부터 1byte가 들어 오면(stop bit가 들어오면) rx interrupt를 발생 시켜라 
    UCSR1B |= 1 << RXEN1 | 1 << TXEN1 | 1 << RXCIE1;
}

// UART1로 1byte를 전송 하는 함수
void UART1_transmit(uint8_t data)
{
	while( !(UCSR1A & 1 << UDRE1) )   // 데이터가 전송 중이면 전송이 끝날떄 까지 기다린다. 
		;   // no operation NOP
	
	UDR1 = data;   // HW전송 register(UDR1)에 data를 쏴준다.  
}

int state_led1 = 0;

void bt_command_processing(void)
{
	if (front1 != rear1) // data가 rx_buff에 존재한다. front와 rear 비교
	{
		printf("%s\n", rx1_buff[front1]); // rx_buff[front] ==> &rx_buff[front][0]
		
		/*for (int j = 1; j < 9; j++)
		{
			if (strncmp(rx_buff[front], str_led[j], strlen(str_led[j])) == 0)
			{
				state_led = j;
			}
		}*/
		
		if (strncmp(rx1_buff[front1], "led_all_on", strlen("led_all_on")) == 0)
		{
			state_led1 = 1;
		}
		else if (strncmp(rx1_buff[front1], "led_all_off", strlen("led_all_off")) == 0)
		{
			state_led1 = 2;
		}
		else if (strncmp(rx1_buff[front1], "floweron", strlen("floweron")) == 0)
		{
			state_led1 = 3;
		}
		else if (strncmp(rx1_buff[front1], "floweroff", strlen("floweroff")) == 0)
		{
			state_led1 = 4;
		}
		else if (strncmp(rx1_buff[front1], "shift_left_ledon", strlen("shift_left_ledon")) == 0)
		{
			state_led1 = 5;
		}
		else if (strncmp(rx1_buff[front1], "shift_right_ledon", strlen("shift_right_ledon")) == 0)
		{
			state_led1 = 6;
		}
		else if (strncmp(rx1_buff[front1], "shift_left_keep_ledon", strlen("shift_left_keep_ledon")) == 0)
		{
			state_led1 = 7;
		}
		else if (strncmp(rx1_buff[front1], "shift_right_keep_ledon", strlen("shift_right_keep_ledon")) == 0)
		{
			state_led1 = 8;
		}
		else
		{
			state_led1 = 0;
		}
		front1++;
		front1 %= COMMAND_NUMBER;
	}
	 if (state_led1 > 0 && state_led1 < 9)
	 {
		 (*fp_led1[state_led1])();
	 }
	// !!!! queue full check하는 logic이 들어가야 한다.
}