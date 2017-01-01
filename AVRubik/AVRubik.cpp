/*
 * AVRubik.cpp
 *
 * Created: 2017-01-01 16:14:44
 *  Author: Vincent
 */


#include <avr/io.h>

#define F_CPU 8000000UL
#include <util/delay.h>

int main(void)
{
	DDRD |= 0x01;
	PORTD = 0;
	DDRC |= 0x20;
	while(1)
	{
		PORTC |= 0x20;
		_delay_ms(2000);
		PORTC &= ~0x20;
		_delay_ms(2000);
	}
}
