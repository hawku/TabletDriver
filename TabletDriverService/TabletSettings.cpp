#include "stdafx.h"
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
	pressureDeadzone = 0;
	scrollSensitivity = 1;
	scrollAcceleration = 1;
	skew = 0;
	dataFormat = TabletFormatNormal;
	buttonMap[0] = "MOUSE1";
	buttonMap[1] = "MOUSE2";
	buttonMap[2] = "MOUSE3";
	buttonCount = 3;

	// Aux
	auxReportId = 0;
	auxReportLength = 0;
	auxDetectMask = 0;
	auxIgnoreMask = 0;
	auxFormat = AuxFormatCustom;
	auxButtonCount = 0;
}

//
// Destructor
//
TabletSettings::~TabletSettings() {
}
