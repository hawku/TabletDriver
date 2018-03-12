#pragma once

#include "HIDDevice.h"

class VMulti {
private:
	HIDDevice * hidDevice;
	BYTE reportBuffer[65];
	BYTE lastReportBuffer[65];
public:

	enum VMultiMode {
		ModeAbsoluteMouse,
		ModeRelativeMouse,
		ModeDigitizer
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


	// Position Double
	typedef struct {
		double x;
		double y;
	} PositionDouble;

	// Position Integer
	typedef struct {
		int x;
		int y;
	} PositionInt;

	// Relative mouse data
	struct {
		PositionInt currentPosition;
		PositionDouble lastPosition;
		PositionDouble targetPosition;
		double sensitivity;
	} relativeData;

	VMultiMode mode;
	bool isOpen;
	bool debugEnabled;
	bool outputEnabled;



	VMulti();
	~VMulti();
	bool HasReportChanged();
	void ResetRelativeData(double x, double y);
	void CreateReport(BYTE buttons, double x, double y, double pressure);
	int ResetReport();
	int WriteReport();
};


