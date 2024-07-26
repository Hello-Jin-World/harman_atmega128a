/*
 * uart0.c
 *
 * Created: 2024-04-30 오전 10:38:28
 *  Author: kccistc
 */ 
#define F_CPU 16000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <string.h>    // strncmp, strcpy, strcmp 등이 들어 있다.
#include "def.h" 
#include "uart0.h"
// 1byte를 수신 할때 마다 이곳으로 들어 온다 
// 입력패턴예: led_all_on\n
ISR(USART0_RX_vect)
{
	volatile static int i=0;
	volatile uint8_t data;
	
	data = UDR0;   // uart0의 H/W register(UDR0)로 부터 1byte를 읽어 간다. 
	               // data = UDR0를 취하면 UDR0의 내용이 빈다.(move)
	if (data == '\r' || data == '\n')
	{
		rx_buff[rear][i] = '\0';   // 문장의 끝을 알리는  null을 insert
		i=0;   // 다음 string을 저장 하기 위해 index값을 0으로 만듬.
		rear++;
		rear %= COMMAND_NUMBER;
		// queue full 조건 check하는 로직이 들어 가야 한다. 
	}
	else
	{
		rx_buff[rear][i++] = data; 		
	}
}
// 1.전송속도: 9600bps : 총byte(글자)수 : 9600/10 ==> 960자
//  (1글자 전송 하는데 소요 되는 시간 : 약 1ms)
// 2.비동기방식(uart의 경우 clock신호에 의지하지 않고 별도의 부호비트(start/stop)로 
//   data를 구분하는 방식) : data 8bit/none parity
// 3. RX(수신) : interrupt 활성화
void init_uart0(void)
{
	UBRR0H = 0x00;
	UBRR0L = 207;   // 9600bps 표9-9
	
	UCSR0A |= 1 << U2X0;   // 2배속 통신
	UCSR0C |= 0x06;   // ASYNC(비동기)/data8bit/none parity
	
	// RXEN0 : UART0로 부터 수신이 가능 하도록 설정
	// TXEN0 : UART0로 부터 송신 가능 하도록 설정
	// RXCIE0 : UART0로 부터 1byte가 들어 오면(stopbit까지 수신완료) rx interrupt를 발생시켜라
	UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;
}

// UART0롤 1byte를 전송 하는 함수 
void UART0_transmit(uint8_t data)
{
	while ( !(UCSR0A & 1 << UDRE0))  // 데이터가 전송중이면 전송이 끝날때 까지 기다린다. 
		;    // no operation   NOP
		
    UDR0 = data;   // HW전송 rtegister(UDR0)에 data를 쏴준다. 
}

// led_all_on\0
// led_all_off\0
void pc_command_processing(void)
{
	if (front != rear)   //
	{
		printf("%s\n", rx_buff[front]);  // &rx_buff[front][0]
		if (strncmp(rx_buff[front],"led_all_on",strlen("led_all_on")) == 0)
		{
			PORTA=0xff;
		}
		else if (strncmp(rx_buff[front],"led_all_off",strlen("led_all_off")) == 0)
		{
			PORTA=0x00;
		}
		front++;
		front %= COMMAND_NUMBER;
		// queue full check  하는 로직이 들어 가야 한다. 
		// shift_left2right_keep_ledon
		// shift_right2left_keep_ledon
		// flower_on
		// flower_off
	}
}