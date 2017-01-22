/*
 * AVRubik.cpp
 *
 * Created: 2017-01-01 16:14:44
 *  Author: Vincent
 */

#include "avr_specific.h"
#include "leds.h"
#include "../Cube/cube.h"

int main(void)
{
	Leds::Init();
	Cube::Reset();
	Cube::Scramble(30);

	Leds::Update();

	while(1)
	{

	}
}
