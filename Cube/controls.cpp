#include "controls.h"
#include "config.h"

// Unnamed namespace for internal details.
namespace
{
// Specifies the number of positive sensor read of an action configuration
// before performing the action. A sensor read takes about 25 ms.
const int8_t DetectionThreshold = 30;	// about 750 ms

// For a given face rotation, there are 8 sensors involved in detecting it.
const uint8_t NumSensorsPerRotation = 8;

// For a given vertex, there are 3 sensors involved in detecting an undo operation.
const uint8_t NumSensorsPerUndo = 3;

// Number of sensor configurations that perform a reset.
const uint8_t NumResetOps = 2;

// For a given reset operation, there are 4 sensors involved in detecting it (a face).
const uint8_t NumSensorsPerReset = 4;

// Sensor counters. A positive value of N means that the sensor has been ON
// for N consecutive Read(), and a negative value of -N means that it has been
// OFF for N consecutive Read().
int8_t g_SensorCounters[Controls::NumSensors];

// Number of entries in the action queue. Maximum number of undo operations.
const uint8_t ActionQueueSize = 16;

// Queue of the last actions taken, most recent first.
Rotation::Type g_ActionQueue[ActionQueueSize];

// This table converts a sensor index into a facelet index.
// This is stored in flash memory and must be accessed using pgm_read_byte().
#ifdef USE_SIMULATOR
const Facelet::Type f_SensorToFacelet[Controls::NumSensors] PROGMEM =
{
	 0,  6,  2,  8,
	 9, 15, 11, 17,
	18, 24, 20, 26,
	27, 33, 29, 35,
	36, 42, 38, 44,
	45, 51, 47, 53
};
#else
const Facelet::Type f_SensorToFacelet[Controls::NumSensors] PROGMEM =
{
	 8,  6,  0,  4,
	17, 15,  9, 13,
	53, 47, 45, 51,
	44, 42, 36, 40,
	26, 20, 24, 18,
	27, 31, 33, 35
};
#endif

// This table gives the array of 8 sensor indices that correspond to a given
// face index. CW and CCW are ignored. f_SensorsPerRotation[i][0] must always
// turn face i CW, and the next sensors are in order.
// This is stored in flash memory and must be accessed using pgm_read_byte().
#ifdef USE_SIMULATOR
const uint8_t f_SensorsPerRotation[Cube::NumFaces][NumSensorsPerRotation] PROGMEM =
{
	{ 4,  5,  8,  9, 12, 13, 16, 17},	// top
	{20, 21, 10,  8,  3,  2, 17, 19},	// front
	{21, 23, 14, 12,  1,  3,  5,  7},	// right
	{23, 22, 18, 16,  0,  1,  9, 11},	// back
	{22, 20,  6,  4,  2,  0, 13, 15},	// left
	{15, 14, 11, 10,  7,  6, 19, 18}	// bottom
};
const uint8_t f_SensorsPerUndo[Cube::NumVertices][NumSensorsPerUndo] PROGMEM =
{
	{ 0, 13, 16},
	{ 1,  9, 12},
	{ 2, 17,  4},
	{ 3,  5,  8},
	{20,  6, 19},
	{21, 10,  7},
	{22, 18, 15},
	{23, 14, 11}
};
const uint8_t f_SensorsPerReset[NumResetOps][NumSensorsPerReset] PROGMEM =
{
	{16, 17, 18, 19},	// Reset, green face
	{ 4,  5,  6,  7}	// Scramble, red face
};
#else
const uint8_t f_SensorsPerRotation[Cube::NumFaces][NumSensorsPerRotation] PROGMEM =
{
	{ 1,  0,  6,  7, 22, 23, 14, 15},	// top
	{18, 19,  4,  6,  8,  9, 15, 13},	// front
	{19, 17, 21, 22, 11,  8,  0,  2},	// right
	{17, 16, 12, 14, 10, 11,  7,  5},	// back
	{16, 18,  3,  1,  9, 10, 23, 20},	// left
	{20, 21,  5,  4,  2,  3, 13, 12}	// bottom
};
const uint8_t f_SensorsPerUndo[Cube::NumVertices][NumSensorsPerUndo] PROGMEM =
{
	{10, 23, 14},
	{11,  7, 22},
	{ 9, 15,  1},
	{ 8,  0,  6},
	{18,  3, 13},
	{19,  4,  2},
	{16, 12, 20},
	{17, 21,  5}
};
const uint8_t f_SensorsPerReset[NumResetOps][NumSensorsPerReset] PROGMEM =
{
	{14, 15, 12, 13},	// Reset, green face
	{ 1,  0,  3,  2}	// Scramble, red face
};
#endif

// Go through all sensor combinations that perform a rotation. If any of them
// reach the given threshold, return the first one (according to the order
// of the Rotation constants). Note: each counter value represents the time
// for one successful sensor read, which takes about 25 ms.
Action::Type DetectRotation(int8_t CounterThreshold)
{
	assert(CounterThreshold > 0);

	STATIC_ASSERT(Action::Top == 0 && Action::Bottom == 5,
		      "The rotation constants are used as indices below.");

	Action::Type DetectedRotation = Action::None;
	for (Action::Type RotIdx = Action::Top; RotIdx <= Action::Bottom; ++RotIdx)
	{
		// Address in Flash memory of the array of 8 sensor indices
		// related to the rotation RotIdx (either CW or CCW).
		const uint8_t* f_SensorIndices = f_SensorsPerRotation[RotIdx];
		
		// Construct a bitfield where each bit, starting from the LSB,
		// indicates whether the corresponding sensor has reached the
		// specified counter threshold.
		STATIC_ASSERT(NumSensorsPerRotation <= 8, "must fit in a uint8_t");
		uint8_t SensorBitfield = 0;

		for (uint8_t i = 0; i < NumSensorsPerRotation; ++i)
		{
			uint8_t SensorIndex = pgm_read_byte(f_SensorIndices++);
			SensorBitfield >>= 1;
			if (g_SensorCounters[SensorIndex] >= CounterThreshold)
				SensorBitfield |= 0x80;
		}

		// The sensors are arranged in the bitfield such that they
		// are in order on the cube. Thus, two bits at distance 4
		// relate to opposing facelets. So, 0x11 and 0x44 indicates
		// that the rotation occurs in the CW direction and 0x22 and
		// 0x88 indicates the CCW direction.
		Action::Type PossibleRotation = Action::None;
		if (SensorBitfield == 0x11 || SensorBitfield == 0x44)
			PossibleRotation = RotIdx;
		else if (SensorBitfield == 0x22 || SensorBitfield == 0x88)
			PossibleRotation = RotIdx + Action::CCW;

		if (PossibleRotation != Action::None)
		{
			// If two rotations are detected, choose neither.
			if (DetectedRotation != Action::None)
				return Action::None;
			DetectedRotation = PossibleRotation;
		}
	}

	return DetectedRotation;
}

// Go through all sensor combinations that perform an undo. If any of them
// reach the given threshold, return Action::Undo. Note: each counter value
// represents the time for one successful sensor read, which takes about 25 ms.
Action::Type DetectUndo(int8_t CounterThreshold)
{
	assert(CounterThreshold > 0);
	
	for (uint8_t VertexIdx = 0; VertexIdx < Cube::NumVertices; ++VertexIdx)
	{
		const uint8_t* f_SensorIndices = f_SensorsPerUndo[VertexIdx];
		uint8_t i = 0;
		for (; i < NumSensorsPerUndo; ++i)
		{
			uint8_t SensorIndex = pgm_read_byte(f_SensorIndices++);
			if (g_SensorCounters[SensorIndex] < CounterThreshold)
				break;
		}
		if (i == NumSensorsPerUndo)
			return Action::Undo;
	}
	
	return Action::None;
}

// Go through all sensor combinations that perform a reset. If any of them
// reach the given threshold, return the reset action. Note: each counter value
// represents the time for one successful sensor read, which takes about 25 ms.
Action::Type DetectReset(int8_t CounterThreshold)
{
	assert(CounterThreshold > 0);
	
	for (uint8_t ResetIdx = 0; ResetIdx < NumResetOps; ++ResetIdx)
	{
		const uint8_t* f_SensorIndices = f_SensorsPerReset[ResetIdx];
		uint8_t i = 0;
		for (; i < NumSensorsPerReset; ++i)
		{
			uint8_t SensorIndex = pgm_read_byte(f_SensorIndices++);
			if (g_SensorCounters[SensorIndex] < CounterThreshold)
				break;
		}
		if (i == NumSensorsPerReset)
		{
			return (ResetIdx == 0 ? Action::Reset : Action::Scramble);
		}
	}
	
	return Action::None;
}

}

namespace Controls
{

// Set all sensor counters to 0 and reset undo actions.
void ResetSensors()
{
	for (uint8_t SensorIdx = 0; SensorIdx < NumSensors; ++SensorIdx)
		g_SensorCounters[SensorIdx] = 0;
}

// Empty the action queue.
void ResetActionQueue()
{
	for (uint8_t QueueIdx = 0; QueueIdx < ActionQueueSize; ++QueueIdx)
		g_ActionQueue[QueueIdx] = Rotation::None;
}

// Increment (or reset) counter of specified sensor according to SensorIsOn
void UpdateCounter(uint8_t SensorIdx, bool SensorIsOn)
{
	assert(SensorIdx < NumSensors);
	int8_t OldValue = g_SensorCounters[SensorIdx];
	int8_t NewValue = OldValue;
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
	Action::Type Rot = DetectRotation(1);
	if (Rotation::IsRotation(Rot))
		Cube::BrightenFace(Rot);

	// Determine if any facelet brightness has changed.
	for (uint8_t FaceletIdx = 0; FaceletIdx < Cube::NumFacelets; ++FaceletIdx)
		if (OldFacelets[FaceletIdx] != pFacelets[FaceletIdx])
			return true;
	return false;
}

// Returns the current action to perform according to the sensors state.
Action::Type DetermineAction()
{
	Action::Type CurAction = DetectReset(DetectionThreshold);
	if (CurAction != Action::None)
		return CurAction;

	CurAction = DetectUndo(DetectionThreshold);
	if (CurAction != Action::None)
		return CurAction;

	return DetectRotation(DetectionThreshold);
}

// Added the given rotation to the front of the queue.
void PushAction(Rotation::Type Rot)
{
	assert(Rotation::IsRotation(Rot));
	for (uint8_t QueueIdx = ActionQueueSize-1; QueueIdx > 0; --QueueIdx)
		g_ActionQueue[QueueIdx] = g_ActionQueue[QueueIdx - 1];
	g_ActionQueue[0] = Rot;
}

// Remove the rotation at the front of the queue. If the queue is empty, returns Rotation::None.
Rotation::Type PopAction()
{
	// Since the queue is always filled with Rotation::None, we don't need
	// a special case to detect an empty queue.
	Rotation::Type Rot = g_ActionQueue[0];
	for (uint8_t QueueIdx = 0; QueueIdx < ActionQueueSize-1; ++QueueIdx)
		g_ActionQueue[QueueIdx] = g_ActionQueue[QueueIdx + 1];
	g_ActionQueue[ActionQueueSize-1] = Rotation::None;
	return Rot;
}

}
