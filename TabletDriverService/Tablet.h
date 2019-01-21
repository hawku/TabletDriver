#pragma once

#include <string>
#include <vector>
#include <map>
#include <atomic>
#include <mutex>
#include <condition_variable>

#include "USBDevice.h"
#include "HIDDevice.h"
#include "TabletSettings.h"
#include "TabletState.h"
#include "TabletFilterSmoothing.h"
#include "TabletFilterAdvancedSmoothing.h"
#include "TabletFilterGravity.h"
#include "TabletFilterNoiseReduction.h"
#include "TabletFilterAntiSmoothing.h"
#include "TabletFilterPeak.h"
#include "TabletMeasurement.h"
#include "DataFormatter.h"

class Tablet {
public:
	USBDevice *usbDevice;
	HIDDevice *hidDevice;
	HIDDevice *hidDeviceAux;
	int usbPipeId;

	// Tablet report status
	enum TabletReportStatus {
		ReportPositionInvalid = 0,
		ReportValid = 1,
		ReportInvalid = 2,
		ReportIgnore = 3
	};

	// Tablet auxiliary report status
	enum TabletAuxReportStatus {
		AuxReportValid = 0,
		AuxReportInvalid = 1,
		AuxReportIgnore = 2,
		AuxReportReadError = 3
	};


	//
	// Tablet report data
	//
#pragma pack(1)
	struct {
		BYTE reportId;
		UINT32 buttons;
		UINT32 x;
		UINT32 y;
		USHORT pressure;
		USHORT height;
	} reportData;

	// Tablet report byte names
	const std::map<std::string, int> reportByteNames = {

		// Report id
		{"reportid", 0},

		// Buttons
		{"buttons", 1},
		{"buttonsbyte1", 1},
		{"buttonsbyte2", 2},
		{"buttonsbyte3", 3},
		{"buttonsbyte4", 4},

		// X position
		{"xlow", 5},
		{"xhigh", 6},
		{"xbyte1", 5},
		{"xbyte2", 6},
		{"xbyte3", 7},
		{"xbyte4", 8},

		// Y position
		{"ylow", 9},
		{"yhigh", 10},
		{"ybyte1", 9},
		{"ybyte2", 10},
		{"ybyte3", 11},
		{"ybyte4", 12},

		// Pressure
		{"pressurelow", 13},
		{"pressurehigh", 14},

		// Height
		{"heightlow", 15},
		{"heighthigh", 16}
	};


	//
	// Auxiliary report data
	//
#pragma pack(1)
	struct {
		BYTE reportId;
		UINT32 buttons;
		UCHAR detect;
		UCHAR isPressed;
	} auxReportData;

	// Auxiliary report byte names
	const std::map<std::string, int> auxReportByteNames = {

		// Report id
		{ "reportid", 0 },

		// Buttons
		{ "buttonslow", 1 },
		{ "buttonshigh", 2 },
		{ "buttonsbyte1", 1 },
		{ "buttonsbyte2", 2 },
		{ "buttonsbyte3", 3 },
		{ "buttonsbyte4", 4 },

		// Detect byte
		{ "detect", 5 },

		// Is button pressed byte
		{ "ispressed", 6 }
	};


	class InitReport {
	public:
		BYTE *data = NULL;
		int length = 0;
		InitReport(int length);
		~InitReport();
	};

	class TabletAuxState {
	public:
		bool isValid;
		bool isHandled;
		USHORT buttons;
		USHORT lastButtons;
	};
	TabletAuxState auxState;
	std::mutex lockAuxState;
	std::condition_variable conditionAuxState;


	// Data formatter
	DataFormatter dataFormatter;

	// Tablet State
	TabletState state;

	// Settings
	TabletSettings settings;

	// Filters
	TabletFilterSmoothing smoothing;
	TabletFilterAdvancedSmoothing advancedSmoothing;
	TabletFilterNoiseReduction noiseFilter;
	TabletFilterAntiSmoothing antiSmoothing;
	TabletFilterGravity gravityFilter;

	// Timed filters
	TabletFilter *filterTimed[10];
	int filterTimedCount;

	// Report filters
	TabletFilter *filterReport[10];
	int filterReportCount;

	// Measurement
	TabletMeasurement measurement;

	//
	std::string name = "Unknown";
	std::atomic<bool> isOpen;
	int skipReports;

	// Pen tip button keep down
	int tipDownCounter;

	// Tablet initialization buffers
	std::vector<InitReport*> initFeatureReports;
	std::vector<InitReport*> initOutputReports;
	std::vector<int> initStrings;

	Tablet(std::string usbGUID);
	Tablet(USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage, bool isExclusive);
	Tablet(USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage);
	Tablet();
	~Tablet();

	bool Init();
	bool IsConfigured();

	std::string GetDeviceString(UCHAR stringId);
	std::string GetDeviceManufacturerName();
	std::string GetDeviceProductName();
	std::string GetDeviceSerialNumber();
	int ReadState();
	bool Write(void *buffer, int length);
	bool Read(void *buffer, int length);
	int ProcessAuxData(void *buffer, int length);
	int ReadAuxReport();
	void CloseDevice();

};
