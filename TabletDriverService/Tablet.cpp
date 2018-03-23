#include "stdafx.h"
#include "Tablet.h"

#define LOG_MODULE "Tablet"
#include "Logger.h"


//
// USB Device Constructor
//
Tablet::Tablet(string usbGUID, int stringId, string stringMatch) : Tablet() {
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
	hidDevice = new HIDDevice(vendorId, productId, usagePage, usage);
	if (hidDevice->isOpen) {
		this->isOpen = true;
	}
	else {
		delete hidDevice;
		hidDevice = NULL;
	}
	hidDevice2 = new HIDDevice(vendorId, productId, usagePage, usage);
	if (hidDevice2->isOpen) {
		this->isOpen = true;
	}
	else {
		delete hidDevice2;
		hidDevice2 = NULL;
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

	// Init reports
	initFeature = NULL;
	initFeatureLength = 0;
	initReport = NULL;
	initReportLength = 0;

	// Reset state
	memset(&state, 0, sizeof(state));

	// Filters
	filterTimed = &smoothing;
	filterPacket = &noise;

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
// Read Position
//
int Tablet::ReadPosition() {
	UCHAR buffer[1024];
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
	if(settings.type == TabletSettings::TypeWacomIntuos) {
		reportData.x = ((buffer[2] * 0x100 + buffer[3]) << 1) | ((buffer[9] >> 1) & 1);
		reportData.y = ((buffer[4] * 0x100 + buffer[5]) << 1) | (buffer[9] & 1);
		reportData.pressure = (buffer[6] << 3) | ((buffer[7] & 0xC0) >> 5) | (buffer[1] & 1);
		reportData.reportId = buffer[0];
		reportData.buttons = buffer[1] & ~0x01;
		//distance = buffer[9] >> 2;

	//
	// Wacom 4100 data format
	//
	} else if(settings.type == TabletSettings::TypeWacom4100) {

		reportData.x = (buffer[2] | (buffer[3] << 8) | (buffer[4] << 16) );
 
		reportData.y = (buffer[5] | (buffer[6] << 8) | (buffer[7] << 16) );

		reportData.pressure = (buffer[8] | (buffer[9] << 8));
		reportData.reportId = buffer[0];
		reportData.buttons = buffer[1] & ~0x01;

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

	// Force tip button down if pressure is detected
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

		// Button is set (not a macro)
		if(buttonMap[buttonIndex] > 0 && buttonMap[buttonIndex] < 6) {

			// Button is pressed
			if((reportData.buttons & (1 << buttonIndex)) > 0) {
				state.buttons |= (1 << (buttonMap[buttonIndex] - 1));
			}
		}
		else if (buttonMap[buttonIndex] == 6)
		{
			if ((reportData.buttons & (1 << buttonIndex)) > 0)
			{
				if (!buttonMacroMap[buttonIndex].second)
					Utils::keyboardShortcutPress(buttonMacroMap[buttonIndex].first);
				buttonMacroMap[buttonIndex].second = true;
			}
			else
				buttonMacroMap[buttonIndex].second = false;
		}
	}

	// Convert report data to state
	state.position.x = ((double)reportData.x / (double)settings.maxX) * settings.width;
	state.position.y = ((double)reportData.y / (double)settings.maxY) * settings.height;
	if(settings.skew != 0) {
		state.position.x += state.position.y * settings.skew;
	}
	state.pressure = ((double)reportData.pressure / (double)settings.maxPressure);

	// Tablet benchmark update
	benchmark.Update(state.position);

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
			hidDevice2->CloseDevice();
		}
	}
	isOpen = false;
}