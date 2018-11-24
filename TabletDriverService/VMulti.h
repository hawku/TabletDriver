#pragma once

#include "HIDDevice.h"
#include "Vector2D.h"

class VMulti {
private:
	HIDDevice * hidDevice;
	BYTE reportBuffer[65];
	BYTE lastReportBuffer[65];
public:

	bool isOpen;
	bool debugEnabled;
	bool outputEnabled;
	int lastButtons;

	VMulti();
	~VMulti();
	bool HasReportChanged();
	void SetReport(void *buffer, int length);
	int WriteReport();
};


