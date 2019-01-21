#include "precompiled.h"
#include "USBDevice.h"

#define LOG_MODULE "USBDevice"
#include "Logger.h"

USBDevice::USBDevice(std::string Guid) {
	this->guid = Guid;
	isOpen = false;
	if(this->OpenDevice(&_deviceHandle, &_usbHandle, &deviceDescriptor, guid)) {
		isOpen = true;
	}
}
USBDevice::~USBDevice() {
	this->CloseDevice();
}


bool USBDevice::OpenDevice(HANDLE *outDeviceHandle, WINUSB_INTERFACE_HANDLE *outUSBHandle, USB_DEVICE_DESCRIPTOR *outDeviceDescriptor, std::string usbDeviceGUIDString) {
	GUID usbDeviceGUID;

	HDEVINFO                         deviceInfo;
	SP_DEVICE_INTERFACE_DATA         deviceInterfaceData;
	PSP_DEVICE_INTERFACE_DETAIL_DATA deviceInterfaceDetailData;
	SP_DEVINFO_DATA                  deviceInfoData;
	USB_DEVICE_DESCRIPTOR			 usbDeviceDescriptor;
	USB_INTERFACE_DESCRIPTOR		 usbInterfaceDescriptor;

	DWORD dwSize;
	DWORD dwMemberIdx;
	ULONG bytesRead = 0;
	bool deviceFound = false;

	HANDLE deviceHandle = 0;
	WINUSB_INTERFACE_HANDLE usbHandle = 0;

	// Create GUID wstring
	std::wstring stemp = std::wstring(usbDeviceGUIDString.begin(), usbDeviceGUIDString.end());
	LPCWSTR wstringGUID = stemp.c_str();

	// Create CLSID from GUID string
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

	// Enumerate device interfaces
	deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);
	dwMemberIdx = 0;
	SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &usbDeviceGUID, dwMemberIdx, &deviceInterfaceData);

	while(GetLastError() != ERROR_NO_MORE_ITEMS) {

		// Get the required interface detail buffer size. 
		deviceInfoData.cbSize = sizeof(deviceInfoData);
		SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, NULL, 0, &dwSize, NULL);

		// Allocate memory for interface detail data
		deviceInterfaceDetailData = (PSP_DEVICE_INTERFACE_DETAIL_DATA)malloc(dwSize);
		deviceInterfaceDetailData->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA);

		// Get device interface detail data
		if(SetupDiGetDeviceInterfaceDetail(deviceInfo, &deviceInterfaceData, deviceInterfaceDetailData, dwSize, &dwSize, &deviceInfoData)) {

			// Create device file
			deviceHandle = CreateFile(
				deviceInterfaceDetailData->DevicePath,
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				OPEN_EXISTING,
				FILE_FLAG_OVERLAPPED,
				NULL
			);

			// Check file handle
			if(deviceHandle != INVALID_HANDLE_VALUE) {

				// Initialize WinUsb
				WinUsb_Initialize(deviceHandle, &usbHandle);
				if(!usbHandle) {
					LOG_ERROR("ERROR! Unable to initialize WinUSB!\n");
					SAFE_CLOSE_HANDLE(deviceHandle);
					return false;
				}

				// Clear descriptors
				memset(&usbDeviceDescriptor, 0, sizeof(USB_DEVICE_DESCRIPTOR));
				memset(&usbInterfaceDescriptor, 0, sizeof(USB_INTERFACE_DESCRIPTOR));
				bytesRead = 0;

				//
				// Is the device valid?
				//
				if(
					// Get device descriptor
					WinUsb_GetDescriptor(
						usbHandle, USB_DEVICE_DESCRIPTOR_TYPE, 0, 0x409,
						(UCHAR*)&usbDeviceDescriptor,
						sizeof(usbDeviceDescriptor),
						&bytesRead
					)
					&&

					// Get device interface settings
					WinUsb_QueryInterfaceSettings(usbHandle, 0, &usbInterfaceDescriptor)
				) {

					// Copy handles
					memcpy(outDeviceHandle, &deviceHandle, sizeof(HANDLE));
					memcpy(outUSBHandle, &usbHandle, sizeof(WINUSB_INTERFACE_HANDLE));

					// Copy device descriptor
					memcpy(outDeviceDescriptor, &usbDeviceDescriptor, sizeof(usbDeviceDescriptor));

					deviceFound = true;
				}

				//
				// Invalid device
				//
				else {

					// Free WinUSB handle
					if(usbHandle && usbHandle != INVALID_HANDLE_VALUE)
						WinUsb_Free(usbHandle);

					// Free device file
					SAFE_CLOSE_HANDLE(deviceHandle);
				}
			}
		}

		// Destroy interface detail data
		free(deviceInterfaceDetailData);

		// Enumerate to the next interface
		SetupDiEnumDeviceInterfaces(deviceInfo, NULL, &usbDeviceGUID, ++dwMemberIdx, &deviceInterfaceData);
	}

	// Destroy device info list
	SetupDiDestroyDeviceInfoList(deviceInfo);

	return deviceFound;
}


int USBDevice::Read(UCHAR pipeId, void *buffer, int length) {
	ULONG bytesRead;
	try {
		if(WinUsb_ReadPipe(_usbHandle, pipeId, (UCHAR *)buffer, length, &bytesRead, 0)) {
			return (int)bytesRead;
		}
	} catch(std::exception &e) {
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

std::string USBDevice::GetString(UCHAR stringId) {
	std::string resultString = "";
	UCHAR buffer[256];
	int bytesRead = 0;

	bytesRead = StringRequest(stringId, buffer, 256);

	// Reply received?
	if(bytesRead > 0) {
		for(int i = 0; i < bytesRead; i += 2) {
			resultString.push_back(buffer[i]);
		}
	}

	return resultString;
}

// Get WinUSB device manufacturer name
std::string USBDevice::GetManufacturerName() {
	if(deviceDescriptor.iManufacturer == 0) return "";
	return GetString(deviceDescriptor.iManufacturer);
}

// Get WinUSB device product name
std::string USBDevice::GetProductName()
{
	if(deviceDescriptor.iProduct == 0) return "";
	return GetString(deviceDescriptor.iProduct);
}

// Get WinUSB device serial number
std::string USBDevice::GetSerialNumber()
{
	if(deviceDescriptor.iSerialNumber == 0) return "";
	return GetString(deviceDescriptor.iSerialNumber);
}

//
// Close WinUSB device
//
void USBDevice::CloseDevice() {
	if(_usbHandle != NULL && _usbHandle != INVALID_HANDLE_VALUE) {
		try {
			WinUsb_Free(_usbHandle);
			_usbHandle = NULL;
		} catch(std::exception &e) {
			LOG_ERROR("WinUSB WinUsb_Free ERROR! %s\n", e.what());
		}
	}
	if(_deviceHandle != NULL && _deviceHandle != INVALID_HANDLE_VALUE) {
		try {
			SAFE_CLOSE_HANDLE(_deviceHandle);
		} catch(std::exception &e) {
			LOG_ERROR("WinUSB CloseHandler ERROR! %s\n", e.what());
		}
	}
	isOpen = false;
}
