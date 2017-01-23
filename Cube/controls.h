#pragma once

#include "cube.h"

namespace Controls
{
void Reset();
void UpdateCube(Rotation::Type* pCurRotation, bool* pCubeHasChanged);
// TODO: add function to get button counters
}
