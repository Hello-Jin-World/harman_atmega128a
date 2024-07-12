#include <avr/io.h>
extern int fnd_main(void);

int main(void)
{
	init_fnd();
    while (1) 
    {
		/*static int digit_select = 0; // 자리수 선택 변수 0~3   static : 전역변수처럼 작동

		switch(digit_select)
		{
			case 0 :
			FND_DIGIT_PORT = ~0x80;
			FND_DATA_PORT = 0x00;   // 0~9초
			break;

			case 1 :
			FND_DIGIT_PORT = ~0x40;
			FND_DATA_PORT = 0x00; // 10단위 초
			break;

			case 2 :
			FND_DIGIT_PORT = ~0x20;
			FND_DATA_PORT = 0x80; // 1단위 분
			break;

			case 3 :
			FND_DIGIT_PORT = ~0x10;
			FND_DATA_PORT = 0x00; // 10단위 분
			break;
		}
		digit_select++;
		digit_select %= 4; //다음 표시할 자리수 선택
		*/
		fnd_main();
    }
}

