#include "stdafx.h"
#include "VMulti.h"

#define LOG_MODULE "VMulti"
#include "Logger.h"

// Constructor
VMulti::VMulti() {
	isOpen = false;
	debugEnabled = false;

	mode = ModeAbsoluteMouse;

	// Absolute mouse init
	reportAbsoluteMouse.vmultiId = 0x40;
	reportAbsoluteMouse.reportLength = 7;
	reportAbsoluteMouse.reportId = 3;
	reportAbsoluteMouse.buttons = 0;
	reportAbsoluteMouse.wheel = 0;

	// Relative mouse init
	reportRelativeMouse.vmultiId = 0x40;
	reportRelativeMouse.reportLength = 5;
	reportRelativeMouse.reportId = 4;
	reportRelativeMouse.buttons = 0;
	reportRelativeMouse.x = 0;
	reportRelativeMouse.y = 0;
	reportRelativeMouse.wheel = 0;

	// Digitizer / Windows Ink init
	reportDigitizer.vmultiId = 0x40;
	reportDigitizer.reportLength = 8;
	reportDigitizer.reportId = 5;
	reportDigitizer.buttons = 0;
	reportDigitizer.pressure = 0;

	// Relative mouse data
	memset(&relativeData, 0, sizeof(relativeData));
	relativeData.sensitivity = 1;

	// Report buffers
	memset(reportBuffer, 0, 65);
	memset(lastReportBuffer, 0, 65);

	hidDevice = new HIDDevice(0x00FF, 0xBACC, 0xFF00, 0x0001);
	if(hidDevice->isOpen) {
		isOpen = true;
		outputEnabled = true;

	} else {
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


void VMulti::ResetRelativeData(double x, double y) {
	relativeData.targetPosition.x = x;
	relativeData.targetPosition.y = y;
	relativeData.lastPosition.x = x;
	relativeData.lastPosition.y = y;
	relativeData.currentPosition.x = (int)x;
	relativeData.currentPosition.y = (int)y;
}

//
// Create report
//
void VMulti::CreateReport(BYTE buttons, double x, double y, double pressure) {
	double dx, dy, distance;

	//
	// Absolute mouse
	//
	if(mode == VMulti::ModeAbsoluteMouse) {
		reportAbsoluteMouse.buttons = buttons;
		reportAbsoluteMouse.x = (USHORT)round(x * 32767.0);
		reportAbsoluteMouse.y = (USHORT)round(y * 32767.0);

		memcpy(reportBuffer, &reportAbsoluteMouse, sizeof(reportAbsoluteMouse));
		if(debugEnabled) {
			LOG_DEBUGBUFFER(&reportAbsoluteMouse, 9, "VMulti Absolute: ");
		}
	}

	//
	// Relative mouse
	//
	else if(mode == VMulti::ModeRelativeMouse) {

		// Buttons
		reportRelativeMouse.buttons = buttons;

		// Mouse move delta
		dx = x - relativeData.lastPosition.x;
		dy = y - relativeData.lastPosition.y;
		distance = sqrt(dx * dx + dy * dy);

		// Reset position when position jumps 10 millimeters
		if(distance > 10) {
			ResetRelativeData(x, y);
			dx = 0;
			dy = 0;
		}

		// Sensitivity
		dx *= relativeData.sensitivity;
		dy *= relativeData.sensitivity;

		// Move target position
		relativeData.targetPosition.x += dx;
		relativeData.targetPosition.y += dy;

		// Send difference of current position and target position
		reportRelativeMouse.x = (BYTE)(relativeData.targetPosition.x - relativeData.currentPosition.x);
		reportRelativeMouse.y = (BYTE)(relativeData.targetPosition.y - relativeData.currentPosition.y);

		// Add report position to real position
		relativeData.currentPosition.x += reportRelativeMouse.x;
		relativeData.currentPosition.y += reportRelativeMouse.y;

		// Last position
		relativeData.lastPosition.x = x;
		relativeData.lastPosition.y = y;

		memcpy(reportBuffer, &reportRelativeMouse, sizeof(reportRelativeMouse));
		if(debugEnabled) {
			LOG_DEBUGBUFFER(&reportRelativeMouse, 10, "VMulti Relative: ");
		}
	}

	//
	// Digitizer
	//
	else if(mode == VMulti::ModeDigitizer) {
		reportDigitizer.buttons = buttons | 0x20;
		reportDigitizer.x = (USHORT)floor(x * 32767.0 - 5);
		reportDigitizer.y = (USHORT)floor(y * 32767.0 - 5);
		reportDigitizer.pressure = (USHORT)floor(pressure * 2047.0);
		if(reportDigitizer.pressure > 2047) {
			reportDigitizer.pressure = 2047;
		}
		memcpy(reportBuffer, &reportDigitizer, sizeof(reportDigitizer));
		if(debugEnabled) {
			LOG_DEBUGBUFFER(&reportDigitizer, 10, "VMulti Digitizer: ");
		}
	}

}



//
// Send reset report
//
int VMulti::ResetReport() {
	if(!outputEnabled) return true;

	// Absolute
	if(mode == ModeAbsoluteMouse) {
		reportAbsoluteMouse.buttons = 0;
		reportAbsoluteMouse.wheel = 0;
		memcpy(reportBuffer, &reportAbsoluteMouse, sizeof(reportAbsoluteMouse));

	// Relative
	} else if(mode == ModeRelativeMouse) {
		reportRelativeMouse.buttons = 0;
		reportRelativeMouse.x = 0;
		reportRelativeMouse.y = 0;
		reportRelativeMouse.wheel = 0;
		memcpy(reportBuffer, &reportRelativeMouse, sizeof(reportRelativeMouse));

	// Digitizer
	} else if(mode == ModeDigitizer) {
		reportDigitizer.buttons = 0;
		reportDigitizer.pressure = 0;
		memcpy(reportBuffer, &reportDigitizer, sizeof(reportDigitizer));
	}
	return hidDevice->Write(reportBuffer, 65);
}


//
// Write report
//
int VMulti::WriteReport() {
	if(!outputEnabled) return true;

	memcpy(lastReportBuffer, reportBuffer, 65);

	return hidDevice->Write(reportBuffer, 65);
}