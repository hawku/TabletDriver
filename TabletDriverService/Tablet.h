#pragma once

#include <string>

#include "USBDevice.h"
#include "HIDDevice.h"
#include "TabletSettings.h"
#include "TabletFilterSmoothing.h"
#include "TabletFilterNoiseReduction.h"
#include "TabletFilterPeak.h"
#include "TabletBenchmark.h"
#include "Vector2D.h"

using namespace std;

class Tablet {
public:

	USBDevice * usbDevice;
	HIDDevice * hidDevice;
	HIDDevice * hidDevice2;
	int usbPipeId;

	//
	// Enums
	//
	enum TabletButtons {
		Button1, Button2, Button3, Button4,
		Button5, Button6, Button7, Button8
	};

	// Tablet packet state
	enum TabletPacketState {
		PacketPositionInvalid = 0,
		PacketValid = 1,
		PacketInvalid = 2
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
	// Tablet state
	//
	struct {
		bool isValid;
		BYTE buttons;
		Vector2D position;
		double pressure;
	} state;

	// Settings
	TabletSettings settings;

	// Smoothing filter
	TabletFilterSmoothing smoothing;

	// Noise reduction filter
	TabletFilterNoiseReduction noise;

	// Peak filter
	TabletFilterPeak peak;

	// Timed filter
	TabletFilter *filterTimed[10];
	int filterTimedCount;

	// Packet filter
	TabletFilter *filterPacket[10];
	int filterPacketCount;

	// Benchmark
	TabletBenchmark benchmark;

	// Button map
	BYTE buttonMap[16];

	//
	string name = "Unknown";
	bool isOpen;
	bool debugEnabled;
	int skipPackets;

	// Pen tip button keep down
	int tipDownCounter;

	// Tablet initialize buffers
	BYTE *initFeature;
	int initFeatureLength;
	BYTE *initReport;
	int initReportLength;

	Tablet(string usbGUID, int stringId, string stringMatch);
	Tablet(USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage);
	Tablet();
	~Tablet();

	bool Init();
	bool IsConfigured();

	int ReadPosition();
	bool Write(void *buffer, int length);
	bool Read(void *buffer, int length);
	void CloseDevice();

};
