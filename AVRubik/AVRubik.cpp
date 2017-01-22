/*
 * AVRubik.cpp
 *
 * Created: 2017-01-01 16:14:44
 *  Author: Vincent
 */

#include "avr_specific.h"
#include "rings.h"
#include "leds.h"
#include "../Cube/cube.h"

void Init()
{
	Rings::Init();
	Leds::Init();
	Cube::Reset();
}

int main(void)
{
	Init();

	Cube::Scramble(30);
	Leds::Update();

	while(1)
	{
		Rings::Read();
	}
}
