#include "rand8.h"
#include "config.h"

// Unnamed namespace for internal details.
namespace
{

const Rand8::Type DEFAULT_SEED = 42; // Seed must never be 0
const Rand8::Type LFSR_POLYNOMIAL = 0xB8; // x^8 + x^6 + x^5 + x^4 + 1
Rand8::Type g_State = DEFAULT_SEED;

}

namespace Rand8
{

// Initializes the pseudo-random number generator.
void Seed(Type S)
{
	// Seed must never be 0
	g_State = (S == 0 ? DEFAULT_SEED : S);
}

// Returns uniformly in [0, 254] (period: 255)
// Based on 8-bit Galois LFSR. See:
//   http://doitwireless.com/2014/06/26/8-bit-pseudo-random-number-generator/
//   https://en.wikipedia.org/wiki/Linear-feedback_shift_register#Galois_LFSRs
Type Get()
{
	// On AVR, this could be done faster using lsr, brcc and eor.
	Type Carry = (g_State & 1);
	g_State >>= 1;
	if (Carry)
		g_State ^= LFSR_POLYNOMIAL;

	// LFSRs never generate 0; output [0, 254] instead of [1, 255].
	assert(g_State != 0);
	return g_State - 1;
}

// Returns uniformly in [Low, High]. This is done by using a modulus operation.
// However, if the random value is in the last interval, it may be skipped
// to avoid biased towards lower values.
// Because AVR does not have division or modulus operations, it is done
// using subtraction, which explains why it is very slow if (High-Low) is small.
Type Get(Type Low, Type High)
{
	assert(Low < High);
	assert(High <= MAX_RAND_VAL);
	Type Range = High - Low + 1;

	for (;;)
	{
		Type RandVal = Get();
		Type MaxVal = MAX_RAND_VAL + 1;

		for (;;)
		{
			if (RandVal < Range)
			{
				if (MaxVal >= Range)
				{
					// RandVal is inside an unbiased range, use it.
					Type ReturnValue = RandVal + Low;
					assert(Low <= ReturnValue && ReturnValue <= High);
					return ReturnValue;
				}
				else
				{
					// RandVal is in the last, biased range. Start over.
					break;
				}
			}

			// RandVal is out of range. Do modulo by hand.
			RandVal -= Range;
			MaxVal -= Range;
		}
	}
}

}
