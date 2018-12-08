#include "stdafx.h"
#include "OutputVMultiAbsolute.h"

#define LOG_MODULE "VMultiAbsolute"
#include "Logger.h"


//
// Constructor
//
OutputVMultiAbsolute::OutputVMultiAbsolute() {

	// Absolute mouse vmulti report
	report.vmultiId = 0x40;
	report.reportLength = 7;
	report.reportId = 3;
	report.buttons = 0;
	report.wheel = 0;

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
	mapper->GetScreenPosition(&x, &y);

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
		if(logger.debugEnabled) {
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
	report.buttons = 0;
	report.wheel = 0;
	report.x = 0;
	report.y = 0;
	vmulti->SetReport(&report, sizeof(report));
	vmulti->WriteReport();
	return true;
}

