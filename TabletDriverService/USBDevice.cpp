#include "stdafx.h"
#include "USBDevice.h"

#define LOG_MODULE "USBDevice"
#include "Logger.h"

USBDevice::USBDevice(string Guid) {
	this->guid = Guid;
	isOpen = false;
	if(this->OpenDevice(&_deviceHandle, &_usbHandle, guid)) {
		isOpen = true;
	}
}
USBDevice::~USBDevice() {
	this->CloseDevice();
}


bool USBDevice::OpenDevice(HANDLE *outDeviceHandle, WINUSB_INTERFACE_HANDLE *outUSBHandle, string usbDeviceGUIDString) {
	GUID usbDeviceGUID;

	HDEVINFO                         deviceInfo;
	SP_DEVICE_INTERFACE_DATA         deviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
	SP_DEVINFO_DATA                  deviceInfoData;
	DWORD dwSize;
	DWORD dwMemberIdx;

	HANDLE deviceHandle = 0;
	WINUSB_INTERFACE_HANDLE usbHandle = 0;

	bool deviceFound = false;

	USB_INTERFACE_DESCRIPTOR usbInterfaceDescriptor;

	ULONG readBytes = 0;




	std::wstring stemp = std::wstring(usbDeviceGUIDString.begin(), usbDeviceGUIDString.end());
	LPCWSTR wstringGUID = stemp.c_str();

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

			if(deviceHandle != INVALID_HANDLE_VALUE) {
				//LOG_DEBUG("Handle: %lu\n", (ULONG)deviceHandle);

				// Init WinUsb
				WinUsb_Initialize(deviceHandle, &usbHandle);
				if(!usbHandle) {
					LOG_ERROR("ERROR! Unable to initialize WinUSB!\n");
					if(deviceHandle != INVALID_HANDLE_VALUE)
						CloseHandle(deviceHandle);
					return false;
				}
				//LOG_DEBUG("USB Handle: %d\n", (ULONG)usbHandle);

				// Query interface settings
				ZeroMemory(&usbInterfaceDescriptor, sizeof(USB_INTERFACE_DESCRIPTOR));

				// Can query interface settings?
				if(WinUsb_QueryInterfaceSettings(usbHandle, 0, &usbInterfaceDescriptor)) {

					memcpy(outDeviceHandle, &deviceHandle, sizeof(HANDLE));
					memcpy(outUSBHandle, &usbHandle, sizeof(WINUSB_INTERFACE_HANDLE));
					deviceFound = true;
				}
				else {
					if(usbHandle && usbHandle != INVALID_HANDLE_VALUE)
						WinUsb_Free(usbHandle);

					if(deviceHandle && deviceHandle != INVALID_HANDLE_VALUE)
						CloseHandle(deviceHandle);
				}
			}
		}

		// Free memory
		std::free(deviceInterfaceDetailData);

		// Continue looping
		SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &usbDeviceGUID, ++dwMemberIdx, &deviceInterfaceData);
	}

	SetupDiDestroyDeviceInfoList(deviceInfo);

	return deviceFound;
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
	ULONG bytesRead = 0;
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

//
// USB string request
//
int USBDevice::StringRequest(UCHAR stringId, UCHAR *buffer, int length)
{
	UCHAR *tmpBuffer;
	int bytesRead = 0;
	tmpBuffer = (UCHAR*)malloc(length);

	bytesRead = ControlTransfer(0x80, 0x06, (0x03 << 8) | stringId, 0x0409, tmpBuffer, length);

	// Offset copy to remove extra bytes
	if(bytesRead > 3) {
		memcpy(buffer, (tmpBuffer + 2), bytesRead - 2);
		bytesRead -= 2;
	}
	else {
		bytesRead = 0;
	}
	delete tmpBuffer;

	return bytesRead;
}

void USBDevice::CloseDevice() {
	if(_usbHandle != NULL && _usbHandle != INVALID_HANDLE_VALUE) {
		try {
			WinUsb_Free(_usbHandle);
			_usbHandle = NULL;
		} catch(exception &e) {
			LOG_ERROR("WinUSB WinUsb_Free ERROR! %s\n", e.what());
		}
	}
	if(_deviceHandle != NULL && _deviceHandle != INVALID_HANDLE_VALUE) {
		try {
			CloseHandle(_deviceHandle);
			_deviceHandle = NULL;
		} catch(exception &e) {
			LOG_ERROR("WinUSB CloseHandler ERROR! %s\n", e.what());
		}
	}
	isOpen = false;
}
