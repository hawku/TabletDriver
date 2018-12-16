#pragma once

#include "HIDDevice.h"
#include "Vector2D.h"

class VMulti {
private:
	HIDDevice * hidDevice;
	BYTE reportBuffer[65];
	BYTE lastReportBuffer[65];
public:

	enum VMultiType {
		TypeXPPen,
		TypeVEIKK
	};
	bool isOpen;
	bool debugEnabled;
	bool outputEnabled;
	int type;
	int lastButtons;

	VMulti(int type);
	~VMulti();
	bool HasReportChanged();
	void SetReport(void *buffer, int length);
	int WriteReport();
};


