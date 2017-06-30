#include "cube.h"
#include "config.h"
#include "rand8.h"

STATIC_ASSERT(Facelet::Bright == 8, "This constant must be bitwise-exclusive with the others.");

// Color levels
#ifdef USE_SIMULATOR
	const uint8_t L1 =  64; // stands for Level 1 (dimmest)
	const uint8_t L2 = 128; // stands for Level 2
	const uint8_t L3 = 255; // stands for Level 3 (brightest)
#else
	const uint8_t L1 = 7; // stands for Level 1 (dimmest)
	const uint8_t L2 = 18; // stands for Level 2
	const uint8_t L3 = 56; // stands for Level 3 (brightest)
	const uint8_t L4 = 21; // stands for Level 2
#endif

// Color LUT: index must be a Facelet::Type.
// It must be in SRAM for fast access during LED update.
// Color order is RGB.
#ifdef USE_SIMULATOR
const SColor Colors[15] =
{
	{ 0,  0,  0}, // Black 
	{L2, L2, L2}, // White 
	{L2,  0,  0}, // Red   
	{ 0,  0, L2}, // Blue  
	{L2, L1,  0}, // Orange
	{ 0, L2,  0}, // Green 
	{L2, L2,  0}, // Yellow
	{L2,  0, L2}, // Unused (magenta)
	{ 0,  0,  0}, // Bright Black 
	{L3, L3, L3}, // Bright White 
	{L3,  0,  0}, // Bright Red   
	{ 0,  0, L3}, // Bright Blue  
	{L3, L2,  0}, // Bright Orange
	{ 0, L3,  0}, // Bright Green 
	{L3, L3,  0}  // Bright Yellow
};
#else
const SColor Colors[15] =
{
	{ 0,  0,  0}, // Black 
	{L2,  0,  0}, // Red   
	{ 0,  0, L2}, // Blue  
	{L2, L2,  0}, // Yellow
	{L2, L1,  0}, // Orange
	{ 0, L2,  0}, // Green 
	{L2, L2, L2}, // White 
	{L2,  0, L2}, // Unused (magenta)
	{ 0,  0,  0}, // Bright Black 
	{L3,  0,  0}, // Bright Red   
	{ 0,  0, L3}, // Bright Blue  
	{L3, L3,  0}, // Bright Yellow
	{L3, L4,  0}, // Bright Orange
	{ 0, L3,  0}, // Bright Green 
	{L3, L3, L3}  // Bright White 
};
#endif

// Unnamed namespace for internal details.
namespace 
{
Facelet::Type g_Facelets[Cube::NumFacelets]; // Global cube state.

#if DEBUG_CODE
const uint8_t NumBackupFacelets = 9;
Facelet::Type g_BackupFacelets[NumBackupFacelets]; // Backup of the cube state for debug display.
#endif

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
#ifdef USE_SIMULATOR
const SRotation f_Rot[Cube::NumFaces] PROGMEM =
{
	{{ 9, 12, 15, 18, 21, 24, 27, 30, 33, 36, 39, 42}, { 2,  5,  8,  7,  6,  3,  0,  1},  4},	// top
	{{45, 48, 51, 20, 19, 18,  8,  5,  2, 42, 43, 44}, {11, 14, 17, 16, 15, 12,  9, 10}, 13},	// front
	{{51, 52, 53, 29, 28, 27,  6,  7,  8, 15, 16, 17}, {20, 23, 26, 25, 24, 21, 18, 19}, 22},	// right
	{{53, 50, 47, 38, 37, 36,  0,  3,  6, 24, 25, 26}, {29, 32, 35, 34, 33, 30, 27, 28}, 31},	// back
	{{47, 46, 45, 11, 10,  9,  2,  1,  0, 33, 34, 35}, {38, 41, 44, 43, 42, 39, 36, 37}, 40},	// left
	{{35, 32, 29, 26, 23, 20, 17, 14, 11, 44, 41, 38}, {47, 50, 53, 52, 51, 48, 45, 46}, 49}	// bottom
};
#else
const SRotation f_Rot[Cube::NumFaces] PROGMEM =
{
	{{ 6,  7,  8,  9, 12, 13, 33, 34, 35, 36, 39, 40}, {51, 52, 53, 48, 47, 46, 45, 50}, 49},	// top
	{{24, 23, 18, 17, 10,  9, 53, 52, 51, 40, 41, 42}, { 4,  3,  0,  1,  8,  7,  6,  5},  2},	// front
	{{18, 19, 20, 31, 32, 33, 47, 48, 53,  8,  1,  0}, {17, 16, 15, 14, 13, 12,  9, 10}, 11},	// right
	{{20, 21, 26, 44, 37, 36, 45, 46, 47, 13, 14, 15}, {31, 30, 27, 28, 35, 34, 33, 32}, 29},	// back
	{{26, 25, 24,  4,  5,  6, 51, 50, 45, 35, 28, 27}, {44, 43, 42, 41, 40, 39, 36, 37}, 38},	// left
	{{27, 30, 31, 15, 16, 17,  0,  3,  4, 42, 43, 44}, {26, 21, 20, 19, 18, 23, 24, 25}, 22}	// bottom
};
#endif

#if DEBUG_CODE
#ifdef USE_SIMULATOR
const FaceletIndex g_DebugIndexes[NumBackupFacelets] = {17, 14, 11, 16, 13, 10, 15, 12, 9};
#else
const FaceletIndex g_DebugIndexes[NumBackupFacelets] = {0, 3, 4, 1, 2, 5, 8, 7, 6};
#endif
#endif

// Swap the facelets of the cube in clockwise order.
void RotateCW(const FaceletIndex* f_Indices, uint8_t NumFacelets)
{
	assert(f_Indices != 0);
	assert(NumFacelets == NumSideFacelets || NumFacelets == NumFrontFacelets);

	FaceletIndex CurIndex = pgm_read_byte(f_Indices++);
	Facelet::Type Temp = g_Facelets[CurIndex];
	
	uint8_t Count = NumFacelets - 1;
	do
	{
		FaceletIndex NextIndex = pgm_read_byte(f_Indices++);
		g_Facelets[CurIndex] = g_Facelets[NextIndex];
		CurIndex = NextIndex;
	} while (--Count);

	g_Facelets[CurIndex] = Temp;
}

// Swap the facelets of the cube in counter-clockwise order.
void RotateCCW(const FaceletIndex* f_Indices, uint8_t NumFacelets)
{
	assert(f_Indices != 0);
	assert(NumFacelets == NumSideFacelets || NumFacelets == NumFrontFacelets);

	f_Indices += NumFacelets;
	FaceletIndex CurIndex = pgm_read_byte(--f_Indices);
	Facelet::Type Temp = g_Facelets[CurIndex];

	uint8_t Count = NumFacelets - 1;
	do
	{
		FaceletIndex NextIndex = pgm_read_byte(--f_Indices);
		g_Facelets[CurIndex] = g_Facelets[NextIndex];
		CurIndex = NextIndex;
	} while (--Count);

	g_Facelets[CurIndex] = Temp;
}

// Move facelets on the side of a face, one step, according to a given rotation.
void RotateSide(Rotation::Type Face)
{
	assert(Face < 2 * Rotation::CCW);

	if (Face >= Rotation::CCW)
	{
		Face -= Rotation::CCW;
		const FaceletIndex* f_Indices = &f_Rot[Face].Side[0];
		RotateCCW(f_Indices, NumSideFacelets);
	}
	else
	{
		const FaceletIndex* f_Indices = &f_Rot[Face].Side[0];
		RotateCW(f_Indices, NumSideFacelets);
	}
}

// Move facelets on the front of a face, one step, according to a given rotation.
void RotateFront(Rotation::Type Face)
{
	assert(Face < 2 * Rotation::CCW);

	if (Face >= Rotation::CCW)
	{
		Face -= Rotation::CCW;
		const FaceletIndex* f_Indices = &f_Rot[Face].Front[0];
		RotateCCW(f_Indices, NumFrontFacelets);
	}
	else
	{
		const FaceletIndex* f_Indices = &f_Rot[Face].Front[0];
		RotateCW(f_Indices, NumFrontFacelets);
	}
}

// Animation-related

// On the AVR, delays will be about 2.2 ms longer because of the LEDs update.
#define ROTATION_DELAY_MS		200

typedef uint16_t (*AnimFuncType)();

uint16_t NoAnim();

uint16_t DoRotation();
uint16_t EndRotation();

AnimFuncType   g_AnimFunc = &NoAnim;
uint8_t        g_AnimStepIdx;
Rotation::Type g_AnimRotationFace;

uint16_t NoAnim()
{
	return 0;
}

uint16_t DoRotation()
{
	RotateSide(g_AnimRotationFace);
	if (g_AnimStepIdx != 1)
		RotateFront(g_AnimRotationFace);
	if (++g_AnimStepIdx == 3)
		g_AnimFunc = &EndRotation;
	return ROTATION_DELAY_MS;
}

uint16_t EndRotation()
{
	Cube::DimAll();
	g_AnimFunc = &NoAnim;
	return 0;
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
	for (Facelet::Type Color = 1; Color <= Cube::NumFaces; ++Color)
		for (uint8_t i = 0; i < NumFaceletsPerFace; ++i)
			*pFacelet++ = Color;
}

// Returns true if the cube is in the solved state.
bool IsSolved()
{
	// Numerical order of colors in Facelet match initialization order.
	const Facelet::Type* pFacelet = g_Facelets;
	for (Facelet::Type Color = 1; Color <= Cube::NumFaces; ++Color)
		for (uint8_t i = 0; i < NumFaceletsPerFace; ++i)
			if (*pFacelet++ != Color)
				return false;
	return true;
}

// Perform NumRotations random rotations on the cube.
void Scramble(uint8_t NumRotations)
{
	STATIC_ASSERT(Rotation::Top == 0 && (Rotation::Bottom + Rotation::CCW) == Rotation::NumRotations - 1,
		      "The rotation constants are used as indices below.");

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

// Dim all facelets of the cube.
void DimAll()
{
	for (FaceletIndex i = 0; i < NumFacelets; ++i)
		g_Facelets[i] &= ~Facelet::Bright;
}

// Brighten the given facelet.
void BrightenFacelet(Facelet::Type FaceletIdx)
{
	assert(FaceletIdx < NumFacelets);
	g_Facelets[FaceletIdx] |= Facelet::Bright;
}


// Set brightness state randomly to all facelets.
void BrightenRandom()
{
	DimAll();
	for (uint8_t i = 0; i < 25; ++i)
	{
		FaceletIndex Index = Rand8::Get(0, NumFacelets-1);
		g_Facelets[Index] |= Facelet::Bright;
	}
}

// Brighten facelets according to a given rotation.
void BrightenFace(Rotation::Type Face)
{
	STATIC_ASSERT(sizeof(SRotation) == NumAffectedFacelets * sizeof(FaceletIndex),
		      "SRotation is expected to contain NumAffectedFacelets contiguous indices.");
	assert(Face < 2 * Rotation::CCW);

	if (Face >= Rotation::CCW)
		Face -= Rotation::CCW;

	const FaceletIndex* f_Indices = &f_Rot[Face].Side[0];
	for (uint8_t i = 0; i < NumAffectedFacelets; ++i)
	{
		FaceletIndex Index = pgm_read_byte(f_Indices++);
		g_Facelets[Index] |= Facelet::Bright;
	}
}

// Move all facelets of a face, according to a given rotation.
void Rotate(Rotation::Type Face)
{
	assert(Face < 2 * Rotation::CCW);
	RotateSide(Face);
	RotateSide(Face);
	RotateSide(Face);
	RotateFront(Face);
	RotateFront(Face);
}

#if DEBUG_CODE

// Save the facelets used by the printing functions
void Backup()
{
	for (FaceletIndex i = 0; i < NumBackupFacelets; ++i)
		g_BackupFacelets[i] = g_Facelets[i];
}

// Restore the facelets used by the printing functions
void Restore()
{
	for (FaceletIndex i = 0; i < NumBackupFacelets; ++i)
		g_Facelets[i] = g_BackupFacelets[i];
}

// Print a uint8_t in binary using one face of the cube.
void PrintUInt8(uint8_t Value)
{
	for (FaceletIndex i = 0; i < NumBackupFacelets; ++i)
	{
		g_Facelets[g_DebugIndexes[i]] = ((Value & 1) != 0)
			? Facelet::White
			: Facelet::Black;
		Value >>= 1;
	}
}

#endif

namespace Animation
{

void Rotate(Rotation::Type Face)
{
	Cube::DimAll();
	Cube::BrightenFace(Face);
	g_AnimFunc         = &DoRotation;
	g_AnimStepIdx      = 0;
	g_AnimRotationFace = Face;
}

uint16_t Next()
{
	return (*g_AnimFunc)();
}

}

}
