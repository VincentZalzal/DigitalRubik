#include "cube.h"

Facelet::Type Facelets[54];

namespace Cube
{

const Facelet::Type* GetFacelets()
{
	return Facelets;
}

void Reset()
{
	// Numerical order of colors in Facelet match initialization order.
	Facelet::Type* pFacelet = Facelets;
	for (Facelet::Type Color = Facelet::White; Color < Facelet::Unused; ++Color)
		for (uint8_t i = 0; i < 9; ++i)
			*pFacelet++ = Color;
}

}
