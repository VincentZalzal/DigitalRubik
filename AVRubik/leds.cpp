#include "leds.h"
#include "avr_specific.h"
#include "../Cube/cube.h"

namespace Leds
{

// Initialize pins and wait during initial reset signal (80 us).
void Init()
{
	LED_STRIP_DDR  |=  _BV(LED_STRIP_PIN);
	LED_STRIP_PORT &= ~_BV(LED_STRIP_PIN);
	WaitForReset();
}

// Update all LEDs according to the cube state; do NOT wait for reset.
// There must be a delay of at least 80 us between two calls to this function.
// If necessary, call WaitForReset() to ensure the proper delay.
// Sends the current color values to all LEDs using bitbanging.
// Timing is VERY important in this function.
void Update()
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

// Wait for reset signal (80 us).
void WaitForReset()
{
	_delay_us(80); // specs says >50, Pololu uses 80
}

}
