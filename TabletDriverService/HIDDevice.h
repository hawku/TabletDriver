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
	string _manufacturerName;
	string _productName;
	string _serialNumber;
public:
	bool isOpen;
	bool debugEnabled;
	bool isReading;
	USHORT vendorId;
	USHORT productId;
	USHORT usagePage;
	USHORT usage;
	bool isExclusive;


	HIDDevice(USHORT VendorId, USHORT ProductId, USHORT UsagePage, USHORT Usage, bool IsExclusive);
	HIDDevice(USHORT VendorId, USHORT ProductId, USHORT UsagePage, USHORT Usage);
	HIDDevice();
	~HIDDevice();
	bool OpenDevice(HANDLE *handle, USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage, bool exclusive);
	bool OpenDevice(HANDLE *handle, USHORT vendorId, USHORT productId, USHORT usagePage, USHORT usage);
	int Read(void *buffer, int length);
	int Write(void *buffer, int length);
	bool SetFeature(void *buffer, int length);
	bool GetFeature(void *buffer, int length);
	int StringRequest(UCHAR stringId, UCHAR *buffer, int length);
	string GetString(UCHAR stringId);
	string GetManufacturerName();
	string GetProductName();
	string GetSerialNumber();
	void CloseDevice();
};