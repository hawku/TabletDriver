#include "stdafx.h"
#include "OutputVMultiRelative.h"

#define LOG_MODULE "VMultiRelative"
#include "Logger.h"

//
// Constructor
//
OutputVMultiRelative::OutputVMultiRelative() {

	// Relative mouse vmulti report
	report.vmultiId = 0x40;
	report.reportLength = 5;
	report.reportId = 4;
	report.buttons = 0;
	report.x = 0;
	report.y = 0;
	report.wheel = 0;

	firstReport = true;
}

//
// Destructor
//
OutputVMultiRelative::~OutputVMultiRelative() {
}




//
// Set output
//
bool OutputVMultiRelative::Set(TabletState *tabletState) {

	double dx, dy, distance;

	double x = tabletState->position.x;
	double y = tabletState->position.y;

	// Map position to virtual screen (values between 0 and 1)
	mapper->GetRotatedTabletPosition(&x, &y);

	if(firstReport) {
		settings->relativeState.lastPosition.x = x;
		settings->relativeState.lastPosition.y = y;
		firstReport = false;
	}

	// Buttons
	report.buttons = tabletState->buttons;

	// Mouse move delta
	dx = x - settings->relativeState.lastPosition.x;
	dy = y - settings->relativeState.lastPosition.y;
	distance = sqrt(dx * dx + dy * dy);

	// Reset relative position when the move distance is long enough
	if(distance > settings->relativeResetDistance) {
		settings->ResetRelativeState(x, y);
		dx = 0;
		dy = 0;
	}

	// Sensitivity
	dx *= settings->relativeSensitivity;
	dy *= settings->relativeSensitivity;

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

	// Last position
	settings->relativeState.lastPosition.Set(x, y);

	// Set relative mouse report output
	report.x = (char)relativeX;
	report.y = (char)relativeY;

	vmulti->SetReport(&report, sizeof(report));
	if(logger.debugEnabled) {
		LOG_DEBUGBUFFER(&report, 10, "Report: ");
	}
	return true;
}

//
// Write output
//
bool OutputVMultiRelative::Write() {

	// Write report to VMulti device if report has changed
	if(vmulti->HasReportChanged() || report.x != 0 || report.y != 0) {
		vmulti->WriteReport();
		return true;
	}
	return false;
}


//
// Reset output
//
bool OutputVMultiRelative::Reset() {
	report.buttons = 0;
	report.wheel = 0;
	vmulti->SetReport(&report, sizeof(report));
	vmulti->WriteReport();
	return true;
}

