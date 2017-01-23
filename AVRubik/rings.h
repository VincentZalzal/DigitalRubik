#pragma once

namespace Rings
{

void Init();	// Initialize ADC and shift register pins.
void Read();	// Read status of all rings (about 25 ms).
void Reset();	// Reset debouncing bits to 0.

}
