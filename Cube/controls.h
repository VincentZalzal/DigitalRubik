#pragma once

#include "cube.h"

namespace Action
{
// Import Rotation::Type, Rotation::Top, Rotation::Bottom, etc and Rotation::None.
using namespace Rotation;

const Type ResetEasy   = 254;
const Type ResetNormal = 253;
const Type Undo        = 252;
}

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
Action::Type DetermineAction();

// Manage the action queue for undo operations.
void PushAction(Rotation::Type Rot);
Rotation::Type PopAction();
}
