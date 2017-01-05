#include "cube.h"
#include "config.h"
#include "rand8.h"

STATIC_ASSERT(Facelet::Bright == 8, "This constant must be bitwise-exclusive with the others.");

// Color LUT: index must be a Facelet::Type.
// It must be in SRAM for fast access during LED update.
// Color order is RGB.
const SColor Colors[15] =
{
	{  0,   0,   0}, // Black 
	{128, 128, 128}, // White 
	{128,   0,   0}, // Red   
	{  0,   0, 128}, // Blue  
	{128,  64,   0}, // Orange
	{  0, 128,   0}, // Green 
	{128, 128,   0}, // Yellow
	{128,   0, 128}, // Unused (magenta)
	{  0,   0,   0}, // Bright Black 
	{255, 255, 255}, // Bright White 
	{255,   0,   0}, // Bright Red   
	{  0,   0, 255}, // Bright Blue  
	{255, 128,   0}, // Bright Orange
	{  0, 255,   0}, // Bright Green 
	{255, 255,   0}  // Bright Yellow
};

// Unnamed namespace for internal details.
namespace 
{
Facelet::Type g_Facelets[Cube::NumFacelets]; // Global cube state.

const uint8_t NumSideFacelets     = 12; // Number of facelets on the side of a turning face.
const uint8_t NumFrontFacelets    =  8; // Number of facelets on the front of a turning face, excluding the center.
const uint8_t NumAffectedFacelets = NumSideFacelets + NumFrontFacelets + 1; // Number of facelets of a face.

typedef uint8_t FaceletIndex;

// Static data structure containing all affected facelets for a single rotation.
struct SRotation
{
	FaceletIndex Side[NumSideFacelets];	// indices of facelets on the side of the turning face.
	FaceletIndex Front[NumFrontFacelets];	// indices of facelets on the front of the turning face, excluding the center.
	FaceletIndex Fixed;			// index of the center facelet of the turning face.
	// contains a total of NumAffectedFacelets facelets.
};

// These are in the same order as namespace Rotation.
// This is stored in flash memory and must be accessed using pgm_read_byte().
const SRotation Rot[Cube::NumFaces] PROGMEM =
{
	{{ 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42}, { 2,  5,  8,  7,  6,  3,  0,  1},  4},	// top
	{{47, 46, 45, 20, 19, 18,  8,  5,  2, 42, 43, 44}, {11, 14, 17, 16, 15, 12,  9, 10}, 13},	// front
	{{51, 52, 53, 29, 28, 27,  6,  7,  8, 15, 16, 17}, {20, 23, 26, 25, 24, 21, 18, 19}, 22},	// right
	{{53, 50, 47, 38, 37, 36,  0,  3,  6, 24, 25, 26}, {29, 32, 35, 34, 33, 30, 27, 28}, 31},	// back
	{{47, 46, 45, 11, 10,  9,  2,  1,  0, 33, 34, 35}, {38, 41, 44, 43, 42, 39, 36, 37}, 40},	// left
	{{35, 32, 29, 26, 23, 20, 17, 14, 11, 44, 41, 38}, {47, 50, 53, 52, 51, 48, 45, 46}, 49}	// bottom
};

// Swap the facelets of the cube in clockwise order.
void RotateCW(const FaceletIndex* Indices, uint8_t NumFacelets)
{
	assert(Indices != 0);
	assert(NumFacelets == NumSideFacelets || NumFacelets == NumFrontFacelets);

	FaceletIndex CurIndex = pgm_read_byte(Indices++);
	Facelet::Type Temp = g_Facelets[CurIndex];
	
	uint8_t Count = NumFacelets - 1;
	do
	{
		FaceletIndex NextIndex = pgm_read_byte(Indices++);
		g_Facelets[CurIndex] = g_Facelets[NextIndex];
		CurIndex = NextIndex;
	} while (--Count);

	g_Facelets[CurIndex] = Temp;
}

// Swap the facelets of the cube in counter-clockwise order.
void RotateCCW(const FaceletIndex* Indices, uint8_t NumFacelets)
{
	assert(Indices != 0);
	assert(NumFacelets == NumSideFacelets || NumFacelets == NumFrontFacelets);

	Indices += NumFacelets;
	FaceletIndex CurIndex = pgm_read_byte(--Indices);
	Facelet::Type Temp = g_Facelets[CurIndex];

	uint8_t Count = NumFacelets - 1;
	do
	{
		FaceletIndex NextIndex = pgm_read_byte(--Indices);
		g_Facelets[CurIndex] = g_Facelets[NextIndex];
		CurIndex = NextIndex;
	} while (--Count);

	g_Facelets[CurIndex] = Temp;
}

}

namespace Cube
{

// Get pointer to 54 facelets, in LED order.
const Facelet::Type* GetFacelets()
{
	return g_Facelets;
}

// Reset cube to solved state.
void Reset()
{
	// Numerical order of colors in Facelet match initialization order.
	Facelet::Type* pFacelet = g_Facelets;
	for (Facelet::Type Color = Facelet::White; Color < Facelet::Unused; ++Color)
		for (uint8_t i = 0; i < NumFaceletsPerFace; ++i)
			*pFacelet++ = Color;
}

// Perform NumRotations random rotations on the cube.
void Scramble(uint8_t NumRotations)
{
	if (NumRotations == 0)
		return;
	Rotation::Type PrevRot = Rand8::Get(0, Rotation::NumRotations - 1);
	Rotate(PrevRot);
	while (--NumRotations)
	{
		Rotation::Type CurRot = Rand8::Get(0, Rotation::NumRotations - 2);
		if (CurRot >= Rotation::Opposite(PrevRot))
			++CurRot;
		Rotate(CurRot);
		PrevRot = CurRot;
	}
}

// Brighten facelets according to a given rotation.
void Brighten(Rotation::Type Face)
{
	STATIC_ASSERT(sizeof(SRotation) == NumAffectedFacelets * sizeof(FaceletIndex),
		      "SRotation is expected to contain NumAffectedFacelets contiguous indices.");
	assert(Face < 2 * Rotation::CCW);

	if (Face >= Rotation::CCW)
		Face -= Rotation::CCW;

	const FaceletIndex* Indices = &Rot[Face].Side[0];
	for (uint8_t i = 0; i < NumAffectedFacelets; ++i)
	{
		FaceletIndex Index = pgm_read_byte(Indices++);
		g_Facelets[Index] |= Facelet::Bright;
	}
}

// Move facelets on the side of a face, one step, according to a given rotation.
void RotateSide(Rotation::Type Face)
{
	assert(Face < 2 * Rotation::CCW);

	if (Face >= Rotation::CCW)
	{
		Face -= Rotation::CCW;
		const FaceletIndex* Indices = &Rot[Face].Side[0];
		RotateCCW(Indices, NumSideFacelets);
	}
	else
	{
		const FaceletIndex* Indices = &Rot[Face].Side[0];
		RotateCW(Indices, NumSideFacelets);
	}
}

// Move facelets on the front of a face, one step, according to a given rotation.
void RotateFront(Rotation::Type Face)
{
	assert(Face < 2 * Rotation::CCW);

	if (Face >= Rotation::CCW)
	{
		Face -= Rotation::CCW;
		const FaceletIndex* Indices = &Rot[Face].Front[0];
		RotateCCW(Indices, NumFrontFacelets);
	}
	else
	{
		const FaceletIndex* Indices = &Rot[Face].Front[0];
		RotateCW(Indices, NumFrontFacelets);
	}
}

// Move all facelets of a face, all steps, according to a given rotation.
void Rotate(Rotation::Type Face)
{
	assert(Face < 2 * Rotation::CCW);
	RotateSide(Face);
	RotateSide(Face);
	RotateSide(Face);
	RotateFront(Face);
	RotateFront(Face);
}

// Dim all facelets of the cube.
void DimAll()
{
	for (FaceletIndex i = 0; i < NumFacelets; ++i)
		g_Facelets[i] &= ~Facelet::Bright;
}

}
