#include "controls.h"
#include "config.h"

// Unnamed namespace for internal details.
namespace
{
// Sensor counters. A positive value of N means that the sensor has been ON
// for N consecutive Read(), and a negative value of -N means that it has been
// OFF for N consecutive Read().
int8_t g_SensorCounters[Controls::NumSensors];

// This table converts a sensor index into a facelet index.
// This is stored in flash memory and must be accessed using pgm_read_byte().
const Facelet::Type f_SensorToFacelet[Controls::NumSensors] PROGMEM =
{ // TODO: fill f_SensorToFacelet with useful values
	 0,  1,  2,  3,
	 4,  5,  6,  7,
	 8,  9, 10, 11,
	12, 13, 14, 15
};

}

namespace Controls
{

// Set all sensor counters to 0.
void Reset()
{
	for (uint8_t SensorIdx = 0; SensorIdx < NumSensors; ++SensorIdx)
		g_SensorCounters[SensorIdx] = 0;
}

// Increment (or reset) counter of specified sensor according to SensorIsOn
void UpdateCounter(uint8_t SensorIdx, bool SensorIsOn)
{
	assert(SensorIdx < NumSensors);
	uint8_t OldValue = g_SensorCounters[SensorIdx];
	uint8_t NewValue = OldValue;
	if (SensorIsOn)
	{
		if (OldValue <= 0)
			NewValue = 1;
		else if (OldValue < 127)
			NewValue = OldValue + 1;
	}
	else
	{
		if (OldValue >= 0)
			NewValue = -1;
		else if (OldValue > -128)
			NewValue = OldValue - 1;
	}
	g_SensorCounters[SensorIdx] = NewValue;
}

// Set the brightness of every facelets according to the sensors currently ON
// and the rotation that is about to happen. Returns the rotation that should
// be applied in pCurRotation and whether any brightness has changed (whether
// LEDs should be refreshed) in pCubeHasChanged.
void UpdateCubeBrightnessAndDetermineAction(
	Rotation::Type* pCurRotation, bool* pCubeHasChanged)
{
	assert(pCurRotation != 0);
	assert(pCubeHasChanged != 0);

	// Copy current facelets to determine later if any facelet has changed.
	Facelet::Type OldFacelets[Cube::NumFacelets];
	const Facelet::Type* const pFacelets = Cube::GetFacelets();
	for (uint8_t FaceletIdx = 0; FaceletIdx < Cube::NumFacelets; ++FaceletIdx)
		OldFacelets[FaceletIdx] = pFacelets[FaceletIdx];

	Cube::DimAll();

	// Brighten all facelets whose sensor is currently ON.
	for (uint8_t SensorIdx = 0; SensorIdx < NumSensors; ++SensorIdx)
	{
		if (g_SensorCounters[SensorIdx] > 0)
		{
			Facelet::Type FaceletIdx = pgm_read_byte(&f_SensorToFacelet[SensorIdx]);
			Cube::BrightenFacelet(FaceletIdx);
		}
	}

	// TODO: implement UpdateCubeBrightnessAndDetermineAction
	STATIC_ASSERT(Rotation::Top == 0 && Rotation::Bottom == 5,
		      "The rotation constants are used as indices below.");

	// Determine if any facelet has changed.
	*pCubeHasChanged = false;
	for (uint8_t FaceletIdx = 0; FaceletIdx < Cube::NumFacelets; ++FaceletIdx)
	{
		if (OldFacelets[FaceletIdx] != pFacelets[FaceletIdx])
		{
			*pCubeHasChanged = true;
			break;
		}
	}
}

}
