#pragma once

#include "cube.h"

namespace Controls
{
const uint8_t NumSensors = 24;	// Number of facelets used as sensors.

void ResetSensors();		// Set all sensor counters to 0.
void ResetActionQueue();	// Empty the action queue.
void UpdateCounter(uint8_t SensorIdx, bool SensorIsOn);	// Increment (or reset) counter of specified sensor according to SensorIsOn

// Set the brightness of every facelet according to the sensors currently ON
// and the rotation that is about to happen.
bool UpdateCubeBrightness();

// Returns the current action to perform according to the sensors state.
Rotation::Type DetermineAction();
}
