#pragma once

#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <string>
#include <math.h>

#include <Windows.h>
#include <winusb.h>
#include <Usb100.h>
#include <SetupAPI.h>
#include <hidsdi.h>
#include <psapi.h>

class USBDevice {
private:
	HANDLE _deviceHandle;
	WINUSB_INTERFACE_HANDLE _usbHandle;
	bool OpenDevice(HANDLE *outDeviceHandle, WINUSB_INTERFACE_HANDLE *outUSBHandle, USB_DEVICE_DESCRIPTOR *outDeviceDescriptor, std::string usbDeviceGUIDString);
public:
	std::string guid;
	USB_DEVICE_DESCRIPTOR deviceDescriptor;
	bool isOpen;

	USBDevice(std::string Guid);
	~USBDevice();
	int Read(UCHAR pipeId, void *buffer, int length);
	int Write(UCHAR pipeId, void *buffer, int length);
	int ControlTransfer(UCHAR requestType, UCHAR request, USHORT value, USHORT index, void *buffer, USHORT length);
	int StringRequest(UCHAR stringId, UCHAR *buffer, int length);
	std::string GetString(UCHAR stringId);
	std::string GetManufacturerName();
	std::string GetProductName();
	std::string GetSerialNumber();
	void CloseDevice();
};