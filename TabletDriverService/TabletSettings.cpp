#include "precompiled.h"
#include "TabletSettings.h"


//
// Constructor
//
TabletSettings::TabletSettings() {
	
	// Initial settings
	reportId = 0;
	reportLength = 8;
	detectMask = 0x00;
	ignoreMask = 0x00;
	maxX = 1;
	maxY = 1;
	maxPressure = 1;
	clickPressure = 0;
	width = 1;
	height = 1;
	pressureSensitivity = 0;
	pressureDeadzoneLow = 0;
	pressureDeadzoneHigh = 0;
	scrollSensitivity = 1;
	scrollAcceleration = 1;
	scrollStopCursor = false;
	scrollDrag = false;
	skew = 0;
	dataFormat = TabletFormatNormal;
	buttonMap[0].Add(new InputEmulator::InputAction(InputEmulator::ActionTypeMouse, "Mouse 1"));
	buttonMap[0].actions[0]->mouseButton = 1;
	buttonMap[1].Add(new InputEmulator::InputAction(InputEmulator::ActionTypeMouse, "Mouse 2"));
	buttonMap[1].actions[0]->mouseButton = 2;
	buttonMap[2].Add(new InputEmulator::InputAction(InputEmulator::ActionTypeMouse, "Mouse 3"));
	buttonMap[2].actions[0]->mouseButton = 3;
	buttonCount = 3;

	// Aux
	for(AuxReport &auxReport : auxReports) {
		auxReport.format = AuxFormatCustom;
	}
	auxCurrentReport = &auxReports[0];
	auxCurrentReportIndex = 0;
	auxReportCount = 1;
	auxButtonCount = 0;
}

//
// Destructor
//
TabletSettings::~TabletSettings() {
}

//
// Set input emulator
//
void TabletSettings::SetInputEmulator(InputEmulator * emulator)
{
	for (InputEmulator::InputActionCollection &collection : buttonMap) {
		collection.emulator = emulator;
	}
	for (InputEmulator::InputActionCollection &collection : auxButtonMap) {
		collection.emulator = emulator;
	}
}
