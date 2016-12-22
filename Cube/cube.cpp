#include "cube.h"

Facelet::Type g_Facelets[54];

struct SRotation
{
	uint8_t Side[12];
	uint8_t Front[8];
	uint8_t Fixed;
};

const SRotation Rot[2] =
{
	{{9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42}, {2, 5, 8, 7, 6, 3, 0, 1}, 4},
	{{15, 16, 17, 51, 52, 53, 29, 28, 27, 6, 7, 8}, {18, 19, 20, 23, 26, 25, 24, 21}, 22}
};

static void RotateImpl(const uint8_t* Indices, uint8_t NumFacelets)
{

	Facelet::Type Temp = g_Facelets[Indices[0]];
	for (uint8_t i = 0; i < NumFacelets-1; ++i)
	{
		g_Facelets[Indices[i]] = g_Facelets[Indices[i + 1]];
	}
	g_Facelets[Indices[NumFacelets - 1]] = Temp;
}

namespace Cube
{

const Facelet::Type* GetFacelets()
{
	return g_Facelets;
}

void Reset()
{
	// Numerical order of colors in Facelet match initialization order.
	Facelet::Type* pFacelet = g_Facelets;
	for (Facelet::Type Color = Facelet::White; Color < Facelet::Unused; ++Color)
		for (uint8_t i = 0; i < 9; ++i)
			*pFacelet++ = Color;
}

void Brighten(uint8_t Side)
{
	const uint8_t* Indices = &Rot[Side].Side[0];
	for (uint8_t i = 0; i < 21; ++i)
		g_Facelets[Indices[i]] |= Facelet::Bright;
}

void RotateSide(uint8_t Side)
{
	const uint8_t* Indices = &Rot[Side].Side[0];
	RotateImpl(Indices, 12);
}

void RotateFront(uint8_t Side)
{
	const uint8_t* Indices = &Rot[Side].Front[0];
	RotateImpl(Indices, 8);
}

void Rotate(uint8_t Side)
{
	RotateSide(Side);
	RotateSide(Side);
	RotateSide(Side);
	RotateFront(Side);
	RotateFront(Side);
}

void DimAll()
{
	for (uint8_t i = 0; i < 54; ++i)
		g_Facelets[i] &= ~Facelet::Bright;
}

}
