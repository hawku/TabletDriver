#pragma once

#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <math.h>
#include <SetupAPI.h>
#include <hidsdi.h>
#include <psapi.h>
#include <iostream>


using namespace std;

class HIDDevice {
private:
	HANDLE _deviceHandle;
	bool OpenDevice(HANDLE *handle, USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage);
public:
	bool isOpen;
	USHORT vendorId;
	USHORT productId;
	USHORT usagePage;
	USHORT usage;

	HIDDevice(USHORT VendorId, USHORT ProductId, USHORT UsagePage, USHORT Usage);
	~HIDDevice();
	int Read(void *buffer, int length);
	int Write(void *buffer, int length);
	bool SetFeature(void *buffer, int length);
	void CloseDevice();
};