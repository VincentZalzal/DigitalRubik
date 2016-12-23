#pragma once

#include <cstdint>

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

struct SColor
{
	uint8_t g, r, b; // LED protocol order
};

// This color LUT must be in SRAM for fast access during LED update.
// Indices map to the Facelet values.
// Warning: color order is GRB.
const SColor Colors[15] =
{
	{  0,   0,   0}, // Black 
	{128, 128, 128}, // White 
	{  0, 128,   0}, // Red   
	{  0,   0, 128}, // Blue  
	{ 64, 128,   0}, // Orange
	{128,   0,   0}, // Green 
	{128, 128,   0}, // Yellow
	{  0, 128, 128}, // Unused (magenta)
	{  0,   0,   0}, // Bright Black 
	{255, 255, 255}, // Bright White 
	{  0, 255,   0}, // Bright Red   
	{  0,   0, 255}, // Bright Blue  
	{128, 255,   0}, // Bright Orange
	{255,   0,   0}, // Bright Green 
	{255, 255,   0}  // Bright Yellow
};

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
}

namespace Cube
{
const uint8_t NumFaceletsPerFace = 9;
const uint8_t NumFaces = 6;
const uint8_t NumFacelets = NumFaceletsPerFace * NumFaces;

const Facelet::Type* GetFacelets();	// Get pointer to 54 facelets, in LED order
void Reset();				// Reset cube to solved state.
void Brighten(Rotation::Type Face);
void RotateSide(Rotation::Type Face);
void RotateFront(Rotation::Type Face);
void Rotate(Rotation::Type Face);
void DimAll();
}
