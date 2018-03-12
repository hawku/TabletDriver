#include "stdafx.h"
#include "HIDDevice.h"

#define LOG_MODULE "HIDDevice"
#include "Logger.h"

HIDDevice::HIDDevice(USHORT VendorId, USHORT ProductId, USHORT UsagePage, USHORT Usage) {
	this->vendorId = VendorId;
	this->productId = ProductId;
	this->usagePage = UsagePage;
	this->usage = Usage;
	isOpen = false;
	_deviceHandle = NULL;
	if(this->OpenDevice(&_deviceHandle, vendorId, productId, usagePage, usage)) {
		isOpen = true;
	}
}
HIDDevice::~HIDDevice() {
	CloseDevice();
}

bool HIDDevice::OpenDevice(HANDLE *handle, USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage) {
	HDEVINFO                         deviceInfo;
	SP_DEVICE_INTERFACE_DATA         deviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
	SP_DEVINFO_DATA                  deviceInfoData;
	DWORD dwSize, dwMemberIdx;
	GUID hidGuid;

	PHIDP_PREPARSED_DATA hidPreparsedData;
	HIDD_ATTRIBUTES hidAttributes;
	HIDP_CAPS hidCapabilities;

	HANDLE deviceHandle;

	HANDLE resultHandle = 0;

	HidD_GetHidGuid(&hidGuid);

	// Setup device info
	deviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if(deviceInfo == INVALID_HANDLE_VALUE) {
		LOG_ERROR("Invalid device info!\n");
		return false;
	}

	// Enum device interface data
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dwMemberIdx = 0;
	SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &hidGuid, dwMemberIdx, &deviceInterfaceData);
	while(GetLastError() != ERROR_NO_MORE_ITEMS) {
		deviceInfoData.cbSize = sizeof(deviceInfoData);

		// Get the required buffer size for device interface detail data
		SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, NULL, 0, &dwSize, NULL);

		// Allocate device interface detail data
		deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwSize);
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Get interface detail
		if(SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, deviceInterfaceDetailData, dwSize, &dwSize, &deviceInfoData)) {

			// Create File            
			deviceHandle = CreateFile(deviceInterfaceDetailData->DevicePath,
									  GENERIC_READ | GENERIC_WRITE,
									  FILE_SHARE_READ | FILE_SHARE_WRITE,
									  NULL,
									  OPEN_EXISTING,
									  0,
									  NULL);

			// HID Attributes
			HidD_GetAttributes(deviceHandle, &hidAttributes);
			// HID Preparsed data
			HidD_GetPreparsedData(deviceHandle, &hidPreparsedData);
			// Capabilities
			HidP_GetCaps(hidPreparsedData, &hidCapabilities);

			if(false && hidAttributes.VendorID == vendorId)
				LOG_DEBUG("VID: %04X PID: %04X UP: %04X U: %04X\n",
						  hidAttributes.VendorID, hidAttributes.ProductID,
						  hidCapabilities.UsagePage, hidCapabilities.Usage);

			   // Set result file if this is the correct device
			if(!resultHandle &&
			   hidAttributes.VendorID == vendorId &&
			   hidAttributes.ProductID == productId &&
			   hidCapabilities.UsagePage == usagePage &&
			   hidCapabilities.Usage == usage
			   ) {
				resultHandle = deviceHandle;

				// Close device
			} else {
				CloseHandle(deviceHandle);
			}
			HidD_FreePreparsedData(hidPreparsedData);
		}

		// Free memory
		delete deviceInterfaceDetailData;


		// Enum next
		SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &hidGuid, ++dwMemberIdx, &deviceInterfaceData);
	}

	// Destroy device info
	SetupDiDestroyDeviceInfoList(deviceInfo);

	// Copy found handle
	if(resultHandle && resultHandle != INVALID_HANDLE_VALUE) {
		memcpy(handle, &resultHandle, sizeof(HANDLE));
		return true;
	}

	return false;
}


int HIDDevice::Read(void *buffer, int length) {
	//return HidD_GetInputReport(_deviceHandle, buffer, length);
	DWORD bytesRead;
	if(ReadFile(_deviceHandle, buffer, length, &bytesRead, 0)) {
		return bytesRead;
	}
	return 0;
}

int HIDDevice::Write(void *buffer, int length) {
	DWORD bytesWritten;
	if(WriteFile(_deviceHandle, buffer, length, &bytesWritten, 0)) {
		return bytesWritten;
	}
	return 0;
}

bool HIDDevice::SetFeature(void *buffer, int length) {
	return HidD_SetFeature(_deviceHandle, buffer, length);
}

void HIDDevice::CloseDevice() {
	if(isOpen && _deviceHandle != NULL && _deviceHandle != INVALID_HANDLE_VALUE) {
		try {
			CloseHandle(_deviceHandle);
		} catch(exception) {}
	}
	isOpen = false;
}
