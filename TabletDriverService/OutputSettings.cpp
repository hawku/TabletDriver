#include "precompiled.h"
#include "OutputSettings.h"

OutputSettings::OutputSettings() {
	relativeState.firstPosition = true;
	relativeSensitivity.Set(1,1);
	relativeResetDistance = 25;
	relativeResetTime = 100;
	relativeDragMove = false;
	ResetRelativeState(0, 0, std::chrono::high_resolution_clock::now());
}


OutputSettings::~OutputSettings() {
}


void OutputSettings::ResetRelativeState(double x, double y, std::chrono::high_resolution_clock::time_point time) {
	relativeState.targetPosition.Set(x, y);
	relativeState.lastPosition.Set(x, y);
	relativeState.pixelPosition.x = (int)x;
	relativeState.pixelPosition.y = (int)y;
	relativeState.lastTime = time;
}
