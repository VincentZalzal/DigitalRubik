#include "avr_specific.h"
#include "rings.h"
#include "leds.h"
#include "../Cube/rand8.h"
#include "../Cube/cube.h"

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
		bool CubeHasChanged = true; // TODO: Controls::UpdateCube();
		if (CubeHasChanged)
			Leds::Update();
		
		// TODO: Controls::ActionType Action = DetermineAction();
		uint8_t Action = Rotation::Top;
		
		// TODO
		//if (Action == Undo)
		//{
		//	
		//}
		//else if (Action == Menu)
		//{
		//	
		//}
		//else
		{
			// Perform the rotation animation.
			Cube::RotateSide(Action);
			Cube::RotateFront(Action);
			Leds::Update();
			_delay_ms(ROTATION_DELAY_MS);
			
			Cube::RotateSide(Action);
			Leds::Update();
			_delay_ms(ROTATION_DELAY_MS);

			Cube::RotateSide(Action);
			Cube::RotateFront(Action);
			Leds::Update();
			_delay_ms(ROTATION_DELAY_MS);
			
			// TODO: Rings::Reset();
			// TODO: Controls::Reset();
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
