#pragma once

#include <stdint.h>
#include "config.h"

// "enum" representing the state of a facelet via its color.
namespace Facelet
{
typedef uint8_t Type;

const Type Black  = 0;

// Order of colors maps to initialization LED order
#ifdef USE_SIMULATOR
const Type White  = 1;	// At reset: top
const Type Red    = 2;	// At reset: front
const Type Blue   = 3;	// At reset: right
const Type Orange = 4;	// At reset: back
const Type Green  = 5;	// At reset: left
const Type Yellow = 6;	// At reset: bottom
#else
const Type White  = 6;	// At reset: top
const Type Red    = 1;	// At reset: front
const Type Blue   = 2;	// At reset: right
const Type Orange = 4;	// At reset: back
const Type Green  = 5;	// At reset: left
const Type Yellow = 3;	// At reset: bottom
#endif

const Type Unused = 7;

const Type Bright = 8;	// additive bit
}

// Structure holding a 24-bit color in the RGB order, as used by the LEDs.
struct SColor
{
	uint8_t r, g, b;
};

// Color LUT:  Facelet::Type --> SColor
extern const SColor Colors[15];

// "enum" representing the possible rotation operations on the cube.
namespace Rotation
{
typedef uint8_t Type;

// By default, clockwise rotation
const Type Top    = 0;
const Type Front  = 1;
const Type Right  = 2;
const Type Back   = 3;
const Type Left   = 4;
const Type Bottom = 5;
const Type CCW    = 6; // additive, means counter-clockwise

const Type NumRotations = 12;

const Type None   = 255;

inline Type Opposite(Type Face)
{
	return (Face < CCW ? Face + CCW : Face - CCW);
}

inline bool IsRotation(Type Face)
{
	return Face < NumRotations;
}

}

// "class" for manipulating the cube state.
namespace Cube
{
const uint8_t NumFaceletsPerFace = 9;
const uint8_t NumFaces = 6;
const uint8_t NumFacelets = NumFaceletsPerFace * NumFaces;
const uint8_t NumVertices = 8;

const Facelet::Type* GetFacelets();	// Get pointer to 54 facelets, in LED order.
void Reset();				// Reset cube to solved state.
bool IsSolved();			// Returns true if the cube is in the solved state.
void Scramble(uint8_t NumRotations);	// Perform NumRotations random rotations on the cube.

// Brightness-related.
void DimAll();				// Dim all facelets of the cube.
void BrightenFacelet(Facelet::Type FaceletIdx);	// Brighten the given facelet.
void BrightenRandom();			// Set brightness state randomly to all facelets.
void BrightenFace(Rotation::Type Face);	// Brighten facelets according to a given rotation.

// Rotation-related.
void RotateSide(Rotation::Type Face);	// Move facelets on the side of a face, one step, according to a given rotation.
void RotateFront(Rotation::Type Face);	// Move facelets on the front of a face, one step, according to a given rotation.
void Rotate(Rotation::Type Face);	// Move all facelets of a face, all steps, according to a given rotation.

#if DEBUG_CODE
void Backup();				// Save the facelets used by the printing functions
void Restore();				// Restore the facelets used by the printing functions
void PrintUInt8(uint8_t Value);		// Print a uint8_t in binary using one face of the cube.
#endif
}
