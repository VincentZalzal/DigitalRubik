#pragma once

#include "cube.h"

namespace Controls
{
const uint8_t NumSensors = 24;

void Reset();
void UpdateCounter(uint8_t SensorIdx, bool SensorIsOn);
void UpdateCube(Rotation::Type* pCurRotation, bool* pCubeHasChanged);
}
