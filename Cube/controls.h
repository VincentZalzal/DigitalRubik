#pragma once

#include "cube.h"

namespace Controls
{
const uint8_t NumSensors = 24;
extern int8_t g_SensorCounters[Controls::NumSensors];

void Reset();
void UpdateCube(Rotation::Type* pCurRotation, bool* pCubeHasChanged);
}
