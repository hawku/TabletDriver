#include "precompiled.h"
#include "OutputSendInputAbsolute.h"

#define LOG_MODULE "SendInputAbsolute"
#include "Logger.h"


//
// Constructor
//
OutputSendInputAbsolute::OutputSendInputAbsolute() {
	lastButtons = 0;
	UpdateMonitorInfo();
}


//
// Destructor
//
OutputSendInputAbsolute::~OutputSendInputAbsolute() {
}


//
// Update monitor information
//
void OutputSendInputAbsolute::UpdateMonitorInfo() {
	monitorInfo.primaryWidth = GetSystemMetrics(SM_CXSCREEN);
	monitorInfo.primaryHeight = GetSystemMetrics(SM_CYSCREEN);
	monitorInfo.virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
	monitorInfo.virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
	monitorInfo.virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
	monitorInfo.virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);
}

//
// Initialize
//
void OutputSendInputAbsolute::Init() {
	UpdateMonitorInfo();
}

//
// Set output
//
bool OutputSendInputAbsolute::Set(TabletState *tabletState) {

	double x = tabletState->position.x;
	double y = tabletState->position.y;
	unsigned char buttons = tabletState->buttons;

	// Map position to virtual screen (values between 0 and 1)
	bool mapValid = mapper->GetScreenPosition(&x, &y);
	if(!mapValid) {
		return false;
	}

	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.dx = (LONG)floor(
		x * 65535.0 * (monitorInfo.virtualWidth / monitorInfo.primaryWidth)
		+ (monitorInfo.virtualX / monitorInfo.primaryWidth * 65535.0)
	);
	input.mi.dy = (LONG)floor(
		y * 65535.0 * (monitorInfo.virtualHeight / monitorInfo.primaryHeight)
		+ (monitorInfo.virtualY / monitorInfo.primaryHeight * 65535.0)
	);
	input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE;

	// Mouse 1
	if((buttons & 0x01) && !(lastButtons & 0x01)) input.mi.dwFlags |= MOUSEEVENTF_LEFTDOWN;
	else if(!(buttons & 0x01) && (lastButtons & 0x01)) input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;

	// Mouse 2
	if((buttons & 0x02) && !(lastButtons & 0x02)) input.mi.dwFlags |= MOUSEEVENTF_RIGHTDOWN;
	else if(!(buttons & 0x02) && (lastButtons & 0x02)) input.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;

	// Mouse 3
	if((buttons & 0x04) && !(lastButtons & 0x04)) input.mi.dwFlags |= MOUSEEVENTF_MIDDLEDOWN;
	else if(!(buttons & 0x04) && (lastButtons & 0x04)) input.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;

	lastButtons = buttons;

	return true;
}

//
// Write output
//
bool OutputSendInputAbsolute::Write() {

	// Send input if changed
	if(memcmp(&lastInput, &input, sizeof(INPUT)) != 0) {

		// SendInput
		SendInput(1, &input, sizeof(INPUT));

		// Debug message
		if(logger.IsDebugOutputEnabled()) {
			LOG_DEBUG("%0.0f,%0.0f | %0.0f,%0.0f | %0.0f,%0.0f | %ld, %ld -> %0.0f,%0.0f\n",
				monitorInfo.virtualWidth, monitorInfo.virtualHeight,
				monitorInfo.virtualX, monitorInfo.virtualY,
				monitorInfo.primaryWidth, monitorInfo.primaryHeight,
				input.mi.dx, input.mi.dy,
				input.mi.dx / 65535.0 * (monitorInfo.primaryWidth) - monitorInfo.virtualX,
				input.mi.dy / 65535.0 * (monitorInfo.primaryHeight) - monitorInfo.virtualY
			);
		}

		// Copy last input
		memcpy(&lastInput, &input, sizeof(INPUT));

		return true;
	}
	return false;
}


//
// Reset output
//
bool OutputSendInputAbsolute::Reset() {

	// Do not reset when buttons are not pressed
	if(lastButtons == 0) {
		return true;
	}

	INPUT input = { 0 };
	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.dx = 0;
	input.mi.dy = 0;
	input.mi.dwFlags = 0;
	if(lastButtons & 0x01) input.mi.dwFlags |= MOUSEEVENTF_LEFTUP;
	if(lastButtons & 0x02) input.mi.dwFlags |= MOUSEEVENTF_RIGHTUP;
	if(lastButtons & 0x04) input.mi.dwFlags |= MOUSEEVENTF_MIDDLEUP;

	SendInput(1, &input, sizeof(INPUT));

	return true;
}

