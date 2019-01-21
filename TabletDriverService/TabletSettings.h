#pragma once

#include "DataFormatter.h"

#include "InputEmulator.h"

class TabletSettings {
public:

	enum TabletDataFormat {
		TabletFormatNormal,
		TabletFormatWacomIntuosV2,
		TabletFormatWacomIntuosV3,
		TabletFormatWacomDrivers,
		TabletFormatSkipFirstDataByte,
		TabletFormatCustom
	};

	enum TabletAuxFormat {
		AuxFormatCustom
	};

	BYTE reportId;
	int reportLength;

	BYTE detectMask;
	BYTE ignoreMask;

	int maxX;
	int maxY;
	int maxPressure;
	int maxHeight;
	int clickPressure;
	int keepTipDown;
	double width;
	double height;
	double pressureSensitivity;
	double pressureDeadzoneLow;
	double pressureDeadzoneHigh;
	double scrollSensitivity;
	double scrollAcceleration;
	bool scrollStopCursor;
	bool scrollDrag;
	double skew;
	TabletDataFormat dataFormat;
	InputEmulator::InputActionCollection buttonMap[32];
	int buttonCount;


	// Aux
	class AuxReport {
	public:
		BYTE reportId;
		BYTE detectMask;
		BYTE ignoreMask;
		TabletAuxFormat format;
		DataFormatter formatter;
	};
	int auxReportLength;
	AuxReport auxReports[10];
	int auxReportCount;
	AuxReport *auxCurrentReport;
	int auxCurrentReportIndex;
	InputEmulator::InputActionCollection auxButtonMap[32];
	int auxButtonCount;

	TabletSettings();
	~TabletSettings();

	void SetInputEmulator(InputEmulator *emulator);
};

