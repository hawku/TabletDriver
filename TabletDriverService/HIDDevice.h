#pragma once

#include <iostream>
#include <stdio.h>
#include <tchar.h>
#include <strsafe.h>
#include <math.h>

#include <Windows.h>
#include <SetupAPI.h>
#include <hidsdi.h>
#include <psapi.h>


class HIDDevice {
private:
	HANDLE _deviceHandle;
	std::string _manufacturerName;
	std::string _productName;
	std::string _serialNumber;
	std::string _devicePath;
	std::wstring _devicePathW;
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
	std::string GetString(UCHAR stringId);
	std::string GetManufacturerName();
	std::string GetProductName();
	std::string GetSerialNumber();
	void CloseDevice();
};