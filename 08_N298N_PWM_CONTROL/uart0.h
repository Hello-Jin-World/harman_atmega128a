#ifndef INCFILE1_H_
#define INCFILE1_H_

// led_all_on\n
// led_all_off\n
// led_left_shift_on\n
// led_right_shift_on\n
// 을 2차원 배열 circular queue에 저장
volatile uint8_t rx_buff[10][80]; // uart0로 부터 들어온 문자
volatile int rear = 0; // input index : USART0_RX_vect에 집어넣는 index
volatile int front = 0; // output index

#endif /* INCFILE1_H_ */