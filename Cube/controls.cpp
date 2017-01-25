#include "controls.h"
#include "config.h"

// Unnamed namespace for internal details.
namespace
{
// Specifies the number of positive sensor read of a rotation configuration
// before performing the rotation. A sensor read takes about 25 ms.
const int8_t RotationDetectionThreshold = 10;	// about 250 ms

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

// Go through all sensor combinations that perform a rotation. If any of them
// reach the given threshold, return the first one (according to the order
// of the Rotation constants). Note: each counter value represents the time
// for one successful sensor read, which takes about 25 ms.
Rotation::Type DetectRotation(int8_t CounterThreshold)
{
	assert(CounterThreshold > 0);

	// TODO: implement DetectRotation
	STATIC_ASSERT(Rotation::Top == 0 && Rotation::Bottom == 5,
		      "The rotation constants are used as indices below.");
	return Rotation::None;
}

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

// Set the brightness of every facelet according to the sensors currently ON
// and the rotation that is about to happen. Returns whether any brightness has
// changed, indicating a LEDs refresh is needed.
bool UpdateCubeBrightness()
{
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

	// Check if there is an active rotation with a very low threshold.
	// This allows to see the corresponding face brighten for a while
	// before the rotation, while still allowing to cancel the movement.
	Rotation::Type Rot = DetectRotation(1);
	if (Rot != Rotation::None)
		Cube::BrightenFace(Rot);

	// Determine if any facelet brightness has changed.
	for (uint8_t FaceletIdx = 0; FaceletIdx < Cube::NumFacelets; ++FaceletIdx)
		if (OldFacelets[FaceletIdx] != pFacelets[FaceletIdx])
			return true;
	return false;
}

// Returns the current action to perform according to the sensors state.
Rotation::Type DetermineAction()
{
	return DetectRotation(RotationDetectionThreshold);
}

}
