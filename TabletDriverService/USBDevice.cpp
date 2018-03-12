#include "stdafx.h"
#include "USBDevice.h"

#define LOG_MODULE "USBDevice"
#include "Logger.h"

USBDevice::USBDevice(string Guid, int StringId, string StringMatch) {
	this->guid = Guid;
	this->stringId = StringId;
	this->stringMatch = StringMatch;
	isOpen = false;
	if(this->OpenDevice(guid, stringId, stringMatch)) {
		isOpen = true;
	}
}
USBDevice::~USBDevice() {
	this->CloseDevice();
}


bool USBDevice::OpenDevice(string usbDeviceGUIDString, int stringId, string stringMatch) {
	GUID usbDeviceGUID;

	HDEVINFO                         deviceInfo;
	SP_DEVICE_INTERFACE_DATA         deviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
	SP_DEVINFO_DATA                  deviceInfoData;
	DWORD dwSize;
	DWORD dwMemberIdx;

	HANDLE deviceHandle = 0;
	WINUSB_INTERFACE_HANDLE usbHandle = 0;

	USB_INTERFACE_DESCRIPTOR usbInterfaceDescriptor;

	ULONG readBytes = 0;


	std::wstring stemp = std::wstring(usbDeviceGUIDString.begin(), usbDeviceGUIDString.end());
	LPCWSTR wstringGUID = stemp.c_str();

	_deviceHandle = NULL;
	_usbHandle = NULL;

	HRESULT hr = CLSIDFromString(wstringGUID, (LPCLSID)&usbDeviceGUID);
	if(hr != S_OK) {
		LOG_ERROR("Can't create the USB Device GUID!\n");
		return false;
	}

	// Setup device Info
	deviceInfo = SetupDiGetClassDevs(&usbDeviceGUID, NULL, 0, DIGCF_DEVICEINTERFACE | DIGCF_PRESENT);
	if(deviceInfo == INVALID_HANDLE_VALUE) {
		LOG_ERROR("Device info invalid!\n");
		SetupDiDestroyDeviceInfoList(deviceInfo);
		return false;
	}

	// Enumerate device interface data
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dwMemberIdx = 0;
	SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &usbDeviceGUID, dwMemberIdx, &deviceInterfaceData);
	while(GetLastError() != ERROR_NO_MORE_ITEMS) {

		// Get the required buffer size. 
		deviceInfoData.cbSize = sizeof(deviceInfoData);
		SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, NULL, 0, &dwSize, NULL);

		// Allocate memory
		deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwSize);
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Get device interface detail data
		if(SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, deviceInterfaceDetailData, dwSize, &dwSize, &deviceInfoData)) {

			//LOG_DEBUG("USB Device path: %S\n", deviceInterfaceDetailData->DevicePath);

			// Create File            
			deviceHandle = CreateFile(deviceInterfaceDetailData->DevicePath,
									  GENERIC_READ | GENERIC_WRITE,
									  FILE_SHARE_READ | FILE_SHARE_WRITE,
									  NULL,
									  OPEN_EXISTING,
									  FILE_FLAG_OVERLAPPED,
									  NULL);
			if(!deviceHandle) {
				continue;
			}
			//LOG_DEBUG("Handle: %lu\n", (ULONG)deviceHandle);


			// Init WinUsb
			WinUsb_Initialize(deviceHandle, &usbHandle);
			if(!usbHandle) {
				LOG_ERROR("ERROR! Unable to start WinUSB for the device!\n");
				if(deviceHandle != INVALID_HANDLE_VALUE)
					CloseHandle(deviceHandle);
				return false;
			}
			//LOG_DEBUG("USB Handle: %d\n", (ULONG)usbHandle);

			// Query interface settings
			ZeroMemory(&usbInterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));
			if(WinUsb_QueryInterfaceSettings(usbHandle, 0, &usbInterfaceDescriptor)) {

				WINUSB_SETUP_PACKET setupPacket;
				ULONG bytesRead;
				BYTE buffer[64];
				string str = "";

				setupPacket.RequestType = 0x80;
				setupPacket.Request = 0x06;
				setupPacket.Value = (0x03 << 8) | stringId;
				setupPacket.Index = 0x0409;
				setupPacket.Length = 64;

				// String request match
				if(WinUsb_ControlTransfer(usbHandle, setupPacket, buffer, 64, &bytesRead, NULL)) {

					if(bytesRead >= stringMatch.length() * 2) {

						// Loop through chars
						for(int i = 2; i < (int)bytesRead; i += 2) {
							str.push_back(buffer[i]);
						}

						LOG_DEBUG("USB String (%d): %s\n", stringId, str.c_str());

						// Match!
						if(str.compare(0, stringMatch.size(), stringMatch) == 0) {
							_deviceHandle = deviceHandle;
							_usbHandle = usbHandle;
						}
					}
				}
			} else {
				LOG_ERROR("ERROR! Can't query interface settings!\n");
			}

			if(_usbHandle == NULL && usbHandle && usbHandle != INVALID_HANDLE_VALUE)
				WinUsb_Free(usbHandle);

			if(_deviceHandle == NULL && deviceHandle && deviceHandle != INVALID_HANDLE_VALUE)
				CloseHandle(deviceHandle);


		}

		// Free memory
		std::free(deviceInterfaceDetailData);

		// Continue looping
		SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &usbDeviceGUID, ++dwMemberIdx, &deviceInterfaceData);
	}

	SetupDiDestroyDeviceInfoList(deviceInfo);

	if(_deviceHandle && _deviceHandle != INVALID_HANDLE_VALUE
	   &&
	   _usbHandle && _usbHandle != INVALID_HANDLE_VALUE
	   )
		return true;

	return false;
}


int USBDevice::Read(UCHAR pipeId, void *buffer, int length) {
	ULONG bytesRead;
	try {
		if(WinUsb_ReadPipe(_usbHandle, pipeId, (UCHAR *)buffer, length, &bytesRead, 0)) {
			return (int)bytesRead;
		}
	} catch(exception &e) {
		LOG_ERROR("Exception USB: %s\n", e.what());
	}
	return 0;
}

int USBDevice::Write(UCHAR pipeId, void *buffer, int length) {
	ULONG bytesWritten;
	if(WinUsb_WritePipe(_usbHandle, pipeId, (UCHAR *)buffer, length, &bytesWritten, 0)) {
		return (int)bytesWritten;
	}
	return 0;
}


int USBDevice::ControlTransfer(UCHAR requestType, UCHAR request, USHORT value, USHORT index, void *buffer, USHORT length) {
	WINUSB_SETUP_PACKET usbSetupPacket;
	ULONG bytesRead;
	usbSetupPacket.RequestType = requestType;
	usbSetupPacket.Request = request;
	usbSetupPacket.Length = length;
	usbSetupPacket.Value = value;
	usbSetupPacket.Index = index;
	if(WinUsb_ControlTransfer(_usbHandle, usbSetupPacket, (UCHAR*)buffer, length, &bytesRead, NULL)) {
		return bytesRead;
	}
	return 0;
}

void USBDevice::CloseDevice() {
	if(_usbHandle != NULL && _usbHandle != INVALID_HANDLE_VALUE) {
		try {
			printf("ASD\n");
			WinUsb_Free(_usbHandle);
		} catch(exception &e) {
			printf("WinUsb ERROR! %s\n", e.what());
		}
	}
	if(_deviceHandle != NULL && _deviceHandle != INVALID_HANDLE_VALUE) {
		try {
			printf("DAS\n");
			CloseHandle(_deviceHandle);
		} catch(exception &e) {
			printf("HID ERROR! %s\n", e.what());
		}
	}
	isOpen = false;
}
