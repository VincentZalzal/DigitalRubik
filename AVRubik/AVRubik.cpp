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

#define LED_STRIP_DELAY_US	80	// specs says >50, Pololu uses 80
#define LED_STRIP_PORT		PORTA
#define LED_STRIP_DDR		DDRA
#define LED_STRIP_PIN		PORTA3

// Sends the current color values to all LEDs using bitbanging.
// Timing is VERY important in this function.
void UpdateLEDs()
{
	const Facelet::Type* pFacelets = Cube::GetFacelets();
	for (uint8_t FaceletIdx = 0; FaceletIdx < Cube::NumFacelets; ++FaceletIdx)
	{
		const uint8_t* pColorComponents = &Colors[pFacelets[FaceletIdx]].r;
		for (uint8_t ColorIdx = 0; ColorIdx < 3; ++ColorIdx)
		{
			uint8_t ColorComponent = pColorComponents[ColorIdx];
			uint8_t NumBits = 8;
			
			// Number of cycles for each instruction is written in the comment.
			// The positive pulse duration must be 3 cycles for transmitting
			// a 0 and 7 cycles for transmitting a 1. The total time for
			// bit must be at least 10 cycles. The implementation below
			// takes 10 cycles for transmitting a 0 and 12 cycles for a 1.
			__asm__ __volatile__(	// volatile prohibits optimizations
			"NEXT_BIT:" "\n\t"
				"rol %[ColorComponent]" "\n\t"	// 1
				"sbi %[IOreg], %[Pin]" "\n\t"	// 2
				"brcs TX_1" "\n\t"		// 2 if taken, 1 otherwise

			"TX_0:" "\n\t"
				"cbi %[IOreg], %[Pin]" "\n\t"	// 2
				"nop" "\n\t"			// 1
				"dec %[NumBits]" "\n\t"		// 1
				"brne NEXT_BIT" "\n\t"		// 2 if taken, 1 otherwise
				"rjmp END_OF_BYTE" "\n\t"	// 2 (only happens between bytes)

			"TX_1:" "\n\t"
				"nop" "\n\t"			// 1
				"nop" "\n\t"			// 1
				"dec %[NumBits]" "\n\t"		// 1
				"cbi %[IOreg], %[Pin]" "\n\t"	// 2
				"brne NEXT_BIT" "\n\t"		// 2 if taken, 1 otherwise

			"END_OF_BYTE:" "\n\t"
			: [ColorComponent] "+r" (ColorComponent),		// in-out register
			  [NumBits]        "+r" (NumBits)			// in-out register
			: [IOreg]          "I"  (_SFR_IO_ADDR(LED_STRIP_PORT)),	// input immediate in 0-63
			  [Pin]            "I"  (LED_STRIP_PIN)			// input immediate in 0-63
			);
		}
	}
}

int main(void)
{
	// Set red LED port pin.
	//DDRC |= 0x20;
	//PORTC = 0;

	// Send LED reset signal.
	LED_STRIP_DDR  |= 0x01;
	LED_STRIP_PORT &= ~_BV(LED_STRIP_PIN);
	_delay_us(LED_STRIP_DELAY_US);

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
		//PINC |= 0x20;
		//_delay_ms(3000);
	}
}
