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
	pressureDeadzoneLow = 0;
	pressureDeadzoneHigh = 0;
	scrollSensitivity = 1;
	scrollAcceleration = 1;
	scrollStopCursor = false;
	scrollDrag = false;
	skew = 0;
	dataFormat = TabletFormatNormal;
	buttonMap[0] = "MOUSE1";
	buttonMap[1] = "MOUSE2";
	buttonMap[2] = "MOUSE3";
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
