#define F_CPU 16000000UL
#include <util/delay.h>
#include <avr/io.h>

#include "led.h"

void init_led(void);

void init_led(void)
{
	AUTO_RUN_LED_PORT_DDR |= 1 << AUTO_RUN_LED_PIN;
}