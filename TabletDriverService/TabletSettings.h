#pragma once
class TabletSettings {
public:

	enum TabletDataFormat {
		TabletFormatNormal,
		TabletFormatWacomIntuosV2,
		TabletFormatWacomIntuosV3,
		TabletFormatWacomDrivers,
		TabletFormatSkipFirstDataByte
	};

	BYTE detectMask;
	BYTE ignoreMask;
	int maxX;
	int maxY;
	int maxPressure;
	int clickPressure;
	int keepTipDown;
	double width;
	double height;
	BYTE reportId;
	int reportLength;
	double skew;
	TabletDataFormat dataFormat;

	TabletSettings();
	~TabletSettings();
};

