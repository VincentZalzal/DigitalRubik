#pragma once

#include <cstdint>

namespace Facelet
{
typedef uint8_t Type;

const Type Black  = 0;

// Order of colors maps to initialization LED order
const Type White  = 1;
const Type Red    = 2;
const Type Blue   = 3;
const Type Orange = 4;
const Type Green  = 5;
const Type Yellow = 6;

const Type Unused = 7;

const Type Bright = 8; // additive bit
}

struct SColor
{
	uint8_t g, r, b; // LED protocol order
};

// This color LUT must be in SRAM for fast access during LED update.
// Warning: order is GRB
const SColor Colors[15] =
{
	{  0,   0,   0}, // Black 
	{192, 192, 192}, // White 
	{  0, 192,   0}, // Red   
	{  0,   0, 192}, // Blue  
	{ 96, 192,   0}, // Orange
	{192,   0,   0}, // Green 
	{192, 192,   0}, // Yellow
	{  0, 192, 192}, // Unused (magenta)
	{  0,   0,   0}, // Bright Black 
	{255, 255, 255}, // Bright White 
	{  0, 255,   0}, // Bright Red   
	{  0,   0, 255}, // Bright Blue  
	{128, 255,   0}, // Bright Orange
	{255,   0,   0}, // Bright Green 
	{255, 255,   0}  // Bright Yellow
};

namespace Cube
{
const Facelet::Type* GetFacelets(); // Get pointer to 54 facelets, in LED order
void Reset(); // Reset cube to solved state.
}
