﻿#define F_CPU 16000000UL   // 16MHZ
#include <avr/io.h>   // PORTA PORTB 등의 IO관련 register들이 들어 있다.
#include <util/delay.h>  // _delay_ms _delay_us
#include <avr/interrupt.h>   // for sei등
#include <string.h>

#include "def.h"
#include "uart0.h"
#include "led.h"

void init_uart0(void);
void led_main(void);
void UART0_transmit(uint8_t data);

extern void shift_left_ledon(void);
extern void shift_right_ledon(void);
extern void shift_left_keep_ledon(void);
extern void shift_right_keep_ledon(void);
extern void floweron(void);
extern void floweroff(void);
extern void led_all_on(void);
extern void led_all_off(void);

void (*fp_led[])() =
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

ISR(USART0_RX_vect)
{
	volatile static int i=0;
	volatile uint8_t data;
	
	data = UDR0;  // uart0의 H/W register(UDR0)로 부터 1 byte를 읽어 간다. 
	              // data = UDR0를 실행 하면 UDR0의 내용이 빈다(empty)
	if (data == '\r' || data == '\n')
	{
		rx_buff[rear][i] = '\0';   // \n --> \0(문장의 끝을 알리는 indicator)
		i=0;   // 다음 string을 저장 하기 위해 index값을 0으로 만든다. 
		rear++;
		rear %= COMMAND_NUMBER;   // 0 ~ 9 if (rear >= 9) rear =0;
		// !!!! 이곳에 queue full (rx_buff) 상태를 check하는 로직이 들어 가야 한다. !!!!!
	}
	else
	{
		// !!!!! COMMAND_LENGTH를 check 하는 로직 추가 !!!!
		rx_buff[rear][i++] = data;
	}
		  
}

void init_uart0(void)
{
	// 1. 9600bps 설정
	UBRR0H = 0x00;
	UBRR0L = 207;   // 9600bps 표9-9
	// 2배속 통신
	UCSR0A |= 1 << U2X0; // 2배속 통신
	UCSR0C |= 0x06; // 비동기(ASYNC)/data8bits/none parity
	// RXEN0 : UART0으로 부터 수신이 가능 하도록
	// TXEN0 : UART0으로 부터 송신이 가능 하도록 한다. 
	// RXCIE0 : UART0로 부터 1byte가 들어 오면(stop bit가 들어오면) rx interrupt를 발생 시켜라 
    UCSR0B |= 1 << RXEN0 | 1 << TXEN0 | 1 << RXCIE0;
}

// UART0로 1byte를 전송 하는 함수
void UART0_transmit(uint8_t data)
{
	while( !(UCSR0A & 1 << UDRE0) )   // 데이터가 전송 중이면 전송이 끝날떄 까지 기다린다. 
		;   // no operation NOP
	
	UDR0 = data;   // HW전송 register(UDR0)에 data를 쏴준다.  
}

int state_led = 0;

void pc_command_processing(void)
{
	if (front != rear) // data가 rx_buff에 존재한다. front와 rear 비교
	{
		printf("%s\n", rx_buff[front]); // rx_buff[front] ==> &rx_buff[front][0]
		
		if (strncmp(rx_buff[front], "led_all_on", strlen("led_all_on")) == 0)
		{
			state_led = 1;
		}
		else if (strncmp(rx_buff[front], "led_all_off", strlen("led_all_off")) == 0)
		{
			state_led = 2;
		}
		else if (strncmp(rx_buff[front], "floweron", strlen("floweron")) == 0)
		{
			state_led = 3;
		}
		else if (strncmp(rx_buff[front], "floweroff", strlen("floweroff")) == 0)
		{
			state_led = 4;
		}
		else if (strncmp(rx_buff[front], "shift_left_ledon", strlen("shift_left_ledon")) == 0)
		{
			state_led = 5;
		}
		else if (strncmp(rx_buff[front], "shift_right_ledon", strlen("shift_right_ledon")) == 0)
		{
			state_led = 6;
		}
		else if (strncmp(rx_buff[front], "shift_left_keep_ledon", strlen("shift_left_keep_ledon")) == 0)
		{
			state_led = 7;
		}
		else if (strncmp(rx_buff[front], "shift_right_keep_ledon", strlen("shift_right_keep_ledon")) == 0)
		{
			state_led = 8;
		}
		else
		{
			state_led = 0;
		}
		front++;
		front %= COMMAND_NUMBER;
	}
	 if (state_led > 0 && state_led < 9)
	 {
		 (*fp_led[state_led])();
	 }
	// !!!! queue full check하는 logic이 들어가야 한다.
}