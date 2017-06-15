#include "avr_specific.h"
#include "rings.h"
#include "leds.h"
#include "../Cube/rand8.h"
#include "../Cube/cube.h"
#include "../Cube/controls.h"

#define NUM_SCRAMBLE_ROTATIONS		30
#define ROTATION_DELAY_MS		150
#define VICTORY_ANIMATION_DELAY_MS	400

void Init()
{
	// Remove the clk /8 prescaler.
	CLKPR = _BV(CLKPCE);
	CLKPR = 0;

	// Disable unused components to reduce power consumption.
	// USI, Timer/Counter 0, TImer/Counter 1.
	PRR = _BV(PRTIM1) | _BV(PRTIM0) | _BV(PRUSI);
	// Analog Comparator
	ACSR |= _BV(ACD);

	Rings::Init();
	Leds::Init();
	Cube::Reset();
	Controls::Reset();

	// Create RNG seed by using the ADC on an open pin.
	// For each conversion, keep the LSB.
	ADMUX |= _BV(MUX0); // Temporarily work on PA1 instead of PA0.
	Rand8::Type Seed = 0;
	for (uint8_t i = 0; i < 8*sizeof(Rand8::Type); ++i)
	{
		ADCSRA |= _BV(ADSC);			// start ADC conversion
		loop_until_bit_is_set(ADCSRA, ADIF);	// wait for conversion to end
		uint8_t Res = ADCL;			// read lower 2 bits (put in higher bits of Res)
		Res <<= 1;				// keep lsb only
		Seed >>= 1;				// leave room for the new bit
		Seed |= Res;				// add new bit
		Res = ADCH;				// finish reading the ADC
		ADCSRA |= _BV( ADIF );			// reset ADC interrupt flag
	}
	ADMUX &= ~_BV(MUX0); // Switch back to PA0.
	Rand8::Seed(Seed);
	// TODO: have debug code that shows the Seed on the LEDs.
}

int main(void)
{
	Init();

	Cube::Scramble(NUM_SCRAMBLE_ROTATIONS);
	Leds::Update();

	// Temporary debug code to tests rings.
	//while (1)
	//{
	//	Rings::Read();
	//	bool CubeHasChanged = Controls::UpdateCubeBrightness();
	//	if (CubeHasChanged)
	//	{
	//		Leds::Update();
	//		_delay_ms(200);
	//	}
	//}

	while (!Cube::IsSolved())
	{
		Rings::Read();

		bool CubeHasChanged = Controls::UpdateCubeBrightness();
		Rotation::Type CurRotation = Controls::DetermineAction();
		if (CurRotation != Rotation::None)
		{
			// Perform the rotation animation.
			Cube::RotateSide(CurRotation);
			Cube::RotateFront(CurRotation);
			Leds::Update();
			_delay_ms(ROTATION_DELAY_MS);
			
			Cube::RotateSide(CurRotation);
			Leds::Update();
			_delay_ms(ROTATION_DELAY_MS);

			Cube::RotateSide(CurRotation);
			Cube::RotateFront(CurRotation);
			Leds::Update();
			_delay_ms(ROTATION_DELAY_MS);
			
			Rings::Reset();
			Controls::Reset();
			Cube::DimAll();
			Leds::Update();
		}
		else if (CubeHasChanged)
		{
			Leds::Update();
		}
	}

	while (1)
	{
		// Perform victory animation forever.
		Cube::BrightenRandom();
		Leds::Update();
		_delay_ms(VICTORY_ANIMATION_DELAY_MS);
	}
}
