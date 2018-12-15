#pragma once
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
	int clickPressure;
	int keepTipDown;
	double width;
	double height;
	double pressureSensitivity;
	double pressureDeadzone;
	double scrollSensitivity;
	double scrollAcceleration;
	double skew;
	TabletDataFormat dataFormat;
	string buttonMap[16];
	int buttonCount;


	// Aux
	BYTE auxReportId;
	int auxReportLength;
	BYTE auxDetectMask;
	BYTE auxIgnoreMask;
	TabletAuxFormat auxFormat;
	string auxButtonMap[16];
	int auxButtonCount;

	TabletSettings();
	~TabletSettings();
};

