#include "avr_specific.h"
#include "rings.h"
#include "leds.h"
#include "../Cube/rand8.h"
#include "../Cube/cube.h"
#include "../Cube/controls.h"

#define RESET_DELAY_MS		100
#define ERROR_DELAY_MS		100

const uint8_t NumScrambleRotations = 15;

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

// This function is necessary to be able to sleep using a delay that isn't
// known at compile-time.
void Sleep(uint16_t DelayMs)
{
	while (DelayMs--)
		_delay_ms(1);
}

void Animate()
{
	for (;;)
	{
		uint16_t NextDelayMs = Cube::Animation::Next();
		Leds::Update();
		if (NextDelayMs == 0)
			break;
		Sleep(NextDelayMs);
	}
}

// Actions

void Reset()
{
	// Fade to black.
	Cube::SetToBlack();
	Leds::Update();
	_delay_ms(RESET_DELAY_MS);

	// Set cube to its solved state.
	Cube::Reset();
	Leds::Update();

	Controls::ResetActionQueue();
}

// Perform some random (animated) rotations.
void Scramble()
{
	STATIC_ASSERT(Rotation::Top == 0 && (Rotation::Bottom + Rotation::CCW) == Rotation::NumRotations - 1,
		      "The rotation constants are used as indices below.");

	uint8_t NumRotations = NumScrambleRotations;
	assert(NumRotations > 0);

	// Choose first rotation randomly.
	Rotation::Type PrevRot = Rand8::Get(0, Rotation::NumRotations - 1);
	
	// Perform the rotation animation.
	Cube::Animation::Rotate(PrevRot);
	Animate();

	while (--NumRotations)
	{
		// Choose the next rotation randomly so that it doesn't cancel
		// the previous one.
		Rotation::Type CurRot = Rand8::Get(0, Rotation::NumRotations - 2);
		if (CurRot >= Rotation::Opposite(PrevRot))
			++CurRot;

		// Perform the rotation animation.
		Cube::Animation::Rotate(CurRot);
		Animate();

		PrevRot = CurRot;
	}

	// Undo makes no sense after a scramble anyway.
	Controls::ResetActionQueue();
}

void Undo()
{
	// If not empty, pop action queue and do the
	// opposite rotation.
	Rotation::Type CurRotation = Controls::PopAction();
	if (CurRotation != Rotation::None)
	{
		Cube::Animation::Rotate(Rotation::Opposite(CurRotation));
		Animate();
	}
	else
	{
		// Nothing to undo, flash to indicate that fact.
		Cube::BrightenAll();
		Leds::Update();
		_delay_ms(ERROR_DELAY_MS);
		Cube::DimAll();
		Leds::Update();
	}
}

void Rotate(Rotation::Type CurRotation)
{
	// Add the rotation to the action queue.
	Controls::PushAction(CurRotation);

	// Perform the rotation animation.
	Cube::Animation::Rotate(CurRotation);
	Animate();

	// If the cube is solved, perform the victory animation.
	if (Cube::IsSolved())
	{
		Cube::Animation::Victory();
		Animate();
	}
}

int main(void)
{
	Init();
	Cube::Reset();
	Rings::Reset();
	Controls::ResetSensors();
	Controls::ResetActionQueue();
	Leds::Update();

	for (;;)
	{
		Rings::Read();
		bool CubeHasChanged = Controls::UpdateCubeBrightness();
		Action::Type CurAction = Controls::DetermineAction();

		switch (CurAction)
		{
		case Action::None:     if (CubeHasChanged) Leds::Update(); break;
		case Action::Reset:    Reset();                            break;
		case Action::Scramble: Scramble();                         break;
		case Action::Undo:     Undo();                             break;
		default:               Rotate(CurAction);                  break;
		}

		// If an action has been done, reset the sensors.
		if (CurAction != Action::None)
		{
			Rings::Reset();
			Controls::ResetSensors();
		}
	}
}
