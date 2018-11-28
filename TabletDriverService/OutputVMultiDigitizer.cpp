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
bool OutputVMultiDigitizer::Set(unsigned char buttons, double x, double y, double pressure) {
	report.buttons = buttons | 0x20;
	report.x = (USHORT)round(x * 32767.0);
	report.y = (USHORT)round(y * 32767.0);
	report.pressure = (USHORT)round(pressure * 2047.0);
	vmulti->SetReport(&report, sizeof(report));

	if(debugEnabled) {
		LOG_DEBUGBUFFER(&report, 9, "Report: ");
	}

	return true;
}


//
// Write output
//
bool OutputVMultiDigitizer::Write() {

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
bool OutputVMultiDigitizer::Reset() {
	report.buttons = 0;
	report.pressure = 0;
	report.x = 0;
	report.y = 0;
	vmulti->SetReport(&report, sizeof(report));
	vmulti->WriteReport();
	return true;
}

