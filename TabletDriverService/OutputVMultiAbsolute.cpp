#include "precompiled.h"
#include "OutputVMultiAbsolute.h"

#define LOG_MODULE "VMultiAbsolute"
#include "Logger.h"


//
// Constructor
//
OutputVMultiAbsolute::OutputVMultiAbsolute() {

	// Absolute mouse VMulti report
	report.reportLength = 7;
	report.buttons = 0;
	report.x = 0;
	report.y = 0;
	report.wheel = 0;

	// XP-Pen
	if(vmulti->type == VMulti::TypeXPPen) {
		report.vmultiId = 0x40;
		report.reportId = 3;
	}

	// VEIKK
	else if(vmulti->type == VMulti::TypeVEIKK) {
		report.vmultiId = 0x09;
		report.reportId = 1;
	}

}


//
// Destructor
//
OutputVMultiAbsolute::~OutputVMultiAbsolute() {
}


//
// Set output
//
bool OutputVMultiAbsolute::Set(TabletState *tabletState) {

	double x = tabletState->position.x;
	double y = tabletState->position.y;

	// Map position to virtual screen (values between 0 and 1)
	bool mapValid = mapper->GetScreenPosition(&x, &y);
	if(!mapValid) {
		return false;
	}

	report.buttons = tabletState->buttons;
	report.x = (USHORT)round(x * 32767.0);
	report.y = (USHORT)round(y * 32767.0);
	report.wheel = 0;
	vmulti->SetReport(&report, sizeof(report));

	return true;
}

//
// Write output
//
bool OutputVMultiAbsolute::Write() {

	// Write report to VMulti device if report has changed
	if(vmulti->HasReportChanged()) {

		// Debug message
		if(logger.IsDebugOutputEnabled()) {
			LOG_DEBUGBUFFER(&report, 9, "Report: ");
		}

		vmulti->WriteReport();
		return true;
	}
	return false;
}


//
// Reset output
//
bool OutputVMultiAbsolute::Reset() {

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

