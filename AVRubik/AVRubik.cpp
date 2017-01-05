/*
 * AVRubik.cpp
 *
 * Created: 2017-01-01 16:14:44
 *  Author: Vincent
 */


#include <avr/io.h>

#define F_CPU 8000000UL
#include <util/delay.h>

#include "../Cube/cube.h"

#define LED_STRIP_DELAY_MS	80
#define LED_STRIP_PORT		PORTD
#define LED_STRIP_DDR		DDRD
#define LED_STRIP_PIN		0

void UpdateLEDs()
{
	const Facelet::Type* pFacelets = Cube::GetFacelets();
	
	uint8_t NumFacelets = Cube::NumFacelets;
	do
	{
		Facelet::Type CurFacelet = *pFacelets++;
		const uint8_t* pColorComponents = &Colors[CurFacelet].r;
		
		uint8_t NumComponents = 3;
		do 
		{
			uint8_t ColorComponent = *pColorComponents++;
			uint8_t NumBits = 8;
			do
			{
				uint8_t Carry = ColorComponent & 0x80;
				ColorComponent <<= 1;
				if (Carry)
				{
					__asm__ __volatile__( "\n"
						"sbi %0, %1" "\n\t"
						"nop" "\n\t"
						"nop" "\n\t"
						"nop" "\n\t"
						"nop" "\n\t"
						"nop" "\n\t"
						"cbi %0, %1" "\n\t"
						:
						: "I" (_SFR_IO_ADDR(LED_STRIP_PORT)),	// %0 is the port register (e.g. PORTD)
						  "I" (LED_STRIP_PIN)			// %1 is the pin number (0-8)
					);
				}
				else
				{
					__asm__ __volatile__( "\n"
						"sbi %0, %1" "\n\t"
						"nop" "\n\t"
						"cbi %0, %1" "\n\t"
						"nop" "\n\t"
						"nop" "\n\t"
						"nop" "\n\t"
						"nop" "\n\t"
						:
						: "I" (_SFR_IO_ADDR(LED_STRIP_PORT)),	// %0 is the port register (e.g. PORTD)
						  "I" (LED_STRIP_PIN)			// %1 is the pin number (0-8)
					);
				}
			} while (--NumBits);
		} while (--NumComponents);
		
	} while (--NumFacelets);
}

int main(void)
{
	// Set red LED port pin.
	DDRC |= 0x20;
	PORTC = 0;

	// Send LED reset signal.
	LED_STRIP_DDR  |= 0x01;
	LED_STRIP_PORT &= ~_BV(LED_STRIP_PIN);
	_delay_ms(LED_STRIP_DELAY_MS);

	Cube::Reset();
	
	// Temporary ugly code to test LEDs.
	//Facelet::Type* pFacelets = (Facelet::Type*)Cube::GetFacelets();
	//int ii = 0;

	while(1)
	{
		// Temporary ugly code to test LEDs.
		//pFacelets[0] = ii;
		//pFacelets[1] = ii + 1;
		//pFacelets[2] = ii + 2;
		//if (ii == 12)
		//	ii = 0;
		//else
		//	++ii;
	
		UpdateLEDs();

		// Toggle red LED to know we're still alive.
		PINC |= 0x20;
		_delay_ms(3000);
	}
}
