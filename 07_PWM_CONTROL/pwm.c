void hw_pwm_fan_control();

/*
	16bit 3번 timer/counter를 사용
	pwm출력 신호
	============
	PE3 : OC3A
	PE4 : OC3B
	PE5 : OC3C <-- 모터 연결
	BTN0 : start/stop
	BTN1 : speed-up (OCR3C : 20증가 최대 250)
	BTN2 : speed-down (OCR3C : 20감소 최소 60)
*/
void init_timer3_pwm()
{
	DDRE
}

void hw_pwm_fan_control(void)
{
	
}