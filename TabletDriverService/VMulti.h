#pragma once

#include "HIDDevice.h"
#include "Vector2D.h"

class VMulti {
private:
	HIDDevice * hidDevice;
	BYTE reportBuffer[65];
	BYTE lastReportBuffer[65];
public:

	enum VMultiMode {
		ModeAbsoluteMouse,
		ModeRelativeMouse,
		ModeDigitizer,
		ModeSendInput
	};

	struct {
		BYTE vmultiId;
		BYTE reportLength;
		BYTE reportId;
		BYTE buttons;
		USHORT x;
		USHORT y;
		BYTE wheel;
	} reportAbsoluteMouse;

	struct {
		BYTE vmultiId;
		BYTE reportLength;
		BYTE reportId;
		BYTE buttons;
		BYTE x;
		BYTE y;
		BYTE wheel;
	} reportRelativeMouse;

	struct {
		BYTE vmultiId;
		BYTE reportLength;
		BYTE reportId;
		BYTE buttons;
		USHORT x;
		USHORT y;
		USHORT pressure;
	} reportDigitizer;


	// Position Integer
	typedef struct {
		int x;
		int y;
	} PositionInt;

	// Relative mouse data
	struct {
		PositionInt currentPosition;
		Vector2D lastPosition;
		Vector2D targetPosition;
		double sensitivity;
	} relativeData;


	struct {
		double primaryWidth = GetSystemMetrics(SM_CXSCREEN);
		double primaryHeight = GetSystemMetrics(SM_CYSCREEN);
		double virtualWidth = GetSystemMetrics(SM_CXVIRTUALSCREEN);
		double virtualHeight = GetSystemMetrics(SM_CYVIRTUALSCREEN);
		double virtualX = GetSystemMetrics(SM_XVIRTUALSCREEN);
		double virtualY = GetSystemMetrics(SM_YVIRTUALSCREEN);
	} monitorInfo;

	VMultiMode mode;
	bool isOpen;
	bool debugEnabled;
	bool outputEnabled;
	int lastButtons;



	VMulti();
	~VMulti();
	bool HasReportChanged();
	void ResetRelativeData(double x, double y);
	void UpdateMonitorInfo();
	void CreateReport(BYTE buttons, double x, double y, double pressure);
	int ResetReport();
	int WriteReport();
};


