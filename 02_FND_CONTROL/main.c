#include <avr/io.h>
extern int fnd_main(void);

int main(void)
{
	init_fnd();
    while (1) 
    {
		fnd_main();
    }
}

