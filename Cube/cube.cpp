#include "cube.h"

namespace 
{
Facelet::Type g_Facelets[Cube::NumFacelets]; // Global cube state.

const uint8_t NumSideFacelets     = 12; // Number of facelets on the side of a turning face.
const uint8_t NumFrontFacelets    =  8; // Number of facelets on the front of a turning face, excluding the center.
const uint8_t NumAffectedFacelets = NumSideFacelets + NumFrontFacelets + 1; // Number of facelets of a face.

typedef uint8_t FaceletIndex;

struct SRotation // contains NumAffectedFacelets facelets.
{
	FaceletIndex Side[NumSideFacelets];
	FaceletIndex Front[NumFrontFacelets];
	FaceletIndex Fixed;
};

// These are in the same order as namespace Rotation.
const SRotation Rot[6] =
{
	{{9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42}, {2, 5, 8, 7, 6, 3, 0, 1}, 4},			// top
	{{47, 46, 45, 20, 19, 18, 8, 5, 2, 42, 43, 44}, {11, 14, 17, 16, 15, 12, 9, 10}, 13},		// front
	{{51, 52, 53, 29, 28, 27, 6, 7, 8, 15, 16, 17}, {20, 23, 26, 25, 24, 21, 18, 19}, 22},		// right
	{{53, 50, 47, 38, 37, 36, 0, 3, 6, 24, 25, 26}, {29, 32, 35, 34, 33, 30, 27, 28}, 31},		// back
	{{47, 46, 45, 11, 10, 9, 2, 1, 0, 33, 34, 35}, {38, 41, 44, 43, 42, 39, 36, 37}, 40},		// left
	{{35, 32, 29, 26, 23, 20, 17, 14, 11, 44, 41, 38}, {47, 50, 53, 52, 51, 48, 45, 46}, 49}	// bottom
};

static void RotateImpl(const FaceletIndex* Indices, uint8_t NumFacelets)
{

	Facelet::Type Temp = g_Facelets[Indices[0]];
	for (uint8_t i = 0; i < NumFacelets-1; ++i)
	{
		g_Facelets[Indices[i]] = g_Facelets[Indices[i + 1]];
	}
	g_Facelets[Indices[NumFacelets - 1]] = Temp;
}
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
		for (uint8_t i = 0; i < NumFaceletsPerFace; ++i)
			*pFacelet++ = Color;
}

void Brighten(Rotation::Type Face)
{
	const FaceletIndex* Indices = &Rot[Face].Side[0];
	for (uint8_t i = 0; i < NumAffectedFacelets; ++i)
		g_Facelets[Indices[i]] |= Facelet::Bright;
}

void RotateSide(Rotation::Type Face)
{
	const FaceletIndex* Indices = &Rot[Face].Side[0];
	RotateImpl(Indices, NumSideFacelets);
}

void RotateFront(Rotation::Type Face)
{
	const FaceletIndex* Indices = &Rot[Face].Front[0];
	RotateImpl(Indices, NumFrontFacelets);
}

void Rotate(Rotation::Type Face)
{
	RotateSide(Face);
	RotateSide(Face);
	RotateSide(Face);
	RotateFront(Face);
	RotateFront(Face);
}

void DimAll()
{
	for (FaceletIndex i = 0; i < NumFacelets; ++i)
		g_Facelets[i] &= ~Facelet::Bright;
}

}
