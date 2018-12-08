#include "stdafx.h"
#include "OutputVMultiAbsoluteV2.h"

#define LOG_MODULE "VMultiAbsoluteV2"
#include "Logger.h"


//
// Constructor
//
OutputVMultiAbsoluteV2::OutputVMultiAbsoluteV2() {

	// Absolute mouse vmulti report
	report.vmultiId = 0x40;
	report.reportLength = 8;
	report.reportId = 9;
	report.buttons = 0;
	report.pressure = 0;

}


//
// Destructor
//
OutputVMultiAbsoluteV2::~OutputVMultiAbsoluteV2() {
}


//
// Set output
//
bool OutputVMultiAbsoluteV2::Set(TabletState *tabletState) {

	double x = tabletState->position.x;
	double y = tabletState->position.y;

	// Map position to virtual screen (values between 0 and 1)
	mapper->GetScreenPosition(&x, &y);

	report.buttons = tabletState->buttons;
	report.x = (USHORT)round(x * 32767.0);
	report.y = (USHORT)round(y * 32767.0);
	//report.pressure = (USHORT)round(tabletState->pressure * 8191.0);
	vmulti->SetReport(&report, sizeof(report));

	if(logger.debugEnabled) {
		LOG_DEBUGBUFFER(&report, 10, "Report: ");
	}

	return true;
}

//
// Write output
//
bool OutputVMultiAbsoluteV2::Write() {

	// Write report to VMulti device if report has changed
	if(vmulti->HasReportChanged()) {
		vmulti->WriteReport();
		return true;
	}
	return false;
}


//
// Reset output
//
bool OutputVMultiAbsoluteV2::Reset() {


	// Do not reset when buttons are not pressed
	if(report.buttons == 0) {
		return true;
	}

	report.buttons = 0;
	report.pressure = 0;
	vmulti->SetReport(&report, sizeof(report));
	vmulti->WriteReport();
	return true;
}

