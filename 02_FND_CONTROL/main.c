#include <avr/io.h>
extern int fnd_main(void);

int main(void)
{
	init_fnd();
    while (1) 
    {
		/*static int digit_select = 0; // �ڸ��� ���� ���� 0~3   static : ��������ó�� �۵�

		switch(digit_select)
		{
			case 0 :
			FND_DIGIT_PORT = ~0x80;
			FND_DATA_PORT = 0x00;   // 0~9��
			break;

			case 1 :
			FND_DIGIT_PORT = ~0x40;
			FND_DATA_PORT = 0x00; // 10���� ��
			break;

			case 2 :
			FND_DIGIT_PORT = ~0x20;
			FND_DATA_PORT = 0x80; // 1���� ��
			break;

			case 3 :
			FND_DIGIT_PORT = ~0x10;
			FND_DATA_PORT = 0x00; // 10���� ��
			break;
		}
		digit_select++;
		digit_select %= 4; //���� ǥ���� �ڸ��� ����
		*/
		fnd_main();
    }
}

