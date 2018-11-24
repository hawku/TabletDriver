#include "stdafx.h"
#include "OutputSettings.h"

OutputSettings::OutputSettings() {
	relativeSensitivity = 1;
	relativeResetDistance = 25;
	ResetRelativeState(0, 0);
}


OutputSettings::~OutputSettings() {
}


void OutputSettings::ResetRelativeState(double x, double y) {
	relativeState.targetPosition.Set(x, y);
	relativeState.lastPosition.Set(x, y);
	relativeState.pixelPosition.x = (int)x;
	relativeState.pixelPosition.y = (int)y;

}
