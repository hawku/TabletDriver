#pragma once
class TabletSettings {
public:

	enum TabletType {
		TabletNormal,
		TypeWacomIntuos,
		TypeWacom4100,
		TypeWacomDrivers
	};

	BYTE buttonMask;
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
	TabletType type;

	TabletSettings();
	~TabletSettings();
};

