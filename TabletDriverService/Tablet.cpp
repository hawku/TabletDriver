#include "stdafx.h"
#include "Tablet.h"

#define LOG_MODULE "Tablet"
#include "Logger.h"


//
// USB Device Constructor
//
Tablet::Tablet(string usbGUID, int stringId, string stringMatch) : Tablet() {
	//_construct();
	usbDevice = new USBDevice(usbGUID, stringId, stringMatch);
	if(usbDevice->isOpen) {
		this->isOpen = true;
		usbPipeId = 0x81;
	} else {
		delete usbDevice;
		usbDevice = NULL;
	}
}

//
// HID Device Constructor
//
Tablet::Tablet(USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage) : Tablet() {
	//_construct();
	hidDevice = new HIDDevice(vendorId, productId, usagePage, usage);
	if(hidDevice->isOpen) {
		this->isOpen = true;
	} else {
		delete hidDevice;
		hidDevice = NULL;
	}
}

//
// Common constructor
//
Tablet::Tablet() {

	name = "Unknown";
	usbDevice = NULL;
	hidDevice = NULL;
	hidDevice2 = NULL;

	usbPipeId = 0;

	// Initial settings
	settings.reportId = 0;
	settings.reportLength = 8;
	settings.buttonMask = 0x00;
	settings.maxX = 1;
	settings.maxY = 1;
	settings.maxPressure = 1;
	settings.clickPressure = 0;
	settings.width = 1;
	settings.height = 1;
	settings.skew = 0;
	settings.type = TabletNormal;

	// Initial filter settings
	filter.timer = NULL;
	filter.interval = 2.0;
	filter.latency = 2.0;
	filter.weight = 1.000;
	filter.threshold = 0.9;
	filter.antichatterType = 2;
	filter.antichatterRange = 0.15;
	filter.antichatterStrength = 3;
	filter.antichatterOffset = 0.0;
	filter.isEnabled = false;
	filter.targetX = 0;
	filter.targetY = 0;
	filter.x = 0;
	filter.y = 0;

	// Init reports
	initFeature = NULL;
	initFeatureLength = 0;
	initReport = NULL;
	initReportLength = 0;

	// Reset state
	memset(&state, 0, sizeof(state));

	// Reset benchmark
	memset(&benchmark, 0, sizeof(benchmark));

	// Button map
	memset(&buttonMap, 0, sizeof(buttonMap));
	buttonMap[0] = 1;
	buttonMap[1] = 2;
	buttonMap[2] = 3;

	// Tablet connection open
	isOpen = false;

	// Debug output
	debugEnabled = false;

	// Skip first packets, some of those might be invalid.
	skipPackets = 5;

	// Keep tip down packet counter
	tipDownCounter = 0;

}

//
// Destructor
//
Tablet::~Tablet() {
	CloseDevice();
	if(usbDevice != NULL)
		delete usbDevice;
	if(hidDevice != NULL)
		delete hidDevice;
	if(hidDevice2 != NULL)
		delete hidDevice2;
	if(initReport != NULL)
		delete initReport;
	if(initFeature != NULL)
		delete initFeature;
}


//
// Init
//
bool Tablet::Init() {

	// Feature report
	if(initFeature != NULL) {
		if(hidDevice->SetFeature(initFeature, initFeatureLength)) {
			return true;
		}
		return false;
	}

	// Output report
	if(initReport != NULL) {
		if(hidDevice->Write(initReport, initReportLength)) {
			return true;
		}
		return false;
	}

	// USB (Huion)
	if(usbDevice != NULL) {
		BYTE buffer[64];
		if(usbDevice->ControlTransfer(0x80, 0x06, (0x03 << 8) | 100, 0x0409, buffer, 64) > 0) {
			return true;
		}
		return false;
	}


	return true;
}


//
// Check if the tablet has enough configuration parameters set
//
bool Tablet::IsConfigured() {
	if(
		settings.maxX > 1 &&
		settings.maxY > 1 &&
		settings.maxPressure > 1 &&
		settings.width > 1 &&
		settings.height > 1
	) return true;
	return false;
}


//
// Calculate filter latency
//
double Tablet::GetFilterLatency(double filterWeight, double interval, double threshold) {
	double target = 1 - threshold;
	double stepCount = -log(1 / target) / log(1 - filterWeight);
	return stepCount * interval;
}
double Tablet::GetFilterLatency(double filterWeight) {
	return this->GetFilterLatency(filterWeight, filter.interval, filter.threshold);
}
double Tablet::GetFilterLatency() {
	return this->GetFilterLatency(filter.weight, filter.interval, filter.threshold);
}


//
// Calculate filter weight
//
double Tablet::GetFilterWeight(double latency, double interval, double threshold) {
	double stepCount = latency / interval;
	double target = 1 - threshold;
	return 1 - 1 / pow(1 / target, 1 / stepCount);
}
double Tablet::GetFilterWeight(double latency) {
	return this->GetFilterWeight(latency, filter.interval, filter.threshold);
}

// Set filter values
void Tablet::SetFilterLatency(double latency) {
	tablet->filter.weight = tablet->GetFilterWeight(latency);
	tablet->filter.latency = latency;
}

//
// Process filter
//
void Tablet::ProcessFilter() {

	double deltaX, deltaY, distance, weightModifier, antichatterRange, antichatterStrength,antichatterOffset;
	int antichatterType;

	deltaX = filter.targetX - filter.x;
	deltaY = filter.targetY - filter.y;
	distance = sqrt(deltaX*deltaX + deltaY * deltaY);
	antichatterType = (int) filter.antichatterType;
	antichatterRange = filter.antichatterRange;
	antichatterStrength = filter.antichatterStrength;
	antichatterOffset = filter.antichatterOffset;

	// Regular smoothing
	if (antichatterType == 0 or distance > antichatterRange) {
		filter.x += deltaX * filter.weight;
		filter.y += deltaY * filter.weight;
	}
	// Strong smoothing on small distances to avoid tablet noise
	//(it needs up to 0.3 near borders in some places and/or when pen is very high, but ~0.15 is enough in most cases)
	else {
		if (antichatterType == 1)
		{
			weightModifier = 1 / antichatterStrength;
			if (weightModifier > 1) weightModifier = 1;
			filter.x += deltaX * (filter.weight * weightModifier);
			filter.y += deltaY * (filter.weight * weightModifier);
		} else
		if (antichatterType == 2) {
			weightModifier = pow(distance, antichatterStrength*-1) + antichatterOffset;
			if (weightModifier < 1) weightModifier = 1;
			filter.x += deltaX * (filter.weight / weightModifier);
			filter.y += deltaY * (filter.weight / weightModifier);
		}
	}
}


//
// Start Filter Timer
//
bool Tablet::StartFilterTimer() {
	return CreateTimerQueueTimer(
		&filter.timer,
		NULL, filter.callback,
		NULL,
		0,
		(int)filter.interval,
		WT_EXECUTEDEFAULT
	);
}


//
// Stop Filter Timer
//
bool Tablet::StopFilterTimer() {
	if(tablet->filter.timer == NULL) return false;
	bool result = DeleteTimerQueueTimer(NULL, filter.timer, NULL);
	if(result) {
		filter.timer = NULL;
	}
	return result;
}


//
// Start tablet benchmark
//
void Tablet::StartBenchmark(int packetCount) {
	tablet->benchmark.maxX = -10000;
	tablet->benchmark.maxY = -10000;
	tablet->benchmark.minX = 10000;
	tablet->benchmark.minY = 10000;
	tablet->benchmark.totalPackets = packetCount;
	tablet->benchmark.packetCounter = packetCount;
}



//
// Read Position
//
int Tablet::ReadPosition() {
	UCHAR buffer[256];
	int buttonIndex;


	// Read report
	if(!this->Read(buffer, settings.reportLength)) {
		return -1;
	}

	// Skip packets
	if(skipPackets > 0) {
		skipPackets--;
		return Tablet::PacketInvalid;
	}

	// Validate packet id
	if(settings.reportId > 0 && buffer[0] != settings.reportId) {
		return Tablet::PacketInvalid;
	}

	//
	// Wacom Intuos data format
	//
	if(settings.type == TypeWacomIntuos) {
		reportData.x = ((buffer[2] * 0x100 + buffer[3]) << 1) | ((buffer[9] >> 1) & 1);
		reportData.y = ((buffer[4] * 0x100 + buffer[5]) << 1) | (buffer[9] & 1);
		reportData.pressure = (buffer[6] << 3) | ((buffer[7] & 0xC0) >> 5) | (buffer[1] & 1);
		reportData.reportId = buffer[0];
		reportData.buttons = buffer[1] & ~0x01;
		//distance = buffer[9] >> 2;

	//
	// Copy buffer to struct
	//
	} else {
		memcpy(&reportData, buffer, sizeof(reportData));
	}


	// Validate position
	if(settings.buttonMask > 0 && (reportData.buttons & settings.buttonMask) != settings.buttonMask) {
		return Tablet::PacketPositionInvalid;
	}

	//
	// Use pen pressure to detect the pen tip click
	//
	if(settings.clickPressure > 0) {
		reportData.buttons &= ~1;
		if(reportData.pressure > settings.clickPressure) {
			reportData.buttons |= 1;
		}

	// Force tip button if pressure is detected
	} else if(reportData.pressure > 10) {
		reportData.buttons |= 1;
	}

	// Keep pen tip button down for a few packets
	if(settings.keepTipDown > 0) {
		if(reportData.buttons & 0x01) {
			tipDownCounter = settings.keepTipDown;
		}
		if(tipDownCounter-- >= 0) {
			reportData.buttons |= 1;
		}
	}




	// Set valid
	state.isValid = true;

	// Button map
	reportData.buttons = reportData.buttons & 0x0F;
	state.buttons = 0;
	for(buttonIndex = 0; buttonIndex < sizeof(buttonMap); buttonIndex++) {

		// Button is set
		if(buttonMap[buttonIndex] > 0) {

			// Button is pressed
			if((reportData.buttons & (1 << buttonIndex)) > 0) {
				state.buttons |= (1 << (buttonMap[buttonIndex] - 1));
			}
		}
	}

	// Convert report data to state
	state.x = ((double)reportData.x / (double)settings.maxX) * settings.width;
	state.y = ((double)reportData.y / (double)settings.maxY) * settings.height;
	if(settings.skew != 0) {
		state.x += state.y * settings.skew;
	}
	state.pressure = ((double)reportData.pressure / (double)settings.maxPressure);

	//
	// Tablet benchmark
	//
	if(benchmark.packetCounter > 0) {

		// Set min & max
		if(state.x < benchmark.minX) benchmark.minX = state.x;
		if(state.x > benchmark.maxX) benchmark.maxX = state.x;
		if(state.y < benchmark.minY) benchmark.minY = state.y;
		if(state.y > benchmark.maxY) benchmark.maxY = state.y;

		benchmark.packetCounter--;
	}

	// Packet and position is valid
	return Tablet::PacketValid;
}


//
// Read report from tablet
//
bool Tablet::Read(void *buffer, int length) {
	if(!isOpen) return false;
	bool status = false;
	if(usbDevice != NULL) {
		status = usbDevice->Read(usbPipeId, buffer, length) > 0;
	} else if(hidDevice != NULL) {
		status = hidDevice->Read(buffer, length);
	}
	if(debugEnabled && status) {
		LOG_DEBUGBUFFER(buffer, length, "Read: ");
	}
	return status;
}

//
// Write report to the tablet
//
bool Tablet::Write(void *buffer, int length) {
	if(!isOpen) return false;
	if(usbDevice != NULL) {
		return usbDevice->Write(usbPipeId, buffer, length) > 0;
	} else if(hidDevice != NULL) {
		return hidDevice->Write(buffer, length);
	}
	return false;
}

//
// Close tablet
//
void Tablet::CloseDevice() {
	if(isOpen) {
		if(usbDevice != NULL) {
			usbDevice->CloseDevice();
		} else if(hidDevice != NULL) {
			hidDevice->CloseDevice();
		}
	}
	isOpen = false;
}