#include "precompiled.h"
#include "OutputVMultiRelative.h"

#define LOG_MODULE "VMultiRelative"
#include "Logger.h"


//
// Constructor
//
OutputVMultiRelative::OutputVMultiRelative() {

	// Relative mouse vmulti report
	report.reportLength = 5;
	report.buttons = 0;
	report.x = 0;
	report.y = 0;
	report.wheel = 0;

	// XP-Pen
	if(vmulti->type == VMulti::TypeXPPen) {
		report.vmultiId = 0x40;
		report.reportId = 4;
	}

	// VEIKK
	else if(vmulti->type == VMulti::TypeVEIKK) {
		report.vmultiId = 0x09;
		report.reportId = 3;
	}

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

	// Buttons
	report.buttons = tabletState->buttons;

	Vector2D delta;
	outputManager->GetRelativePositionDelta(tabletState, &delta);

	// Set relative mouse report output
	report.x = (char)delta.x;
	report.y = (char)delta.y;

	vmulti->SetReport(&report, sizeof(report));

	return true;
}

//
// Write output
//
bool OutputVMultiRelative::Write() {

	// Write report to VMulti device if report has changed
	if(vmulti->HasReportChanged() || report.x != 0 || report.y != 0) {
		if(logger.IsDebugOutputEnabled()) {
			LOG_DEBUGBUFFER(&report, 10, "Report: ");
		}
		vmulti->WriteReport();
		return true;
	}
	return false;
}


//
// Reset output
//
bool OutputVMultiRelative::Reset() {

	// Do not reset when buttons are not pressed
	if(report.buttons == 0) {
		return true;
	}

	report.buttons = 0;
	report.wheel = 0;
	vmulti->SetReport(&report, sizeof(report));
	vmulti->WriteReport();
	return true;
}

