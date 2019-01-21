#include "precompiled.h"
#include "OutputSendInputRelative.h"

#define LOG_MODULE "SendInputRelative"
#include "Logger.h"


//
// Constructor
//
OutputSendInputRelative::OutputSendInputRelative() {
	lastButtons = 0;
}


//
// Destructor
//
OutputSendInputRelative::~OutputSendInputRelative() {
}


//
// Initialize
//
void OutputSendInputRelative::Init() {
}

//
// Set output
//
bool OutputSendInputRelative::Set(TabletState *tabletState) {

	// Get relative position delta
	Vector2D delta;
	outputManager->GetRelativePositionDelta(tabletState, &delta);


	// Set data
	input.type = INPUT_MOUSE;
	input.mi.mouseData = 0;
	input.mi.dx = (LONG)delta.x;
	input.mi.dy = (LONG)delta.y;
	input.mi.dwFlags = MOUSEEVENTF_MOVE;


	unsigned char buttons = tabletState->buttons;

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
bool OutputSendInputRelative::Write() {

	// Send input if changed
	if(memcmp(&lastInput, &input, sizeof(INPUT)) != 0) {

		// SendInput
		SendInput(1, &input, sizeof(INPUT));

		// Debug message
		if(logger.IsDebugOutputEnabled()) {
			LOG_DEBUG("X: %d, Y: %d, Flags: %d\n",
				input.mi.dx, input.mi.dy, input.mi.dwFlags
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
bool OutputSendInputRelative::Reset() {

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

