#include "stdafx.h"
#include "OutputSettings.h"

OutputSettings::OutputSettings() {
	relativeSensitivity = 1;
	relativeResetDistance = 25;
	relativeResetTime = 100;
	ResetRelativeState(0, 0, chrono::high_resolution_clock::now());
}


OutputSettings::~OutputSettings() {
}


void OutputSettings::ResetRelativeState(double x, double y, chrono::high_resolution_clock::time_point time) {
	relativeState.targetPosition.Set(x, y);
	relativeState.lastPosition.Set(x, y);
	relativeState.pixelPosition.x = (int)x;
	relativeState.pixelPosition.y = (int)y;
	relativeState.lastTime = time;
}
