/*
 * ultrasonic.c
 *
 * Created: 2024-07-17 오전 9:33:26
 *  Author: kccistc
 */ 

#include "ultrasonic.h"
#include "def.h"

extern volatile uint32_t ultrasonic_check_timer;
extern volatile uint32_t msec_count;
extern int func_state; // pfunction을 찾아가는 인덱스

void distance_check();
void init_ultrasonic();

volatile int ultrasonic_left_distance = 0;
volatile int ultrasonic_center_distance = 0;
volatile int ultrasonic_right_distance = 0;

// PE4 : 외부 INT4 초음파 센서 상승, 하강 edge 둘 다 이곳으로 들어온다.
//결국은 상승edge에서 1번, 하강edge에서 1번씩 이곳으로 들어온다.
ISR(INT4_vect) // LEFT
{
   // 1. 상승 edge
   if (ECHO_PIN_LEFT & (1 << ECHO_LEFT))
   {
      TCNT3 = 0;
   }
   // 2. 하강 edge
   else
   {
      // ECHO 핀에 들어온 펄스 길이를 us로 환산
      ultrasonic_left_distance = (1000000.0 * TCNT3 * 1024 / F_CPU) / 58;
      
      // 예) TCNT1에 10이 들어 있다고 가정하자
      // 15.625khz의 1주기가 64us이다.
      // 64us * 10 == 640us
      // 640us / 58us (58us는 초음파 센서에서 1cm이동하는데 58us가 소요됨). ==> 11cm 
      // sprintf(scm, "dis : %dcm\n", ultrasonic_distance / 58); // sprintf는 버퍼에 찍는 것
   }
}

ISR(INT5_vect) // CENTER
{
   // 1. 상승 edge
   if (ECHO_PIN_CENTER & (1 << ECHO_CENTER))
   {
      TCNT3 = 0;
   }
   // 2. 하강 edge
   else
   {
      // ECHO 핀에 들어온 펄스 길이를 us로 환산
      ultrasonic_center_distance = (1000000.0 * TCNT3 * 1024 / F_CPU) / 58;
      
      // 예) TCNT1에 10이 들어 있다고 가정하자
      // 15.625khz의 1주기가 64us이다.
      // 64us * 10 == 640us
      // 640us / 58us (58us는 초음파 센서에서 1cm이동하는데 58us가 소요됨). ==> 11cm
      // sprintf(scm, "dis : %dcm\n", ultrasonic_distance / 58); // sprintf는 버퍼에 찍는 것
   }
}

ISR(INT6_vect) // RIGHT
{
   // 1. 상승 edge
   if (ECHO_PIN_RIGHT & (1 << ECHO_RIGHT))
   {
      TCNT3 = 0;
   }
   // 2. 하강 edge
   else
   {
      // ECHO 핀에 들어온 펄스 길이를 us로 환산
      ultrasonic_right_distance = (1000000.0 * TCNT3 * 1024 / F_CPU) / 58;
      
      // 예) TCNT1에 10이 들어 있다고 가정하자
      // 15.625khz의 1주기가 64us이다.
      // 64us * 10 == 640us
      // 640us / 58us (58us는 초음파 센서에서 1cm이동하는데 58us가 소요됨). ==> 11cm
      // sprintf(scm, "dis : %dcm\n", ultrasonic_distance / 58); // sprintf는 버퍼에 찍는 것
   }
}

void init_ultrasonic()
{
   //////////////// left ////////////////
   TRIG_DDR_LEFT |= 1 << TRIG_LEFT; // output mode로 설정
   ECHO_DDR_LEFT &= ~(1 << ECHO_LEFT); // input mode로 설정
   
   // 0 1 : 상승, 하강 edge에 둘 다 INT를 띄우도록 설정
   // INT0~3은 EICRA, INT4~7은 EICRB 레지스터
   EICRB |= 0 << ISC41 | 1 << ISC40; // INT4니까 EICRB에서 ISC41, 40
   
   // 16bit timer1을 설정해서 사용 16비트는 0~65535(0xffff)가 최대
   // 16Mhz를 1024로 분주 16000000/1024 --> 15625hz --> 15.625khz
   // 1주기 (1개의 펄스 소요시간) 1/15625 = 0.000064sec = 64us   
   TCCR3B |=  1 << CS32 | 1 << CS30; // 1024로 분주
   EIMSK |= 1 << INT4; // 외부 인터럽트4번 (ECHO) 사용
   
   
   //////////////// center ////////////////
   TRIG_DDR_CENTER |= 1 << TRIG_CENTER; // output mode로 설정
   ECHO_DDR_CENTER &= ~(1 << ECHO_CENTER); // input mode로 설정
   
   // 0 1 : 상승, 하강 edge에 둘 다 INT를 띄우도록 설정
   // INT0~3은 EICRA, INT4~7은 EICRB 레지스터
   EICRB |= 0 << ISC51 | 1 << ISC50; // INT5니까 EICRB에서 ISC51, 50
   
   // 16bit timer1을 설정해서 사용 16비트는 0~65535(0xffff)가 최대
   // 16Mhz를 1024로 분주 16000000/1024 --> 15625hz --> 15.625khz
   // 1주기 (1개의 펄스 소요시간) 1/15625 = 0.000064sec = 64us
   TCCR3B |=  1 << CS32 | 1 << CS30; // 1024로 분주
   EIMSK |= 1 << INT5; // 외부 인터럽트4번 (ECHO) 사용
   
   
   //////////////// right ////////////////
   TRIG_DDR_RIGHT |= 1 << TRIG_RIGHT; // output mode로 설정
   ECHO_DDR_RIGHT &= ~(1 << ECHO_RIGHT); // input mode로 설정
   
   // 0 1 : 상승, 하강 edge에 둘 다 INT를 띄우도록 설정
   // INT0~3은 EICRA, INT4~7은 EICRB 레지스터
   EICRB |= 0 << ISC61 | 1 << ISC60; // INT6니까 EICRB에서 ISC61, 60
   
   // 16bit timer1을 설정해서 사용 16비트는 0~65535(0xffff)가 최대
   // 16Mhz를 1024로 분주 16000000/1024 --> 15625hz --> 15.625khz
   // 1주기 (1개의 펄스 소요시간) 1/15625 = 0.000064sec = 64us
   TCCR3B |=  1 << CS32 | 1 << CS30; // 1024로 분주
   EIMSK |= 1 << INT6; // 외부 인터럽트4번 (ECHO) 사용
}

void ultrasonic_trigger()
{
      ////////// left //////////
      TRIG_PORT_LEFT &= ~(1 << TRIG_LEFT); // 해당되는 포트만 LOW로 만듦
      _delay_us(1);
      TRIG_PORT_LEFT |= 1 << TRIG_LEFT; // HIGH
      _delay_us(15); // 규격에는 10us인데 reduance
      TRIG_PORT_LEFT &= ~(1 << TRIG_LEFT); // LOW
      _delay_ms(50); // delay 기다리는 시간을 timer0 변수로 체크할 수 있도록 개선
      // 초음파센서 echo 응답 대기시간이 최대 38ms
      
      ////////// center //////////
      TRIG_PORT_CENTER &= ~(1 << TRIG_CENTER); // 해당되는 포트만 LOW로 만듦
      _delay_us(1);
      TRIG_PORT_CENTER |= 1 << TRIG_CENTER; // HIGH
      _delay_us(15); // 규격에는 10us인데 reduance
      TRIG_PORT_CENTER &= ~(1 << TRIG_CENTER); // LOW
      _delay_ms(50); // delay 기다리는 시간을 timer0 변수로 체크할 수 있도록 개선
      
      ////////// right //////////
      TRIG_PORT_RIGHT &= ~(1 << TRIG_RIGHT); // 해당되는 포트만 LOW로 만듦
      _delay_us(1);
      TRIG_PORT_RIGHT |= 1 << TRIG_RIGHT; // HIGH
      _delay_us(15); // 규격에는 10us인데 reduance
      TRIG_PORT_RIGHT &= ~(1 << TRIG_RIGHT); // LOW
      _delay_ms(50); // delay 기다리는 시간을 timer0 변수로 체크할 수 있도록 개선
}

void distance_check(void)
{
      printf("left   :  %5d\n", ultrasonic_left_distance); // printf 찍으면 delay생김
      printf("center :  %5d\n", ultrasonic_center_distance);
      printf("right  :  %5d\n", ultrasonic_right_distance);

   ultrasonic_trigger(); // 이것도 수정해야한다.
   func_state = AUTO_MODE_CHECK;
}