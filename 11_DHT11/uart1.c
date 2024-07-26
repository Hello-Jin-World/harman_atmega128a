/*
 * uart1.c
 *
 * Created: 2024-05-07 오후 3:40:26
 *  Author: kccistc
 */ 

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
#include "uart1.h"
// 1byte를 수신 할때 마다 이곳으로 들어 온다 
// 입력패턴예: led_all_on\n
ISR(USART1_RX_vect)
{
	static int i=0;
	uint8_t data1;
	
	data1 = UDR1;  // uart1의 H/W register(UDR1)로 부터 1byte를 읽어 간다. 
	               // data = UDR1를 취하면 UDR1의 내용이 빈다.(move)
	if (data1 == '\r' || data1 == '\n')
	{
		rx1_buff[i] = '\0';   // 문장의 끝을 알리는  null을 insert
		i=0;   // 다음 string을 저장 하기 위해 index값을 0으로 만듬.
		rx1_ready_flag=1;   // 완전한 문장이 들어 온면 1로 set 
	}
	else
	{
		rx1_buff[i++] = data1; 		
	}
}
// 1.전송속도: 9600bps : 총byte(글자)수 : 9600/10 ==> 960자
//  (1글자 전송 하는데 소요 되는 시간 : 약 1ms)
// 2.비동기방식(uart의 경우 clock신호에 의지하지 않고 별도의 부호비트(start/stop)로 
//   data를 구분하는 방식) : data 8bit/none parity
// 3. RX(수신) : interrupt 활성화
void init_uart1(void)
{
	UBRR1H = 0x00;
	UBRR1L = 207;   // 9600bps 표9-9
	
	UCSR1A |= 1 << U2X1;   // 2배속 통신
	
	// RXEN1 : UART1로 부터 수신이 가능 하도록 설정
	// TXEN1 : UART1로 부터 송신 가능 하도록 설정
	// RXCIE1 : UART1로 부터 1byte가 들어 오면(stopbit까지 수신완료) rx interrupt를 발생시켜라
	UCSR1B |= 1 << RXEN1 | 1 << TXEN1 | 1 << RXCIE1;
}

// UART1롤 1byte를 전송 하는 함수 
void UART1_transmit(uint8_t data)
{
	while ( !(UCSR1A & 1 << UDRE1))  // 데이터가 전송중이면 전송이 끝날때 까지 기다린다. 
		;    // no operation   NOP
		
    UDR1 = data;   // HW전송 rtegister(UDR0)에 data를 쏴준다. 
}

// led_all_on\0
// led_all_off\0
void bt_command_processing(void)
{
	if (rx1_ready_flag)   // if (rx_ready_flag >= 1)  data가 \n까지 들어 왔으면
	{
		rx1_ready_flag=0;
		printf("%s\n", rx1_buff);
		if (strncmp(rx1_buff,"led_all_on",strlen("led_all_on")) == 0)
		{
			PORTA=0xff;
		}
		else if (strncmp(rx1_buff,"led_all_off",strlen("led_all_off")) == 0)
		{
			PORTA=0x00;
		}
		// shift_left2right_keep_ledon
		// shift_right2left_keep_ledon
		// flower_on
		// flower_off
	}
}