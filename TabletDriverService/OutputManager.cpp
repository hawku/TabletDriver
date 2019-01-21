#include "precompiled.h"
#include "OutputManager.h"

#define LOG_MODULE "OutputManager"
#include "Logger.h"


OutputManager::OutputManager() {
	output = &vmultiAbsolute;
	mode = ModeVMultiAbsolute;

	// Set outputs array
	outputs[ModeVMultiAbsolute] = &vmultiAbsolute;
	outputs[ModeVMultiRelative] = &vmultiRelative;
	outputs[ModeVMultiDigitizer] = &vmultiDigitizer;
	outputs[ModeVMultiDigitizerRelative] = &vmultiDigitizerRelative;
	outputs[ModeSendInputAbsolute] = &sendInputAbsolute;
	outputs[ModeSendInputRelative] = &sendInputRelative;
	outputs[ModeDummy] = &dummy;

	// Use same settings on all outputs
	settings = new OutputSettings();
	for(Output* out : outputs) {
		out->settings = settings;
	}

}


OutputManager::~OutputManager() {
	Reset();
	if(settings != NULL)
		delete settings;
}

void OutputManager::SetOutputMode(OutputMode newMode) {

	// Reset old output
	if(newMode != mode) {
		Reset();
	}
	mode = newMode;

	// Set new output
	output = outputs[newMode];

	// Initialize new output
	output->Init();

}

bool OutputManager::Set(TabletState *tabletState) {
	if(output == NULL) return false;
	return output->Set(tabletState);
}

bool OutputManager::Write() {
	if(output == NULL) return false;
	return output->Write();
}

bool OutputManager::Reset() {
	if(output == NULL) return false;
	return output->Reset();
}

//
// Get relative position
//
bool OutputManager::GetRelativePositionDelta(TabletState * tabletState, Vector2D *delta)
{
	double dx, dy, distance;

	double x = tabletState->position.x;
	double y = tabletState->position.y;

	// Map position to virtual screen (values between 0 and 1)
	mapper->GetRotatedTabletPosition(&x, &y);

	if(settings->relativeState.firstPosition) {
		settings->relativeState.lastPosition.x = x;
		settings->relativeState.lastPosition.y = y;
		settings->relativeState.firstPosition = false;
	}

	// Mouse move delta
	dx = x - settings->relativeState.lastPosition.x;
	dy = y - settings->relativeState.lastPosition.y;
	distance = sqrt(dx * dx + dy * dy);

	//
	// Reset relative state depenging on the reset time (milliseconds)
	//
	if(settings->relativeResetTime > 0) {
		double timeDelta = (tabletState->time - settings->relativeState.lastTime).count() / 1000000.0;
		if(timeDelta > settings->relativeResetTime) {
			settings->ResetRelativeState(x, y, tabletState->time);
			dx = 0;
			dy = 0;
			distance = 0;
		}
	}

	//
	// Reset relative position when the movement distance is long enough
	//
	else if(distance > settings->relativeResetDistance) {
		settings->ResetRelativeState(x, y, tabletState->time);
		dx = 0;
		dy = 0;
	}

	// Sensitivity
	dx *= settings->relativeSensitivity.x;
	dy *= settings->relativeSensitivity.y;

	// Move only when pressure is detected?
	if(settings->relativeDragMove && tabletState->inputPressure <= 0.01) {
		dx = 0;
		dy = 0;
	}


	// Move target position
	settings->relativeState.targetPosition.Add(dx, dy);

	// Set coordinates between current position and the target position
	int relativeX = (int)settings->relativeState.targetPosition.x - settings->relativeState.pixelPosition.x;
	int relativeY = (int)settings->relativeState.targetPosition.y - settings->relativeState.pixelPosition.y;

	// Limit values
	if(relativeX > 127) relativeX = 127;
	else if(relativeX < -127) relativeX = -127;
	if(relativeY > 127) relativeY = 127;
	else if(relativeY < -127) relativeY = -127;

	// Move current position
	settings->relativeState.pixelPosition.x += relativeX;
	settings->relativeState.pixelPosition.y += relativeY;

	// Set last position
	settings->relativeState.lastPosition.Set(x, y);

	// Set last time if movement distance is long enough
	if(distance > 0.1) {
		settings->relativeState.lastTime = tabletState->time;
	}

	// Set relative mouse report output
	delta->Set(relativeX, relativeY);

	return true;
}
