#pragma once

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <string>

#include <math.h>
#include <winusb.h>
#include <Usb100.h>
#include <SetupAPI.h>
#include <hidsdi.h>
#include <psapi.h>

using namespace std;

class USBDevice {
private:
	HANDLE _deviceHandle;
	WINUSB_INTERFACE_HANDLE _usbHandle;
	bool OpenDevice(string usbDeviceGUIDString, int stringId, string stringMatch);
public:
	string guid;
	int stringId;
	string stringMatch;

	bool isOpen;
	USBDevice(string Guid, int StringId, string StringMatch);
	~USBDevice();
	int Read(UCHAR pipeId, void *buffer, int length);
	int Write(UCHAR pipeId, void *buffer, int length);
	int ControlTransfer(UCHAR requestType, UCHAR request, USHORT value, USHORT index, void *buffer, USHORT length);
	void CloseDevice();
};