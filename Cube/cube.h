#pragma once

#include <stdint.h>

// "enum" representing the state of a facelet via its color.
namespace Facelet
{
typedef uint8_t Type;

const Type Black  = 0;

// Order of colors maps to initialization LED order
const Type White  = 1;	// At reset: top
const Type Red    = 2;	// At reset: front
const Type Blue   = 3;	// At reset: right
const Type Orange = 4;	// At reset: back
const Type Green  = 5;	// At reset: left
const Type Yellow = 6;	// At reset: bottom

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

inline Type Opposite(Type Face)
{
	return (Face < CCW ? Face + CCW : Face - CCW);
}

}

// "class" for manipulating the cube state.
namespace Cube
{
const uint8_t NumFaceletsPerFace = 9;
const uint8_t NumFaces = 6;
const uint8_t NumFacelets = NumFaceletsPerFace * NumFaces;

const Facelet::Type* GetFacelets();	// Get pointer to 54 facelets, in LED order.
void Reset();				// Reset cube to solved state.
bool IsSolved();			// Returns true if the cube is in the solved state.
void Scramble(uint8_t NumRotations);	// Perform NumRotations random rotations on the cube.
void BrightenRandom();			// Set brightness state randomly to all facelets.
void Brighten(Rotation::Type Face);	// Brighten facelets according to a given rotation.
void RotateSide(Rotation::Type Face);	// Move facelets on the side of a face, one step, according to a given rotation.
void RotateFront(Rotation::Type Face);	// Move facelets on the front of a face, one step, according to a given rotation.
void Rotate(Rotation::Type Face);	// Move all facelets of a face, all steps, according to a given rotation.
void DimAll();				// Dim all facelets of the cube.
}
