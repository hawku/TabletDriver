#pragma once

#include <string>
#include <vector>

#include "USBDevice.h"
#include "HIDDevice.h"
#include "TabletSettings.h"
#include "TabletState.h"
#include "TabletFilterSmoothing.h"
#include "TabletFilterNoiseReduction.h"
#include "TabletFilterAntiSmoothing.h"
#include "TabletFilterPeak.h"
#include "TabletFilterGravity.h"
#include "TabletMeasurement.h"
#include "Vector2D.h"

using namespace std;

class Tablet {
public:
	USBDevice *usbDevice;
	HIDDevice *hidDevice;
	HIDDevice *hidDeviceAux;
	int usbPipeId;

	//
	// Enums
	//
	enum TabletButtons {
		Button1, Button2, Button3, Button4,
		Button5, Button6, Button7, Button8
	};

	// Tablet report state
	enum TabletReportState {
		ReportPositionInvalid = 0,
		ReportValid = 1,
		ReportInvalid = 2,
		ReportIgnore = 3
	};

	//
	// Position report data
	//
#pragma pack(1)
	struct {
		BYTE reportId;
		BYTE buttons;
		USHORT x;
		USHORT y;
		USHORT pressure;
	} reportData;

	//
	// Tablet State
	//
	TabletState state;

	// Settings
	TabletSettings settings;

	// Smoothing filter
	TabletFilterSmoothing smoothing;

	// Noise reduction filter
	TabletFilterNoiseReduction noiseFilter;

	// Anti-smoothing filter
	TabletFilterAntiSmoothing antiSmoothing;

	// Gravity filter
	TabletFilterGravity gravityFilter;

	// Timed filters
	TabletFilter *filterTimed[10];
	int filterTimedCount;

	// Report filters
	TabletFilter *filterReport[10];
	int filterReportCount;

	// Measurement
	TabletMeasurement measurement;

	// Button map
	BYTE buttonMap[16];

	//
	string name = "Unknown";
	bool isOpen;
	int skipReports;

	// Pen tip button keep down
	int tipDownCounter;

	// Tablet initialize buffers
	BYTE *initFeature;
	int initFeatureLength;
	BYTE *initReport;
	int initReportLength;
	vector<int> initStrings;



	Tablet(string usbGUID);
	Tablet(USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage);
	Tablet();
	~Tablet();

	bool Init();
	bool IsConfigured();

	string GetDeviceString(UCHAR stringId);
	int ReadPosition();
	bool Write(void *buffer, int length);
	bool Read(void *buffer, int length);
	void CloseDevice();

};
