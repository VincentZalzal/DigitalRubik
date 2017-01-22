#pragma once

namespace Leds
{

void Init();		// Initialize pins and wait during initial reset signal (80 us).
void Update();		// Update all LEDs according to the cube state; do NOT wait for reset.
void WaitForReset();	// Wait for reset signal (80 us).

}
