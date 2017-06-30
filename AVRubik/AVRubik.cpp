#include "avr_specific.h"
#include "rings.h"
#include "leds.h"
#include "../Cube/rand8.h"
#include "../Cube/cube.h"
#include "../Cube/controls.h"

#define NUM_SCRAMBLE_ROTATIONS_NORMAL	30
#define NUM_SCRAMBLE_ROTATIONS_EASY	1
#define VICTORY_ANIMATION_DELAY_MS	400
#define NUM_VICTORY_ANIMATION_ITER	15

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
}

void Reset()
{
	Cube::Reset();
	Rings::Reset();
	Controls::ResetSensors();
	Controls::ResetActionQueue();
}

// This function is necessary to be able to sleep using a delay that isn't
// known at compile-time.
void Sleep(uint16_t DelayMs)
{
	while (DelayMs--)
		_delay_ms(1);
}

void Animate(bool HiSpeed = false)
{
	for (;;)
	{
		uint16_t NextDelayMs = Cube::Animation::Next();
		Leds::Update();
		if (NextDelayMs == 0)
			break;
		if (HiSpeed)
			NextDelayMs = (NextDelayMs + 7) >> 3; // ceil(NextDelayMs / 8)
		Sleep(NextDelayMs);
	}
}

int main(void)
{
	Init();
	
	uint8_t NumScrambleRotations = NUM_SCRAMBLE_ROTATIONS_NORMAL;
	for (;;)
	{
		Reset();
		Cube::Scramble(NumScrambleRotations);
		Leds::Update();

		bool MustReset = false;
		while (!Cube::IsSolved() && !MustReset)
		{
			Rings::Read();
			bool CubeHasChanged = Controls::UpdateCubeBrightness();
			
			Action::Type CurAction = Controls::DetermineAction();
			Rotation::Type CurRotation = Rotation::None;
			switch (CurAction)
			{
			case Action::ResetEasy:
				NumScrambleRotations = NUM_SCRAMBLE_ROTATIONS_EASY;
				MustReset = true;
				break;
			case Action::ResetNormal:
				NumScrambleRotations = NUM_SCRAMBLE_ROTATIONS_NORMAL;
				MustReset = true;
				break;
			case Action::None:
				if (CubeHasChanged)
					Leds::Update();
				break;
			case Action::Undo:
				// If not empty, pop action queue and do the
				// opposite rotation.
				CurRotation = Controls::PopAction();
				if (CurRotation != Rotation::None)
					CurRotation = Rotation::Opposite(CurRotation);
				break;
			default:
				// This is a rotation, add it to the action queue.
				CurRotation = CurAction;
				Controls::PushAction(CurRotation);
				break;
			}
			
			if (CurRotation != Rotation::None)
			{
				Cube::Animation::Rotate(CurRotation);
				Animate();
				Rings::Reset();
				Controls::ResetSensors();
			}
		}

		if (!MustReset)
		{
			// Perform victory animation.
			for (uint8_t i = 0; i< NUM_VICTORY_ANIMATION_ITER; ++i)
			{
				Cube::BrightenRandom();
				Leds::Update();
				_delay_ms(VICTORY_ANIMATION_DELAY_MS);
			}
			Cube::DimAll();
			Leds::Update();
			
			// Wait for reset command.
			while (!MustReset)
			{
				Rings::Read();
				Action::Type CurAction = Controls::DetermineAction();
				switch (CurAction)
				{
				case Action::ResetEasy:
					NumScrambleRotations = NUM_SCRAMBLE_ROTATIONS_EASY;
					MustReset = true;
					break;
				case Action::ResetNormal:
					NumScrambleRotations = NUM_SCRAMBLE_ROTATIONS_NORMAL;
					MustReset = true;
					break;
				default:
					// Ignore all other actions
					break;
				}
			}
		}
	}
}
