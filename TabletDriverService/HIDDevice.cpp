#include "precompiled.h"
#include "HIDDevice.h"

#define LOG_MODULE "HIDDevice"
#include "Logger.h"

HIDDevice::HIDDevice(USHORT VendorId, USHORT ProductId, USHORT UsagePage, USHORT Usage, bool IsExclusive) {
	this->vendorId = VendorId;
	this->productId = ProductId;
	this->usagePage = UsagePage;
	this->usage = Usage;
	this->isExclusive = IsExclusive;
	if (this->OpenDevice(&this->_deviceHandle, this->vendorId, this->productId, this->usagePage, this->usage, this->isExclusive)) {
		isOpen = true;
	}
	isReading = false;
}

HIDDevice::HIDDevice(USHORT VendorId, USHORT ProductId, USHORT UsagePage, USHORT Usage) : HIDDevice(VendorId, ProductId, UsagePage, Usage, false) {
}

HIDDevice::HIDDevice() {
	isOpen = false;
	isReading = false;
	_manufacturerName = "";
	_productName = "";
	_serialNumber = "";
	_deviceHandle = NULL;
}

HIDDevice::~HIDDevice() {
	CloseDevice();
}


bool GetHIDStrings(HANDLE deviceHandle, std::string *manufacturerName, std::string *productName, std::string *serialNumber) {
	BYTE buffer[1024];
	int i;

	// HID manufacturer string
	memset(buffer, 0, sizeof(buffer));
	if (HidD_GetManufacturerString(deviceHandle, &buffer, sizeof(buffer))) {
		for (i = 0; i < (int)sizeof(buffer); i += 2) {
			if (buffer[i]) manufacturerName->push_back(buffer[i]);
			else break;
		}
	}

	// HID product string
	memset(buffer, 0, sizeof(buffer));
	if (HidD_GetProductString(deviceHandle, &buffer, sizeof(buffer))) {
		for (i = 0; i < (int)sizeof(buffer); i += 2) {
			if (buffer[i]) productName->push_back(buffer[i]);
			else break;
		}
	}

	// HID serial number
	memset(buffer, 0, sizeof(buffer));
	if (HidD_GetSerialNumberString(deviceHandle, &buffer, sizeof(buffer))) {
		for (i = 0; i < (int)sizeof(buffer); i += 2) {
			if (buffer[i]) serialNumber->push_back(buffer[i]);
			else break;
		}
	}

	return true;
}


//
// Open device
//
bool HIDDevice::OpenDevice(HANDLE *handle, USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage, bool exclusive) {
	HDEVINFO                         deviceInfo;
	SP_DEVICE_INTERFACE_DATA         deviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
	SP_DEVINFO_DATA                  deviceInfoData;
	DWORD dwSize, dwMemberIdx;
	GUID hidGuid;

	PHIDP_PREPARSED_DATA hidPreparsedData;
	HIDD_ATTRIBUTES hidAttributes;
	HIDP_CAPS hidCapabilities;
	std::string manufacturerName;
	std::string productName;
	std::string serialNumber;

	HANDLE deviceHandle;
	HANDLE resultHandle = 0;

	HidD_GetHidGuid(&hidGuid);

	// Setup device info
	deviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if (deviceInfo == INVALID_HANDLE_VALUE) {
		LOG_ERROR("Invalid device info!\n");
		return false;
	}

	// Enumerate device interface data
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dwMemberIdx = 0;
	SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &hidGuid, dwMemberIdx, &deviceInterfaceData);
	while (GetLastError() != ERROR_NO_MORE_ITEMS) {
		deviceInfoData.cbSize = sizeof(deviceInfoData);

		// Get the required buffer size for device interface detail data
		SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, NULL, 0, &dwSize, NULL);

		// Allocate device interface detail data
		deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwSize);
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Get interface detail
		if (SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, deviceInterfaceDetailData, dwSize, &dwSize, &deviceInfoData)) {

			// Open HID in exclusive mode
			if (exclusive) {
				deviceHandle = CreateFile(
					deviceInterfaceDetailData->DevicePath,
					GENERIC_READ | GENERIC_WRITE,
					0, // No sharing
					NULL,
					OPEN_EXISTING,
					0,
					NULL);
			}

			// Open HID in sharing mode
			else {
				deviceHandle = CreateFile(
					deviceInterfaceDetailData->DevicePath,
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					0,
					NULL);
			}

			// HID handle valid?
			if (deviceHandle != INVALID_HANDLE_VALUE) {

				// HID Attributes
				HidD_GetAttributes(deviceHandle, &hidAttributes);

				// HID Preparsed data
				HidD_GetPreparsedData(deviceHandle, &hidPreparsedData);

				// HID Capabilities
				HidP_GetCaps(hidPreparsedData, &hidCapabilities);

				// Debug logging
				if (this->debugEnabled) {

					manufacturerName = "";
					productName = "";
					serialNumber = "";

					GetHIDStrings(deviceHandle, &manufacturerName, &productName, &serialNumber);

					LOG_DEBUG("HID Device: Vendor: '%s' Product: '%s', Serial: '%s'\n",
						manufacturerName.c_str(),
						productName.c_str(),
						serialNumber.c_str()
					);
					LOG_DEBUG("  Vendor Id: 0x%04X, Product Id: 0x%04X\n",
						hidAttributes.VendorID,
						hidAttributes.ProductID
					);
					LOG_DEBUG("  Usage Page: 0x%04X, Usage: 0x%04X\n",
						hidCapabilities.UsagePage,
						hidCapabilities.Usage
					);
					LOG_DEBUG("  FeatureLen: %d, InputLen: %d, OutputLen: %d\n",
						hidCapabilities.FeatureReportByteLength,
						hidCapabilities.InputReportByteLength,
						hidCapabilities.OutputReportByteLength
					);
					LOG_DEBUG("\n");
				}

				// Set the result handle if this is the correct device
				if (!resultHandle &&
					hidAttributes.VendorID == vendorId &&
					hidAttributes.ProductID == productId &&
					hidCapabilities.UsagePage == usagePage &&
					hidCapabilities.Usage == usage
					) {
					GetHIDStrings(deviceHandle, &_manufacturerName, &_productName, &_serialNumber);

					// Set timeouts
					COMMTIMEOUTS commTimeOuts;
					GetCommTimeouts(deviceHandle, &commTimeOuts);
					commTimeOuts.ReadIntervalTimeout = 1;
					commTimeOuts.ReadTotalTimeoutConstant = 1;
					commTimeOuts.ReadTotalTimeoutMultiplier = 1;
					SetCommTimeouts(deviceHandle, &commTimeOuts);

					_devicePathW = deviceInterfaceDetailData->DevicePath;
					std::string str(_devicePathW.begin(), _devicePathW.end());
					_devicePath = str;

					resultHandle = deviceHandle;
				}

				// Close the HID handle if the device is incorrect
				else {
					SAFE_CLOSE_HANDLE(deviceHandle);
				}

				// Free HID preparsed data
				HidD_FreePreparsedData(hidPreparsedData);
			}
		}

		// Free memory
		delete deviceInterfaceDetailData;

		// Get next interface data
		SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &hidGuid, ++dwMemberIdx, &deviceInterfaceData);
	}

	// Destroy device info
	SetupDiDestroyDeviceInfoList(deviceInfo);

	// Copy found handle
	if (resultHandle && resultHandle != INVALID_HANDLE_VALUE) {
		memcpy(handle, &resultHandle, sizeof(HANDLE));
		return true;
	}

	return false;
}

//
// Open device
//
bool HIDDevice::OpenDevice(HANDLE * handle, USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage)
{
	return OpenDevice(handle, vendorId, productId, usagePage, usage, false);
}

// Read HID report
int HIDDevice::Read(void *buffer, int length) {
	//return HidD_GetInputReport(_deviceHandle, buffer, length);
	DWORD bytesRead;
	isReading = true;
	if (ReadFile(_deviceHandle, buffer, length, &bytesRead, 0)) {
		return bytesRead;
	}
	return 0;
}

// Write HID report
int HIDDevice::Write(void *buffer, int length) {
	DWORD bytesWritten;
	if (WriteFile(_deviceHandle, buffer, length, &bytesWritten, 0)) {
		return bytesWritten;
	}
	return 0;
}

// Set feature report
bool HIDDevice::SetFeature(void *buffer, int length) {
	return HidD_SetFeature(_deviceHandle, buffer, length);
}

// Get feature report
bool HIDDevice::GetFeature(void *buffer, int length) {
	return HidD_GetFeature(_deviceHandle, buffer, length);
}

// String request
int HIDDevice::StringRequest(UCHAR stringId, UCHAR * buffer, int length)
{
	if (HidD_GetIndexedString(_deviceHandle, stringId, buffer, length)) {
		int realLength = length;
		for (int i = 0; i < length; i += 2) {
			if (buffer[i] == 0) {
				realLength = i;
				break;
			}
		}
		return realLength;
	}
	return 0;
}

std::string HIDDevice::GetString(UCHAR stringId)
{
	std::string resultString = "";
	UCHAR buffer[256];
	int bytesRead = 0;

	if (isReading) {
		throw std::runtime_error("HID string request can't be sent when the device is in use!");
	}
	else {
		bytesRead = StringRequest(stringId, buffer, 256);
	}

	// Reply received?
	if (bytesRead > 0) {
		for (int i = 0; i < bytesRead; i += 2) {
			resultString.push_back(buffer[i]);
		}
	}

	return resultString;
}

// Get HID manufacturer name
std::string HIDDevice::GetManufacturerName()
{
	return _manufacturerName;
}

// Get HID product name
std::string HIDDevice::GetProductName()
{
	return _productName;
}

// Get HID serial number
std::string HIDDevice::GetSerialNumber()
{
	return _serialNumber;
}

// Close the device
void HIDDevice::CloseDevice() {
	if (isOpen && _deviceHandle != NULL && _deviceHandle != INVALID_HANDLE_VALUE) {
		try {

			// Cancel IO
			CancelIoEx(_deviceHandle, NULL);

			/*
			// Workaround for hanging CloseHandle
			printf("  HID closing fix\n");
			HANDLE tmpHandle = CreateFile(
					_devicePathW.c_str(),
					GENERIC_READ | GENERIC_WRITE,
					FILE_SHARE_READ | FILE_SHARE_WRITE,
					NULL,
					OPEN_EXISTING,
					0,
					NULL);

			SAFE_CLOSE_HANDLE(tmpHandle);
			printf("HID closing fix done!\n");
			*/

			SAFE_CLOSE_HANDLE(_deviceHandle);

		}
		catch (std::exception) {
			SAFE_CLOSE_HANDLE(_deviceHandle);
		}
	}
	isOpen = false;
}
