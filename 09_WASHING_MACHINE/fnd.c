#include "fnd.h"
#include "button.h"
#define CUSTOM_WASH 0
#define AUTO_WASH 1
#define FAST_WASH 2
#define ONLY_SPINDRY 3
#define MAIN_SCREEN 4


void init_fnd(void);
void fnd_display(void); // 숫자 표시
void fnd_loading_display(); // 로딩 표시
void fnd_ment_display(); // 모드 멘트 표시
int fnd_main(void);

// 버튼
extern void init_button(void);
extern int get_button(int button_num, int button_pin);
// 모터
extern void washing_machine_fan_control();
// LED
extern void make_pwm_led_control();
// 초음파
extern void ultrasonic_distance_check();

// 메인 화면에서 선택하는 함수들
void custom_wash();
void auto_wash();
void fast_wash();
void only_spindry();
void main_screen();
void end_screen();

// 수동 세탁 모드에서 선탁하는 함수들
void water_temperature();
void rinse_frequency();
void spindry_strength();
void custom_wash_start();
void pause_wash();
void nextstep_pause_wash();
void end_display();
void dumy_fanc();

uint32_t sec_count = 0; // 초를 재는 count 변수 unsigned int = uint32_t
volatile int led_shift_num = 0;

extern volatile uint32_t fnd_refreshrate; // fnd 잔상효과를 유지하기 위한 변수 2ms
extern volatile uint32_t msec_count;
extern volatile uint32_t check_timer; // 모터 회전 방향 반대로 하기위한 시각 체크 변수
extern volatile uint32_t loading_clock_change;
extern volatile uint32_t loading_refreshrate;
extern volatile int ultrasonic_distance; // 초음파 거리 변수


int select_wash_mode = MAIN_SCREEN; // 메인화면에서 모드 선택 변수
int custom_wash_mode = 5; // 커스텀 세탁 모드안에서 진행과정 선택 변수
int custom_wash_mode_toggle = 1; // 커스텀 세탁 모드 안에서 모든 과정을 마쳤는지 아는 토글 / 이게 0 되면 세탁을 시작함.
int total_wash_time = 90; // 총 세탁 시각 default : 60초 + default 탈수 시간 30초
int spin_strength_val = 0; // 1단계 115, 2단계 160, 3단계 205, 4단계 250
volatile int loading_rot = 10; // 로딩  돌아가는거 보여주는 변수
int loading_clock_change_val = 1; // 시간이랑 로딩 창 번갈아 가는거 변수
int closed_check = 0; // 세탁기 문이 닫혀있는지 확인하는 변수

void (*fp_wash_mode[])() =
{
	custom_wash, // 0 수동 세탁
	auto_wash, // 1 자동 세탁
	fast_wash, // 2 쾌속 세탁
	only_spindry, // 3 탈수 단독
	main_screen // 4 Idle 메인화면
};

void (*custom_wash_select[])() =
{
	water_temperature, // 물 온도 선택
	rinse_frequency, // 헹굼 횟수 선택
	spindry_strength, // 탈수 강도 선택
	custom_wash_start, // 세탁 시작
	end_display, // end 화면
	dumy_fanc // dumy
};

int fnd_main(void)
{
	DDRA = 0xff;	//PORTA에 연결된 PIN 8개를 모두 output으로 설정. 출력 : 1
	init_fnd(); // fnd 초기화
	init_button(); // button 초기화
	
	int	main_ment_on = 0; // select mode 글씨가 fnd에 돌아가면서 표시되도록 하는 변수

	while(1)
	{
		if (get_button(BUTTON0, BUTTON0PIN) && closed_check == 1) // 버튼 0을 받으면 수동 세탁 모드로 진입하고 계속 버튼 0을 누를수록 다음 모드로 순회하듯이 작동, 버튼 1을 누르면 해당되는 모드 진입, 버튼 3을 누르면 메인화면으로 되돌아가기
		{
			loading_rot = 5;
			select_wash_mode = CUSTOM_WASH;
		}
		
		fp_wash_mode[select_wash_mode]();
		
		ultrasonic_distance_check();   //// 세탁기 문이 닫혔는지 확인하고 닫혀있으면 1을, 열려있으면 0을 반환
		if (ultrasonic_distance / 58 > 0 && ultrasonic_distance / 58 < 10)
		{
			closed_check = 1;
		}
		else
		{
			closed_check = 0;
		}
		
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_ment_display(&loading_rot,&main_ment_on);
		}
		
		if (msec_count >= 400) // SELECT MODE 글씨 fnd에 400ms마다 한칸씩 밀리게 표시하기
		{
			msec_count = 0;
			main_ment_on++;
			main_ment_on %= 13;
		}
	}
}

////////////////////////////////////////////메인화면에서 동작하는 함수들////////////////////////////////////////////////

void custom_wash() // 수동 세탁
{
	int custom_step_led = 0;
	
	while (custom_step_led < 4)
	{
		PORTA = 0xc0;
		if (get_button(BUTTON0, BUTTON0PIN)) // 버튼 0을 받으면 다음 쾌속 세탁으로 이동
		{
			loading_rot = 6;
			select_wash_mode = AUTO_WASH;
			break;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			select_wash_mode = 4;
			break;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			if (custom_step_led == 0) // 버튼 1을 누르면 수동 세탁을 시작하는 단계로 진입. 물 온도, 헹굼 횟수, 탈수 강도 선택
			{
				custom_wash_mode = 0;
			}
		}
		
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_ment_display(&loading_rot, 10);
		}
		custom_wash_select[custom_wash_mode]();
	}
	// 여기까지 수동 세탁의 물온도, 헹굼횟수, 탈수 강도를 선택함 밑에서 지정한 시간만큼 모터가 돌아가고 탈수 강도에 맞게 30초 동안 강도에 맞게 진행
	
	//여기에 시간 만큼 회전하는 함수 추가.
	
}

void auto_wash(int *auto_ment_on) // 자동 세탁
{
	while (1)
	{
		PORTA = 0x30;
		if (get_button(BUTTON0, BUTTON0PIN)) // 버튼 0을 받으면 다음 헹굼 + 탈수 세탁으로 이동
		{
			loading_rot = 7;
			select_wash_mode = FAST_WASH;
			break;
		}
		
		if (get_button(BUTTON1, BUTTON1PIN)) // 버튼 1을 받으면 바로 자동 세탁 custom_wash_select[3]이 세탁 시작 함수라서 거기로 가기
		{
			total_wash_time = 150;
			spin_strength_val = 160;
			custom_wash_mode = 3;
			break;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			select_wash_mode = 4;
			break;
		}
		
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_ment_display(&loading_rot, 10);
		}
		custom_wash_select[custom_wash_mode]();
	}
}

void fast_wash(int *fast_ment_on) // 쾌속 세탁
{
	while (1)
	{
		PORTA = 0x0c;
		if (get_button(BUTTON0, BUTTON0PIN)) // 버튼 0을 받으면 다음 탈수 세탁으로 이동
		{
			loading_rot = 8;
			select_wash_mode = ONLY_SPINDRY;
			break;
		}
		
		if (get_button(BUTTON1, BUTTON1PIN)) // 버튼 1을 받으면 바로 쾌속 세탁 custom_wash_select[3]이 세탁 시작 함수라서 거기로 가기
		{
			total_wash_time = 70;
			spin_strength_val = 250;
			custom_wash_mode = 3;
			break;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			select_wash_mode = 4;
			break;
		}
		
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_ment_display(&loading_rot, 10);
		}
		custom_wash_select[custom_wash_mode]();
	}
}

void only_spindry(int *dry_ment_on) // 탈수
{
	while (1)
	{
		PORTA = 0x03;
		if (get_button(BUTTON0, BUTTON0PIN)) // 버튼 0을 받으면 다시 수동 세탁으로 이동
		{
			loading_rot = 5;
			select_wash_mode = CUSTOM_WASH;
			break;
		}
		
		if (get_button(BUTTON1, BUTTON1PIN)) // 버튼 1을 받으면 바로 탈수 선택 custom_wash_select[2]이 탈수 강도 선택 함수라서 거기로 가기
		{
			total_wash_time = 30;
			custom_wash_mode = 2;
			break;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			select_wash_mode = 4;
			break;
		}
		
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_ment_display(&loading_rot, 10);
		}
		custom_wash_select[custom_wash_mode]();
	}
}

void main_screen(int *main_num) // 메인 화면
{
	PORTA = 0;
	loading_rot = 10; // NULL
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////수동 세척 모드에서 동작하는 함수들////////////////////////////////////////////
void water_temperature() // 물 온도 선택
{
	loading_rot = 0;
	PORTA = 0x07;
	int water_tem_toggle = 1;
	
	sec_count = 30;  // deflaut 물 온도
	
	while (water_tem_toggle)
	{
		if (get_button(BUTTON2, BUTTON2PIN))
		{
			sec_count -= 5;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN))
		{
			sec_count += 5;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			sec_count = 0;
			custom_wash_mode = 1;
			water_tem_toggle = 0;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
		if (custom_wash_mode == 5)
		{
			break;
		}
	}
}

void rinse_frequency() // 헹굼 횟수 조절
{
	PORTA = 0x03;
	int rinse_frequency_toggle = 1; // 버튼1 누를 때 까지 반복
	
	sec_count = 2; // deflaut 헹굼 횟수
	
	while (rinse_frequency_toggle)
	{
		if (get_button(BUTTON2, BUTTON2PIN)) // 버튼 2 누르면 횟수 1 감소
		{
			sec_count -= 1;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN)) // 버튼 3 누르면 횟수 1 증가
		{
			sec_count += 1;
		}
		if (get_button(BUTTON1, BUTTON1PIN))
		{
			total_wash_time += sec_count * 10; // 버튼 1누르면 횟수 * 10초를 전역변수에 저장하고 다음 단계로 간 다음 탈출
			sec_count = 0;
			custom_wash_mode = 2;
			rinse_frequency_toggle = 0;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}

void spindry_strength()
{
	PORTA = 0x01;

	int spindry_strength_toggle = 1; // 버튼1 누를 때 까지 반복
	
	sec_count = 3; // deflaut 탈수 강도 최대 4까지
	
	while (spindry_strength_toggle)
	{
		if (get_button(BUTTON2, BUTTON2PIN)) // 버튼 2 누르면 횟수 1 감소
		{
			sec_count -= 1;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN)) // 버튼 3 누르면 횟수 1 증가
		{
			sec_count += 1;
		}
		if (get_button(BUTTON1, BUTTON1PIN)) // 버튼 1 누르면 세팅한 강도에 맞는 회전값 반환
		{
			if(sec_count == 1)
			{
				spin_strength_val = 123; // duty cycle 50%
			}
			else if (sec_count == 2)
			{
				spin_strength_val = 160; // duty cycle 62%
			}
			else if (sec_count == 3)
			{
				spin_strength_val = 205; // duty cycle 80%
			}
			else if (sec_count == 4)
			{
				spin_strength_val = 250; // duty cycle 98%
			}
			PORTA = 0;
			spindry_strength_toggle = 0;
			custom_wash_mode = 3;
			sec_count = 0;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
	}
}


void custom_wash_start()
{
	int return_enable = 1;
	int led_pwm_count; // LED 몇개 킬건지 정하는 변수
	int forward_state = 0;
	int spin_forward_count = 0;
	loading_rot = 0;
	
	sec_count = total_wash_time;
	
	while (sec_count > 0 && return_enable == 1)
	{
		washing_machine_fan_control(&spin_strength_val, &forward_state);
		
		if (msec_count >= 500) // 1초마다 시간 1초씩 감소하고 로딩 회전이 됨.
		{
			msec_count = 0;
			sec_count--;
		}
		
		if (loading_refreshrate >= 300)
		{
			loading_refreshrate = 0;
			spin_forward_count++;
			spin_forward_count %= 4;
		}
		
		if (loading_clock_change >= 3000) // 3초마다 로딩창이랑 시간 화면 뜨는거 토글
		{
			loading_clock_change = 0;
			loading_clock_change_val = !loading_clock_change_val;
		}
		
		/////여기에 일반세탁 60초 + 헹굼 횟수 * 10초 + 탈수 30초로 구분을 해야함
		///// 일반세탁 60초 시작 /////
		if (total_wash_time - sec_count <= 30) // 일반 세탁 30초 경과
		{
			led_pwm_count = 255; // 0xff
		}
		
		else if (total_wash_time - sec_count < 60 && total_wash_time - sec_count > 30) // 일반세탁 60초 완료
		{
			led_pwm_count = 126; // 0x7c
		}
		///// 일반세탁 60초 끝 /////
		else if (total_wash_time - sec_count == 60)
		{
			nextstep_pause_wash(); // 모드가 바뀔 때 5초동안 멈춤
		}
		///// 헹굼 시작 /////
		else if (total_wash_time - sec_count > 60 && sec_count > 30) // 헹굼
		{
			led_pwm_count = 60; // 0x3c
		}
		///// 헹굼 끝 /////
		else if (sec_count == 30)
		{
			nextstep_pause_wash(); // 모드가 바뀔 때 5초동안 멈춤
		}
		///// 탈수 시작 /////
		else if (sec_count < 30)
		{
			led_pwm_count = 24; // 0x18
		}
		///// 탈수 끝 /////
		else if (sec_count == 0)
		{
			led_pwm_count = 0;
		}
		
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			make_pwm_led_control(&led_pwm_count);
			if (loading_clock_change_val)
			{
				fnd_display(); // 시간 보여주기
			}
			else
			{
				fnd_loading_display(&spin_forward_count, &forward_state); // 로딩 보여주기
			}
		}
		
		ultrasonic_distance_check();   //// 세탁기 문이 닫혔는지 확인하고 닫혀있으면 1을, 열려있으면 0을 반환
		if (ultrasonic_distance / 58 > 0 && ultrasonic_distance / 58 < 10)
		{
			closed_check = 1;
		}
		else
		{
			closed_check = 0;
		}
		
		///// 일시정지 구현
		
		if (closed_check == 0) // 뚜껑 열리면 일시정지
		{
			return_enable = 0;
			pause_wash(&return_enable, &led_pwm_count);
		}
		if (get_button(BUTTON0, BUTTON0PIN)) // 버튼 0 누르면 일시정지
		{
			closed_check = 0;
			return_enable = 0;
			pause_wash(&return_enable, &led_pwm_count);
		}
		
	}
	OCR3C = 0;
	custom_wash_mode = 4; // end 화면으로 이동
	sec_count = 0; // 다 끝나면 끝
}

void pause_wash(int *return_enable, int *led_pause_count)
{
	while (*return_enable == 0)
	{
		OCR3C = 0;
		
		ultrasonic_distance_check();   //// 세탁기 문이 닫혔는지 확인하고 닫혀있으면 1을, 열려있으면 0을 반환
		if (ultrasonic_distance / 58 > 0 && ultrasonic_distance / 58 < 10)
		{
			closed_check = 1;
		}
		else
		{
			closed_check = 0;
		}
		
		if (get_button(BUTTON0, BUTTON0PIN) && closed_check == 1) // 뚜껑을 닫고 버튼 0 누르면 이어서 시작
		{
			*return_enable = 1;
			custom_wash_mode = 3;
		}
		
		if (get_button(BUTTON3, BUTTON3PIN)) // 버튼 3 누르면 세탁 취소
		{
			*return_enable = 1;
			OCR3C = 0;
			custom_wash_mode = 4;
			sec_count = 0;
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_display();
		}
		PORTA = *led_pause_count;
	}
}

void nextstep_pause_wash()
{
	int wait_for_three_sec = 1;
	loading_rot = 9;
	int prerpare_ment = 0;
	
	while (wait_for_three_sec)
	{
		OCR3C = 0;
		if(msec_count >= 5000)
		{
			wait_for_three_sec = 0;
		}
		if (loading_clock_change >= 300) // 모드가 바뀔 때 prepare 글자를 출력함
		{
			loading_clock_change = 0;
			prerpare_ment++;
			prerpare_ment %= 9;
		}
		fnd_ment_display(&loading_rot, &prerpare_ment);
	}
}

void end_display() // end 화면
{
	int end_toggle = 1;
	PORTA = 0;
	loading_rot = 4; // end fnd display
	while (end_toggle)
	{
		if (get_button(BUTTON0, BUTTON0PIN) || get_button(BUTTON1, BUTTON1PIN) || get_button(BUTTON2, BUTTON2PIN) || get_button(BUTTON3, BUTTON3PIN)) // 아무 버튼 누르면 다시 메인 화면으로
		{
			select_wash_mode = MAIN_SCREEN;
			end_toggle = !end_toggle;
			custom_wash_mode = 5; // 끝
		}
		if (fnd_refreshrate >= 2) // 2ms 주기로 fnd를 display
		{
			fnd_refreshrate = 0;
			fnd_ment_display(&loading_rot,10); // end 보여주기
		}
		if (msec_count <= 500)
		{
			PORTA = 0b01010101;
		}
		else if (msec_count > 500)
		{
			PORTA = 0b10101010;
			if (msec_count >= 1000)
			{
				msec_count = 0;
			}
		}
	}
}

void dumy_fanc()
{
	PORTA = 0;
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////fnd 초기화 및 fnd 표시 함수들 ///////////////////////////////////////////////////////////

void init_fnd(void)
{
	FND_DATA_DDR = 0xff; // 출력모드로 설정

	FND_DIGIT_DDR |= 1 << FND_DIGIT_D1 | 1 << FND_DIGIT_D2 |
	1 << FND_DIGIT_D3 | 1 << FND_DIGIT_D4;

	FND_DATA_PORT = 0x00; // FND를 all off
}

void fnd_display(void)
{
	//  0      1       2      3      4      5      6     7      8       9      .
	uint8_t fnd_font[] = {~0xc0, ~0xf9, ~0xa4, ~0xb0, ~0x99, ~0x92, ~0x82, ~0xd8, ~0x80, ~0x90, ~0x7f};

	static int digit_select = 0; // 자리수 선택 변수 0~3   static : 전역변수처럼 작동

	switch(digit_select)
	{
		case 0 :
		FND_DIGIT_PORT = ~0x80;
		FND_DATA_PORT = fnd_font[sec_count % 10];   // 0~9초
		break;

		case 1 :
		FND_DIGIT_PORT = ~0x40;

		FND_DATA_PORT = fnd_font[sec_count / 10 % 6]; // 10단위 초
		break;

		case 2 :
		FND_DIGIT_PORT = ~0x20;
		FND_DATA_PORT = fnd_font[sec_count / 60 % 10] | fnd_font[10]; // 1단위 분
		break;

		case 3 :
		FND_DIGIT_PORT = ~0x10;
		FND_DATA_PORT = fnd_font[sec_count / 600 % 6]; // 10단위 분
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}

void fnd_ment_display(int *loading_rot, int *main_ment) // 진행 로딩 상황 표시
{
	int a, b, c, d;
	
	uint8_t fnd_loading_font[] =
	// s     e        l      e     c     t               m     o     d        e                    n
	{~0x92, ~0x86, ~0xc7, ~0x86, ~0xc6, ~0x87, ~0xff, ~0xaa, ~0xa3, ~0xa1, ~0x86, ~0xff, ~0xff, ~0xab // 0~13
		, ~0xc6, ~0x92, ~0x87, ~0xaa	// CSTM // 14번 칸 부터
		, ~0x88, ~0xc1, ~0x87, ~0xc0	// AUTO // 18
		, ~0x8e, ~0x88, ~0x92, ~0x87	// FAST // 22
		, ~0xff, ~0xa1, ~0xaf, ~0x91	//  DRY // 26
		, ~0xff, ~0xff, ~0x8c, ~0xaf, ~0x86, ~0x8c, ~0x88, ~0xaf, ~0x86, /*~0x7f, ~0x7f, ~0x7f*/ // 30~38   prepare
		//     ' '     ' '    p       r     e      p     a      r      e       .       .      .
	};
	
	if (*loading_rot == 4) // end screen
	{
		a = 9; b = 13; c = 1; d = 6;
	}
	
	else if (*loading_rot == 5) // custom
	{
		a = 17; b = 16; c = 15; d = 14;
	}
	else if (*loading_rot == 6) // auto
	{
		a = 21; b = 20; c = 19; d = 18;
	}
	else if (*loading_rot == 7) // fast
	{
		a = 25; b = 24; c = 23; d = 22;
	}
	else if (*loading_rot == 8) // dry
	{
		a = 29; b = 28; c = 27; d = 26;
	}
	
	else if (*loading_rot == 9)
	{
		d = *main_ment;
		c = d + 1; c %= 9;
		b = c + 1; b %= 9;
		a = b + 1; a %= 9;
		a += 30; b += 30; c += 30; d += 30;
	}
	
	else // main screen
	{
		d = *main_ment;
		c = d + 1; c %= 13;
		b = c + 1; b %= 13;
		a = b + 1; a %= 13;
	}

	static int digit_select = 0; // 자리수 선택 변수 0~3   static : 전역변수처럼 작동

	switch(digit_select)
	{
		case 0 :
		FND_DIGIT_PORT = ~0x80;
		FND_DATA_PORT = fnd_loading_font[a];
		break;

		case 1 :
		FND_DIGIT_PORT = ~0x40;
		FND_DATA_PORT = fnd_loading_font[b];
		break;

		case 2 :
		FND_DIGIT_PORT = ~0x20;
		FND_DATA_PORT = fnd_loading_font[c];
		break;

		case 3 :
		FND_DIGIT_PORT = ~0x10;
		FND_DATA_PORT = fnd_loading_font[d];
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}


void fnd_loading_display(int *forward_state_count, int *forward_state) // 진행 로딩 상황 표시
{
	int a, b, c, d;
	
	uint8_t fnd_loading_font[] =
	{
		~0xce, ~0xf8, ~0xc7, ~0xf1, ~0xfe, ~0xf7, ~0xf6 // 0~6
		//  |-     -|    |_     _|     -      _     -,_
	};
	
	if (*forward_state)
	{
		if (*forward_state_count == 0)
		{
			d = 0; b = 5, c = 4, a = 3;	// loading 1
		}
		else if (*forward_state_count == 1)
		{
			d = 2; b = 4, c = 5, a = 1; // loading 2
		}
		else if (*forward_state_count == 2)
		{
			d = 5; b = 6, c = 6, a = 4; // loaing 3
		}
		else if (*forward_state_count == 3)
		{
			d = 4; b = 6, c = 6, a = 5; // loaing 4
		}
	}
	else
	{
		if (*forward_state_count == 3)
		{
			d = 0; b = 5, c = 4, a = 3;	// loading 1
		}
		else if (*forward_state_count == 2)
		{
			d = 2; b = 4, c = 5, a = 1; // loading 2
		}
		else if (*forward_state_count == 1)
		{
			d = 5; b = 6, c = 6, a = 4; // loaing 3
		}
		else if (*forward_state_count == 0)
		{
			d = 4; b = 6, c = 6, a = 5; // loaing 4
		}
	}
	
	static int digit_select = 0; // 자리수 선택 변수 0~3   static : 전역변수처럼 작동

	switch(digit_select)
	{
		case 0 :
		FND_DIGIT_PORT = ~0x80;
		FND_DATA_PORT = fnd_loading_font[a];
		break;

		case 1 :
		FND_DIGIT_PORT = ~0x40;
		FND_DATA_PORT = fnd_loading_font[b];
		break;

		case 2 :
		FND_DIGIT_PORT = ~0x20;
		FND_DATA_PORT = fnd_loading_font[c];
		break;

		case 3 :
		FND_DIGIT_PORT = ~0x10;
		FND_DATA_PORT = fnd_loading_font[d];
		break;
	}
	digit_select++;
	digit_select %= 4; //다음 표시할 자리수 선택
}



