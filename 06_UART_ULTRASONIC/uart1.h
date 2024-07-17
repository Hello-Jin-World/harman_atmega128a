/*
 * uart1.h
 *
 * Created: 2024-07-16 오전 10:47:17
 *  Author: kccistc
 */ 


#ifndef UART1_H_
#define UART1_H_

volatile uint8_t rx1_buff[10][80]; // uart0로 부터 들어온 문자
volatile int rear1 = 0; // input index : USART0_RX_vect에 집어넣는 index
volatile int front1 = 0; // output index



#endif /* UART1_H_ */