#include "controls.h"

// Unnamed namespace for internal details.
namespace
{

}

namespace Controls
{
int8_t g_SensorCounters[Controls::NumSensors];
	
void Reset()
{
	for (uint8_t SensorIdx = 0; SensorIdx < NumSensors; ++SensorIdx)
		g_SensorCounters[SensorIdx] = 0;
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
