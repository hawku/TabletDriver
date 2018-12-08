#include "stdafx.h"
#include "OutputVMultiDigitizer.h"

#define LOG_MODULE "VMultiDigitizer"
#include "Logger.h"


//
// Constructor
//
OutputVMultiDigitizer::OutputVMultiDigitizer() {

	// VMulti digitizer report
	report.vmultiId = 0x40;
	report.reportLength = 8;
	report.reportId = 5;
	report.buttons = 0;
	report.pressure = 0;

}


//
// Destructor
//
OutputVMultiDigitizer::~OutputVMultiDigitizer() {
}


//
// Set output
//
bool OutputVMultiDigitizer::Set(TabletState *tabletState) {

	double x = tabletState->position.x;
	double y = tabletState->position.y;

	// Map position to virtual screen (values between 0 and 1)
	mapper->GetScreenPosition(&x, &y);

	// Offset coordinates by one pixel
	double offsetX = -(32767.0 / mapper->areaVirtualScreen.width);
	double offsetY = -(32767.0 / mapper->areaVirtualScreen.height);

	report.buttons = tabletState->buttons | 0x20;
	report.x = (USHORT)round(x * 32767.0 + offsetX);
	report.y = (USHORT)round(y * 32767.0 + offsetY);
	report.pressure = (USHORT)round(tabletState->pressure * 2047.0);
	vmulti->SetReport(&report, sizeof(report));

	return true;
}


//
// Write output
//
bool OutputVMultiDigitizer::Write() {

	// Write report to VMulti device if report has changed
	if(vmulti->HasReportChanged()) {

		// Debug message
		if(logger.debugEnabled) {
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
bool OutputVMultiDigitizer::Reset() {

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

