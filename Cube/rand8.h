#pragma once

#include <cstdint>

namespace Rand8
{
typedef uint8_t Type;

const Rand8::Type MAX_RAND_VAL = 254;

void Seed(Type S);		// Initializes the pseudo-random number generator.
Type Get();			// Returns uniformly in [0, MAX_RAND_VAL] (period: 255)
Type Get(Type Low, Type High);	// Returns uniformly in [Low, High]; very slow if High-Low is small.

}
