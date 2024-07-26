/*
 * uart1.h
 *
 * Created: 2024-05-07 오후 3:40:03
 *  Author: kccistc
 */ 


#ifndef UART1_H_
#define UART1_H_

// led_all_on
volatile uint8_t rx1_buff[100];   // uart0로 부터 들어온 문자를 저장 하는 버퍼
volatile uint8_t rx1_ready_flag=0;  // 완전한 문장(\n)을 만나면 1로 set된다.

void UART1_transmit(uint8_t data);
void bt_command_processing(void);
void init_uart1(void);




#endif /* UART1_H_ */