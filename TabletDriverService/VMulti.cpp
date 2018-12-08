#include "stdafx.h"
#include "VMulti.h"

#define LOG_MODULE "VMulti"
#include "Logger.h"

// Constructor
VMulti::VMulti() {
	isOpen = false;
	debugEnabled = false;
	lastButtons = 0;

	// Report buffers
	memset(reportBuffer, 0, 65);
	memset(lastReportBuffer, 0, 65);

	hidDevice = new HIDDevice(0x00FF, 0xBACC, 0xFF00, 0x0001);
	if(hidDevice->isOpen) {
		isOpen = true;
		outputEnabled = true;

	}
	else {
		delete hidDevice;
		hidDevice = NULL;
	}
}

// Destructor
VMulti::~VMulti() {
	if(hidDevice != NULL)
		delete hidDevice;
}

//
// Has report data changed?
//
bool VMulti::HasReportChanged() {
	if(memcmp(reportBuffer, lastReportBuffer, 65) == 0) {
		return false;
	}
	return true;
}

//
// Set VMulti Report
//
void VMulti::SetReport(void * buffer, int length) {
	memcpy(reportBuffer, buffer, length);
}

//
// Write report
//
int VMulti::WriteReport() {
	if(!outputEnabled) return true;

	// Copy last report
	memcpy(lastReportBuffer, reportBuffer, 65);

	// Debug
	if(logger.debugEnabled) {
		LOG_DEBUGBUFFER(reportBuffer, 12, "Write: ");
	}

	// Write
	return hidDevice->Write(reportBuffer, 65);

}