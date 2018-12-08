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
	skew = 0;
	dataFormat = TabletFormatNormal;
}

//
// Destructor
//
TabletSettings::~TabletSettings() {
}
