// Timer : 일정 시간이 되었느냐?
// Counter : 펄스의 개수를 센다.

#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> // sei 등

extern int led_main(void);

volatile uint32_t msec_count = 0; // 인터럽트 서비스 루틴에서 쓰는 변수 type앞에는 volatile이라고 선언
							   // 이는 최적화를 방지하기 위함이다
//------최적화 예제------
// 1) int i;
// i = 1;
// i = 2;
// i = 3;
// compiler가 i=1; i=2;는 생략하고 i=3;만 실행하는 경우가 발생한다.
// 2) 1~10까지의 합을 구한다.
// (1) sum = 0; i = 0;
// (2) while (i <= 10)
// (3) {
// (4)  	i++; sum += i;
// (5) }
// (6) printf("sum ==> %d\n", sum);
// compiler가 (1)~(5)까지는 실행을 안 하고 (6)에서 1~10까지의 합인 55를 출력하는 경우가 발생한다.
// volatile을 선언하면 하나부터 쭉 다 실행함.

// HW 가 SW 한테 interrput가 발생했다는 것을 알려주는 공간
// TIMER0_OVF_vect : interrupt 신호 이름
// 250개의 pulse를 count(1ms)하면 이곳으로 자동으로 들어온다.
// ISR은 가능한 최대한으로 짧게 작성한다.
ISR(TIMER0_OVF_vect) // interrupt service routine :routine은 interrupt 서비스 함수
{
	TCNT0 = 6; // 6 ~ 256 : 250개 (1ms) 그래서 TCNT0를 6으로 설정
	msec_count++; // 1ms마다 msec_count가 1씩 증가
}

int main(void)
{
	init_timer0();
	DDRA = 0xff; // led 출력 모드
	PORTA = 0x00; // led all off
	
	led_main();
	
	while (1) 
    {
#if 1
		led_all_on_off();
#else
		PORTA = 0xff;
		_delay_ms(1000); // sleep 상태이면 button등의 이벤트(환경 변화)를 check하지 못 한다.
		PORTA = 0x00;
		_delay_ms(1000);	
#endif
	}
}

// timer0를 초기화 시켜주는 것이다.
// AVR에서 8bit timer 0/2번 중에서 0번을 초기화 하는 것이다.
// 임베디드 / FPGA 등 제일 중요한 것이 초기화를 정확히 해주는 것이다.
// 그래서 이 부분을 특별히 신경을 써서 정확히 작성한다.
void init_timer0()
{
// 16Mhz -> 1/64로 down (분주 : divider/prescale)
// 1. 분주비 계산
// 16000000Hz / 64 = 250,000Hz
// 2. T(주기) 1가 잡아먹는 시간 : 1/f = 1/250,000 ==> 0.000004sec (4us) : 0.004ms
// 3. 8bit timer overflow : 0.004ms x 256 = 1.024ms
// 그러면 정확히 1ms를 재고 싶다면 0.004ms x 250개 = 1ms 

	TCNT0 = 6; // TCNT : 0 ~ 256 == 1ms마다 TIMER0_OVF_vect로 진입한다.
			   // 6으로 설정 한 이유 : 6~256  250개를 count(정확히 1ms를 맞추기 위해서)
// 4. 분주비 설정 (250khz)
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;
// 5. TIMER0 Overflow enable
	TIMSK |= 1 << TOIE0; // TIMSK |= 0x01;
//6. 전역 인터럽트
	sei(); // 전역적(대문)으로 interrupt 허용

} 

