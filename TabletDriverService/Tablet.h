#pragma once

#include <string>
#include <unordered_map>

#include "USBDevice.h"
#include "HIDDevice.h"
#include "Utils.h"

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
	enum TabletType {
		TabletNormal,
		TypeWacomIntuos
	};
	enum TabletButtons {
		Button1, Button2, Button3, Button4,
		Button5, Button6, Button7, Button8
	};
	enum TabletState {
		PacketPositionInvalid = 0,
		PacketValid = 1,
		PacketInvalid = 2
	};

	//
	// Settings
	//
	struct {
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
	} settings;


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
		double x;
		double y;
		double pressure;
	} state;

	//
	// Filter
	//
	struct {
		HANDLE timer;
		WAITORTIMERCALLBACK callback;
		double interval;
		double latency;
		double weight;
		double threshold;
		bool isEnabled;
		double targetX;
		double targetY;
		double x;
		double y;
	} filter;

	//
	// Benchmark
	//
	struct {
		double minX;
		double maxX;
		double minY;
		double maxY;
		int totalPackets;
		int packetCounter;
	} benchmark;

	// Button map
	BYTE buttonMap[16];
	std::unordered_map<int, std::vector<int>> buttonTabletMap;

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

	double GetFilterLatency(double filterWeight, double interval, double threshold);
	double GetFilterLatency(double filterWeight);
	double GetFilterLatency();
	double GetFilterWeight(double latency, double interval, double threshold);
	double GetFilterWeight(double latency);
	void SetFilterLatency(double latency);
	void ProcessFilter();
	bool StartFilterTimer();
	bool StopFilterTimer();

	void StartBenchmark(int packetCount);
	
	int ReadPosition();
	bool Write(void *buffer, int length);
	bool Read(void *buffer, int length);
	void CloseDevice();

};
