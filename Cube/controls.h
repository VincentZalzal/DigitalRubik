#pragma once

#include "cube.h"

namespace Controls
{
const uint8_t NumSensors = 24;	// Number of facelets used as sensors.

void Reset();	// Set all sensor counters to 0.
void UpdateCounter(uint8_t SensorIdx, bool SensorIsOn);	// Increment (or reset) counter of specified sensor according to SensorIsOn
// Set the brightness of every facelets according to the sensors currently ON
// and the rotation that is about to happen.
void UpdateCubeBrightnessAndDetermineAction(Rotation::Type* pCurRotation, bool* pCubeHasChanged);
}
