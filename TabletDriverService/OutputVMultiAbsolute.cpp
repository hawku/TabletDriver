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
bool OutputVMultiAbsolute::Set(unsigned char buttons, double x, double y, double pressure) {
	report.buttons = buttons;
	report.x = (USHORT)round(x * 32767.0);
	report.y = (USHORT)round(y * 32767.0);
	vmulti->SetReport(&report, sizeof(report));

	if(debugEnabled) {
		LOG_DEBUGBUFFER(&report, 9, "Report: ");
	}

	return true;
}

//
// Write output
//
bool OutputVMultiAbsolute::Write() {

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
bool OutputVMultiAbsolute::Reset() {
	report.buttons = 0;
	report.wheel = 0;
	vmulti->SetReport(&report, sizeof(report));
	vmulti->WriteReport();
	return true;
}

