#include "controls.h"

// Unnamed namespace for internal details.
namespace
{
int8_t g_SensorCounters[Controls::NumSensors];
}

namespace Controls
{

void Reset()
{
	for (uint8_t SensorIdx = 0; SensorIdx < NumSensors; ++SensorIdx)
		g_SensorCounters[SensorIdx] = 0;
}

void UpdateCounter(uint8_t SensorIdx, bool SensorIsOn)
{
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

void UpdateCube(Rotation::Type* pCurRotation, bool* pCubeHasChanged)
{
	// Copy current facelets to determine later if any facelet has changed.
	Facelet::Type OldFacelets[Cube::NumFacelets];
	const Facelet::Type* const pFacelets = Cube::GetFacelets();
	for (uint8_t FaceletIdx = 0; FaceletIdx < Cube::NumFacelets; ++FaceletIdx)
		OldFacelets[FaceletIdx] = pFacelets[FaceletIdx];

	// TODO: implement UpdateCube

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

int8_t* GetSensorCounters()
{
	return g_SensorCounters;
}

}
