/*
 * extern.h
 *
 * Created: 2024-04-29 오전 11:27:46
 *  Author: kccistc
 */ 


#ifndef EXTERN_H_
#define EXTERN_H_

extern void shift_left2right_keep_ledon(int *pjob);
extern void shift_right2left_keep_ledon(int *pjob);
extern void flower_on(int *pjob);
extern void flower_off(int *pjob);

extern int get_button(int button_pin, int button_num);
extern void led_all_on_off();

extern void init_uart0(void);
extern void UART0_transmit(uint8_t data);
extern void pc_command_processing(void);

extern void bt_command_processing(void);
extern void init_uart1(void);

extern void init_ultrasonic();
extern void ultrasonic_trigger();
extern void ultrasonic_distance_check();

extern void I2C_LCD_init(void);
extern void I2C_LCD_Test();
extern void make_pwm_led_control(void);

extern volatile uint32_t shift_timer; 
extern volatile uint32_t ms_count;   // ms count
extern volatile uint32_t ultrasonic_trigger_timer;   // trigger를 동작시키는 주기 1000ms

extern char scm[50];
#endif /* EXTERN_H_ */