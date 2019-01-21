#include "precompiled.h"
#include "Tablet.h"
#include "TabletHandler.h"

#define LOG_MODULE "Tablet"
#include "Logger.h"


//
// USB Device Constructor
//
Tablet::Tablet(std::string usbGUID) : Tablet() {
	usbDevice = new USBDevice(usbGUID);
	if (usbDevice->isOpen) {
		this->isOpen = true;
		usbPipeId = 0x81;
	}
	else {
		delete usbDevice;
		usbDevice = NULL;
	}
	hidDevice = NULL;
}

//
// HID Device Constructor
//
Tablet::Tablet(USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage, bool isExclusive) : Tablet() {
	hidDevice = new HIDDevice(vendorId, productId, usagePage, usage, isExclusive);
	if (hidDevice->isOpen) {
		this->isOpen = true;
	}
	else {
		delete hidDevice;
		hidDevice = NULL;
	}
	usbDevice = NULL;
}
Tablet::Tablet(USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage) :
	Tablet(vendorId, productId, usagePage, usage, false) {
}

//
// Common constructor
//
Tablet::Tablet() {

	name = "Unknown";
	usbDevice = NULL;
	hidDevice = NULL;
	hidDeviceAux = NULL;

	usbPipeId = 0;

	// Timed filters
	filterTimed[0] = &smoothing;
	filterTimed[1] = &advancedSmoothing;
	filterTimed[2] = &gravityFilter;
	filterTimedCount = 3;

	// Report filters
	filterReport[0] = &antiSmoothing;
	filterReport[1] = &noiseFilter;
	filterReportCount = 2;

	// Tablet connection open
	isOpen = false;

	// Skip first reports, some of those might be invalid.
	skipReports = 5;

	// Keep tip down report counter
	tipDownCounter = 0;
}

//
// Destructor
//
Tablet::~Tablet() {

	isOpen = false;

	// HID device
	if (hidDevice != NULL) {
		printf("  Cleanup HIDDevice\n");
		hidDevice->CloseDevice();
		delete hidDevice;
		hidDevice = NULL;
	}

	// Aux device
	if (hidDeviceAux != NULL) {
		printf("  Cleanup Aux HIDDevice\n");
		hidDeviceAux->CloseDevice();
		delete hidDeviceAux;
		hidDeviceAux = NULL;
	}

	// WinUSB device
	if (usbDevice != NULL) {
		printf("  Cleanup USBDevice\n");
		usbDevice->CloseDevice();
		delete usbDevice;
		usbDevice = NULL;
	}

	// Init feature reports
	for (InitReport *report : initFeatureReports) {
		delete report;
	}

	// Init output reports
	for (InitReport *report : initOutputReports) {
		delete report;
	}

}


//
// Init
//
bool Tablet::Init() {

	// String requests
	if (initStrings.size() > 0) {
		for (int stringId : initStrings) {
			GetDeviceString(stringId);
			Sleep(10);
		}
	}
	Sleep(100);

	// Feature reports
	for (InitReport *report : initFeatureReports) {
		if (!hidDevice->SetFeature(report->data, report->length)) {
			return false;
		}
		Sleep(20);
	}
	Sleep(100);

	// Output reports
	for (InitReport *report : initOutputReports) {
		if (!hidDevice->Write(report->data, report->length)) {
			return false;
		}
		Sleep(20);
	}
	Sleep(100);

	return true;
}


//
// Check if the tablet has enough configuration parameters set
//
bool Tablet::IsConfigured() {
	if (
		settings.maxX > 1 &&
		settings.maxY > 1 &&
		settings.maxPressure > 1 &&
		settings.width > 1 &&
		settings.height > 1
		) return true;
	return false;
}


//
// Get a device string from HID or USB device.
//
std::string Tablet::GetDeviceString(UCHAR stringId)
{
	// USB device
	if (usbDevice != NULL) {
		return usbDevice->GetString(stringId);
	}

	// HID device
	else if (hidDevice != NULL) {
		return hidDevice->GetString(stringId);
	}

	return "";
}

//
// Get HID or WinUSB device manufacturer name
//
std::string Tablet::GetDeviceManufacturerName()
{
	// USB device
	if (usbDevice != NULL) {
		return usbDevice->GetManufacturerName();
	}

	// HID device
	else if (hidDevice != NULL) {
		return hidDevice->GetManufacturerName();
	}

	return "";
}

//
// Get HID or WinUSB device product name
//
std::string Tablet::GetDeviceProductName()
{
	// USB device
	if (usbDevice != NULL) {
		return usbDevice->GetProductName();
	}

	// HID device
	else if (hidDevice != NULL) {
		return hidDevice->GetProductName();
	}

	return "";
}

//
// Get HID or WinUSB device serial number
//
std::string Tablet::GetDeviceSerialNumber()
{
	// USB device
	if (usbDevice != NULL) {
		return usbDevice->GetSerialNumber();
	}

	// HID device
	else if (hidDevice != NULL) {
		return hidDevice->GetSerialNumber();
	}

	return "";
}


//
// Read Position
//
int Tablet::ReadState() {
	UCHAR buffer[1024];
	UCHAR *data;

	// Read report
	if (!this->Read(buffer, settings.reportLength)) {
		return -1;
	}

	if (!isOpen) return Tablet::ReportInvalid;

	// Process auxiliary data
	if (settings.auxReports[0].reportId > 0 && hidDeviceAux == NULL) {
		ProcessAuxData(buffer, settings.reportLength);

		// Notify aux state change
		conditionAuxState.notify_one();
	}

	// Skip reports
	if (skipReports > 0) {
		skipReports--;
		return Tablet::ReportInvalid;
	}

	// Set data pointer
	if (settings.dataFormat == TabletSettings::TabletFormatWacomDrivers) {
		data = buffer + 1;
	}
	else {
		data = buffer;
	}

	//
	// Wacom Intuos data format V2
	//
	if (settings.dataFormat == TabletSettings::TabletFormatWacomIntuosV2) {

		// Wacom driver device
		if (settings.reportLength == 11) {
			data = buffer + 1;
		}

		reportData.reportId = data[0];
		reportData.buttons = data[1] & ~0x01;
		reportData.x = ((data[2] * 0x100 + data[3]) << 1) | ((data[9] >> 1) & 1);
		reportData.y = ((data[4] * 0x100 + data[5]) << 1) | (data[9] & 1);
		reportData.pressure = (data[6] << 3) | ((data[7] & 0xC0) >> 5) | (data[1] & 1);
		reportData.height = data[9] >> 2;


	}

	//
	// Wacom Intuos data format V3 (Wacom 4100)
	//
	else if (settings.dataFormat == TabletSettings::TabletFormatWacomIntuosV3) {

		// Wacom driver device
		if (settings.reportLength == 193) {
			data = buffer + 1;
		}

		reportData.reportId = data[0];
		reportData.buttons = data[1] & ~0x01;
		reportData.x = (data[2] | (data[3] << 8) | (data[4] << 16));
		reportData.y = (data[5] | (data[6] << 8) | (data[7] << 16));
		reportData.pressure = (data[8] | (data[9] << 8));

	}

	//
	// Skip first data byte (VEIKK)
	//
	else if (settings.dataFormat == TabletSettings::TabletFormatSkipFirstDataByte) {

		// Validate report length
		if (settings.reportLength >= 9) {
			reportData.reportId = data[0];
			reportData.buttons = data[2] & ~0x01;
			reportData.x = data[3] | (data[4] << 8);
			reportData.y = data[5] | (data[6] << 8);
			reportData.pressure = data[7] | (data[8] << 8);
		}

	}

	//
	// Custom data format
	//
	else if (settings.dataFormat == TabletSettings::TabletFormatCustom) {

		// Data formatter target length valid?
		if (dataFormatter.targetLength <= sizeof(reportData)) {

			// Clear report data
			memset(&reportData, 0, sizeof(reportData));

			// Format data
			dataFormatter.Format(&reportData, buffer);
		}
	}

	//
	// Standard tablet data format
	//
	else {
		reportData.reportId = data[0];
		reportData.buttons = data[1] & ~0x01;
		reportData.x = data[2] | (data[3] << 8);
		reportData.y = data[4] | (data[5] << 8);
		reportData.pressure = data[6] | (data[7] << 8);

		// Wacom height
		if (settings.reportLength > 8) {
			reportData.height = data[8];
		}
	}


	// Validate report id
	if (settings.reportId > 0 && reportData.reportId != settings.reportId) {
		return Tablet::ReportInvalid;
	}

	// Detect mask
	if (settings.detectMask > 0 && (reportData.buttons & settings.detectMask) != settings.detectMask) {
		return Tablet::ReportPositionInvalid;
	}

	// Ignore mask
	if (settings.ignoreMask > 0 && (reportData.buttons & settings.ignoreMask) == settings.ignoreMask) {
		return Tablet::ReportIgnore;
	}

	//
	// Use pen pressure to detect the pen tip click
	//
	if (settings.clickPressure > 0) {
		reportData.buttons &= ~1;
		if (reportData.pressure > settings.clickPressure) {
			reportData.buttons |= 1;
		}


	}

	// Force tip button down if pressure is detected
	else if (reportData.pressure > 10) {
		reportData.buttons |= 1;
	}

	// Keep pen tip button down for a few reports
	if (settings.keepTipDown > 0) {
		if (reportData.buttons & 0x01) {
			tipDownCounter = settings.keepTipDown;
		}
		if (tipDownCounter-- >= 0) {
			reportData.buttons |= 1;
		}
	}


	// Set valid
	state.isValid = true;


	// Time
	state.time = std::chrono::high_resolution_clock::now();


	// Buttons
	reportData.buttons = reportData.buttons & 0x0F;
	state.buttons = state.inputButtons = reportData.buttons;


	// Convert report data to state
	state.position.x = state.inputPosition.x = ((double)reportData.x / (double)settings.maxX) * settings.width;
	state.position.y = state.inputPosition.y = ((double)reportData.y / (double)settings.maxY) * settings.height;

	state.inputPosition.Set(state.position);

	if (settings.skew != 0) {
		state.position.x += state.position.y * settings.skew;
	}
	state.pressure = state.inputPressure = ((double)reportData.pressure / (double)settings.maxPressure);


	//
	// Pressure deadzone
	//
	if (settings.pressureDeadzoneLow > 0.0 || settings.pressureDeadzoneHigh > 0.0) {

		// Minimum
		if (state.pressure < settings.pressureDeadzoneLow) {
			state.pressure = 0.0;
		}

		// Maximum
		else if (state.pressure > 1 - settings.pressureDeadzoneHigh) {
			state.pressure = 1.0;
		}

		// Between
		else {
			double pressure;
			pressure = state.pressure - settings.pressureDeadzoneLow;
			pressure /= (1 - settings.pressureDeadzoneHigh);
			state.pressure = pressure;
		}
	}

	//
	// Pressure sensitivity 
	//
	if (settings.pressureSensitivity > 0.0) {
		state.pressure = 1 - pow(1 - state.pressure, (1 + settings.pressureSensitivity));
	}
	else if (settings.pressureSensitivity < 0.0) {
		state.pressure = pow(state.pressure, 1 - settings.pressureSensitivity);
	}


	//
	// Height
	//
	state.height = state.inputHeight = reportData.height;


	// Tablet measurement update
	if (measurement.IsRunning()) {
		state.buttons = reportData.buttons & 0x0F;
		measurement.Update(state);
		return Tablet::ReportInvalid;
	}

	// Report and position is valid
	return Tablet::ReportValid;
}


//
// Read report from tablet
//
bool Tablet::Read(void *buffer, int length) {
	if (!isOpen) return false;
	bool status = false;
	if (usbDevice != NULL) {
		status = usbDevice->Read(usbPipeId, buffer, length) > 0;
	}
	else if (hidDevice != NULL) {
		status = hidDevice->Read(buffer, length);
	}
	if (logger.IsDebugOutputEnabled() && status) {
		LOG_DEBUGBUFFER(buffer, length, "Read: ");
	}
	return status;
}

//
// Process auxiliary device data
//
int Tablet::ProcessAuxData(void *buffer, int length)
{
	int status = TabletAuxReportStatus::AuxReportInvalid;
	bool firstReport = true;

	for (int i = 0; i < settings.auxReportCount; i++) {
		TabletSettings::AuxReport *auxReport = &settings.auxReports[i];

		// Clear report data
		memset(&auxReportData, 0, sizeof(auxReportData));
		auxReportData.isPressed = 0x01;

		// Format data
		auxReport->formatter.Format(&auxReportData, buffer);

		// Report id valid?
		if (auxReportData.reportId == auxReport->reportId) {

			// Detect mask
			if (auxReport->detectMask > 0 && (auxReportData.detect & auxReport->detectMask) != auxReport->detectMask) {
				if (status != TabletAuxReportStatus::AuxReportValid)
					status = TabletAuxReportStatus::AuxReportInvalid;
				continue;
			}

			// Ignore mask
			if (auxReport->ignoreMask > 0 && (auxReportData.detect & auxReport->ignoreMask) == auxReport->ignoreMask) {
				if (status != TabletAuxReportStatus::AuxReportValid)
					status = TabletAuxReportStatus::AuxReportIgnore;
				continue;
			}

			// Reset buttons
			if (firstReport) {
				auxState.buttons = 0;
				firstReport = false;
			}

			// Update aux state
			if (auxReportData.isPressed > 0) {
				auxState.buttons |= auxReportData.buttons;
			}
			auxState.isValid = true;

			status = TabletAuxReportStatus::AuxReportValid;
		}
	}

	return status;
}

//
// Read report from auxiliary device
//
int Tablet::ReadAuxReport()
{
	UCHAR buffer[1024];
	int length = settings.auxReportLength;

	// Read aux device
	bool status = false;
	if (hidDeviceAux != NULL) {
		status = hidDeviceAux->Read(buffer, length);

		// Debug message
		if (logger.IsDebugOutputEnabled()) {
			LOG_DEBUGBUFFER(buffer, length, "Aux read: ");
		}

		return ProcessAuxData(buffer, length);

	}
	else if (usbDevice != NULL || settings.auxReports[0].reportId > 0) {

		// Wait for an auxiliary state change
		std::unique_lock<std::mutex> mlock(lockAuxState);
		conditionAuxState.wait(mlock);

		if (auxState.isValid) {
			return TabletAuxReportStatus::AuxReportValid;
		}
		else {
			return TabletAuxReportStatus::AuxReportInvalid;
		}
	}
	return TabletAuxReportStatus::AuxReportReadError;

}

//
// Write report to the tablet
//
bool Tablet::Write(void *buffer, int length) {
	if (!isOpen) return false;
	if (usbDevice != NULL) {
		return usbDevice->Write(usbPipeId, buffer, length) > 0;
	}
	else if (hidDevice != NULL) {
		return hidDevice->Write(buffer, length);
	}
	return false;
}

//
// Close tablet
//
void Tablet::CloseDevice() {
	if (isOpen) {
		if (usbDevice != NULL) {
			printf("  Tablet: Closing WinUSB device\n");
			usbDevice->CloseDevice();
		}
		if (hidDevice != NULL) {
			printf("  Tablet: Closing HID device\n");
			hidDevice->CloseDevice();
		}
		if (hidDeviceAux != NULL) {
			printf("  Tablet: Closing Aux HID device\n");
			hidDeviceAux->CloseDevice();
		}
	}
	isOpen = false;
}

//
// Init report constructor
//
Tablet::InitReport::InitReport(int length)
{
	this->data = new BYTE[length];
	this->length = length;
}

//
// Init report destructor
//
Tablet::InitReport::~InitReport()
{
	delete this->data;
}
