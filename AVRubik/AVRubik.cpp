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
	Rings::Init();
	Leds::Init();
	Cube::Reset();
	Rand8::Seed(42); // TODO: seed RNG using ADC
	// TODO: disable unused components to reduce power consumption
}

int main(void)
{
	Init();

	Cube::Scramble(NUM_SCRAMBLE_ROTATIONS);
	Leds::Update();

	while (!Cube::IsSolved())
	{
		Rings::Read();
		
		bool CubeHasChanged;
		Rotation::Type CurRotation;
		Controls::UpdateCube(&CurRotation, &CubeHasChanged);
		if (CubeHasChanged)
			Leds::Update();
		
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
	}
	
	while (1)
	{
		// Perform victory animation forever.
		Cube::BrightenRandom();
		Leds::Update();
		_delay_ms(VICTORY_ANIMATION_DELAY_MS);
	}
}
