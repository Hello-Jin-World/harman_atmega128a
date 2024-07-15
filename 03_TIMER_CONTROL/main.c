// Timer : ���� �ð��� �Ǿ�����?
// Counter : �޽��� ������ ����.

#define F_CPU 16000000L
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h> // sei ��

extern int led_main(void);

volatile uint32_t msec_count = 0; // ���ͷ�Ʈ ���� ��ƾ���� ���� ���� type�տ��� volatile�̶�� ����
							   // �̴� ����ȭ�� �����ϱ� �����̴�
//------����ȭ ����------
// 1) int i;
// i = 1;
// i = 2;
// i = 3;
// compiler�� i=1; i=2;�� �����ϰ� i=3;�� �����ϴ� ��찡 �߻��Ѵ�.
// 2) 1~10������ ���� ���Ѵ�.
// (1) sum = 0; i = 0;
// (2) while (i <= 10)
// (3) {
// (4)  	i++; sum += i;
// (5) }
// (6) printf("sum ==> %d\n", sum);
// compiler�� (1)~(5)������ ������ �� �ϰ� (6)���� 1~10������ ���� 55�� ����ϴ� ��찡 �߻��Ѵ�.
// volatile�� �����ϸ� �ϳ����� �� �� ������.

// HW �� SW ���� interrput�� �߻��ߴٴ� ���� �˷��ִ� ����
// TIMER0_OVF_vect : interrupt ��ȣ �̸�
// 250���� pulse�� count(1ms)�ϸ� �̰����� �ڵ����� ���´�.
// ISR�� ������ �ִ������� ª�� �ۼ��Ѵ�.
ISR(TIMER0_OVF_vect) // interrupt service routine :routine�� interrupt ���� �Լ�
{
	TCNT0 = 6; // 6 ~ 256 : 250�� (1ms) �׷��� TCNT0�� 6���� ����
	msec_count++; // 1ms���� msec_count�� 1�� ����
}

int main(void)
{
	init_timer0();
	DDRA = 0xff; // led ��� ���
	PORTA = 0x00; // led all off
	
	led_main();
	
	while (1) 
    {
#if 1
		led_all_on_off();
#else
		PORTA = 0xff;
		_delay_ms(1000); // sleep �����̸� button���� �̺�Ʈ(ȯ�� ��ȭ)�� check���� �� �Ѵ�.
		PORTA = 0x00;
		_delay_ms(1000);	
#endif
	}
}

// timer0�� �ʱ�ȭ �����ִ� ���̴�.
// AVR���� 8bit timer 0/2�� �߿��� 0���� �ʱ�ȭ �ϴ� ���̴�.
// �Ӻ���� / FPGA �� ���� �߿��� ���� �ʱ�ȭ�� ��Ȯ�� ���ִ� ���̴�.
// �׷��� �� �κ��� Ư���� �Ű��� �Ἥ ��Ȯ�� �ۼ��Ѵ�.
void init_timer0()
{
// 16Mhz -> 1/64�� down (���� : divider/prescale)
// 1. ���ֺ� ���
// 16000000Hz / 64 = 250,000Hz
// 2. T(�ֱ�) 1�� ��ƸԴ� �ð� : 1/f = 1/250,000 ==> 0.000004sec (4us) : 0.004ms
// 3. 8bit timer overflow : 0.004ms x 256 = 1.024ms
// �׷��� ��Ȯ�� 1ms�� ��� �ʹٸ� 0.004ms x 250�� = 1ms 

	TCNT0 = 6; // TCNT : 0 ~ 256 == 1ms���� TIMER0_OVF_vect�� �����Ѵ�.
			   // 6���� ���� �� ���� : 6~256  250���� count(��Ȯ�� 1ms�� ���߱� ���ؼ�)
// 4. ���ֺ� ���� (250khz)
	TCCR0 |= 1 << CS02 | 0 << CS01 | 0 << CS00;
// 5. TIMER0 Overflow enable
	TIMSK |= 1 << TOIE0; // TIMSK |= 0x01;
//6. ���� ���ͷ�Ʈ
	sei(); // ������(�빮)���� interrupt ���

} 

