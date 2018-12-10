#include "stdafx.h"
#include "CommandHandler.h"

#define LOG_MODULE ""
#include "Logger.h"

//
// Create HID/USB device commands
//
void CommandHandler::CreateDeviceCommands() {

	//
	// Command: USBTablet
	//
	AddHelp("USBTablet", "Usage: USBTablet \"<WinUSB GUID>\"");
	AddHelp("USBTablet", "");
	AddCommand(new Command("USBTablet", [&](CommandLine *cmd) {

		// 
		if(cmd->valueCount >= 1) {
			string guid = cmd->GetString(0, "");
			if(tablet == NULL) {
				tablet = new Tablet(guid);
				if(tablet->isOpen) {
					LOG_INFO("WinUSB tablet found!\n");
				}
				else {
					LOG_WARNING("Can't open WinUSB tablet '%s'\n", guid.c_str());
					delete tablet;
					tablet = NULL;
				}
			}
		}

		//
		else {
			if(tablet != NULL) {
				if(tablet->usbDevice != NULL) {
					USBDevice *usb = tablet->usbDevice;
					LOG_INFO("USBTablet = \"%s\"\n",
						usb->guid.c_str()
					);
				}
			}
			else {
				LOG_INFO("Tablet is not defined.\n");
			}
		}

		return true;
	}));


	//
	// Command: HIDTablet
	//
	AddHelp("HIDTablet", "Usage: HIDTablet <VendorId> <ProductId> <Usage Page> <Usage>");
	AddHelp("HIDTablet", "");
	AddCommand(new Command("HIDTablet", [&](CommandLine *cmd) {

		//
		if(cmd->valueCount >= 4) {
			USHORT vendorID = cmd->GetInt(0, 0);
			USHORT productID = cmd->GetInt(1, 0);
			USHORT usagePage = cmd->GetInt(2, 0);
			USHORT usage = cmd->GetInt(3, 0);
			if(tablet == NULL) {
				tablet = new Tablet(vendorID, productID, usagePage, usage);
				if(tablet->isOpen) {
					LOG_INFO("Tablet found!\n");
				}
				else {
					LOG_WARNING("Can't open HID tablet 0x%04X 0x%04X 0x%04X 0x%04X\n", vendorID, productID, usagePage, usage);
					delete tablet;
					tablet = NULL;
				}
			}
		}

		//
		else {
			if(tablet != NULL) {
				if(tablet->hidDevice != NULL) {
					HIDDevice *hid = tablet->hidDevice;
					LOG_INFO("Tablet = HID(0x%04X 0x%04X 0x%04X 0x%04X)\n",
						hid->vendorId,
						hid->productId,
						hid->usagePage,
						hid->usage
					);
				}
			}
			else {
				LOG_INFO("Tablet is not defined.\n");
			}
		}

		return true;
	}));


	//
	// Command: CloseTablet
	//
	AddCommand(new Command("CloseTablet", [&](CommandLine *cmd) {
		if(tablet != NULL) {
			delete tablet;
			tablet = NULL;
			LOG_INFO("Tablet closed!\n");
			return true;
		}
		else {
			LOG_INFO("Tablet is not open!\n");
			return false;
		}
		return true;
	}));


	//
	// Command: HIDAux
	//
	AddCommand(new Command("HIDAuxiliary", [&](CommandLine *cmd) {

		if(cmd->valueCount == 4) {
			USHORT vendorID = cmd->GetInt(0, 0);
			USHORT productID = cmd->GetInt(1, 0);
			USHORT usagePage = cmd->GetInt(2, 0);
			USHORT usage = cmd->GetInt(3, 0);
			if(tablet->hidDeviceAux == NULL) {
				tablet->hidDeviceAux = new HIDDevice(vendorID, productID, usagePage, usage);
				if(tablet->hidDeviceAux->isOpen) {
					LOG_INFO("HID Device found!\n");
					return true;
				}
				else {
					LOG_ERROR("Can't open HID device 0x%04X 0x%04X 0x%04X 0x%04X\n", vendorID, productID, usagePage, usage);
					delete tablet->hidDeviceAux;
					tablet->hidDeviceAux = NULL;
				}
			}
		}

		return false;
	}));


	//
	// Command: HIDAuxRead
	//
	AddCommand(new Command("HIDAuxRead", [&](CommandLine *cmd) {

		if(cmd->valueCount > 1 && tablet->hidDeviceAux != NULL) {
			int length = cmd->GetInt(0, 1);
			int count = cmd->GetInt(1, 1);
			UCHAR buffer[1024];
			for(int i = 0; i < count; i++) {
				tablet->hidDeviceAux->Read(buffer, length);
				LOG_INFOBUFFER(buffer, length, "Aux read(%d): ", i + 1);
			}
		}

		return true;
	}));


	//
	// Command: HIDList
	//
	AddAlias("HIDAux", "HIDAuxiliary");
	AddCommand(new Command("HIDList", [&](CommandLine *cmd) {
		HANDLE hidHandle = 0;
		HIDDevice *hid = new HIDDevice();
		hid->debugEnabled = true;
		hid->OpenDevice(&hidHandle, 1, 1, 1, 1);
		if(hidHandle > 0) {
			CloseHandle(hidHandle);
		}
		delete hid;

		return true;
	}));


	//
	// Command: SetFeature, Feature
	//
	// Sends HID feature report
	//
	AddAlias("Feature", "SetFeature");
	AddCommand(new Command("SetFeature", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 1) return false;
		if(tablet == NULL) return false;
		if(tablet->hidDevice == NULL) return false;
		int length = cmd->GetInt(0, 1);
		BYTE *buffer = new BYTE[length];
		for(int i = 0; i < length; i++) {
			buffer[i] = cmd->GetInt(i + 1, 0);
		}
		LOG_INFOBUFFER(buffer, length, "Set Feature Report (%d): ", length);
		tablet->hidDevice->SetFeature(buffer, length);
		LOG_INFO("HID Feature set!\n");
		delete buffer;
		return true;
	}));


	//
	// Command: GetFeature
	//
	// Requests a HID feature report from the device
	//
	AddCommand(new Command("GetFeature", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 1) return false;
		if(tablet == NULL) return false;
		if(tablet->hidDevice == NULL) return false;
		int length = cmd->GetInt(0, 1);
		BYTE *buffer = new BYTE[length];
		for(int i = 0; i < length; i++) {
			buffer[i] = cmd->GetInt(i + 1, 0);
		}
		LOG_INFOBUFFER(buffer, length, "Get Feature Report (%d): ", length);
		tablet->hidDevice->GetFeature(buffer, length);
		LOG_INFOBUFFER(buffer, length, "Result Feature Report (%d): ", length);
		delete buffer;
		return true;
	}));


	//
	// Command: OutputReport, Report
	//
	// Sends HID output report
	//
	AddAlias("Report", "OutputReport");
	AddCommand(new Command("OutputReport", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 1) return false;
		if(tablet == NULL) return false;
		if(tablet->hidDevice == NULL) return false;
		int length = cmd->GetInt(0, 1);
		BYTE *buffer = new BYTE[length];
		for(int i = 0; i < length; i++) {
			buffer[i] = cmd->GetInt(i + 1, 0);
		}
		LOG_INFOBUFFER(buffer, length, "Sending HID Report: ");
		tablet->hidDevice->Write(buffer, length);
		LOG_INFO("Report sent!\n");
		delete buffer;
		return true;
	}));


	//
	// Command: HIDStringRequest, HIDString
	//
	// Request a string from HID device
	//
	AddAlias("HIDString", "HIDStringRequest");
	AddCommand(new Command("HIDStringRequest", [&](CommandLine *cmd) {
		UCHAR buffer[256];
		int bytesRead;
		string stringReply = "";

		if(cmd->valueCount <= 0) return false;
		if(tablet == NULL) return false;
		if(tablet->hidDevice == NULL) return false;
		if(tabletHandler->isRunning) {
			LOG_ERROR("This command can only be run during the tablet configuration!\n");
			return false;
		}
		int stringIdMin = cmd->GetInt(0, 0);
		int stringIdMax = cmd->GetInt(1, stringIdMin);

		// Limits
		if(stringIdMin < 1) stringIdMin = 1;
		else if(stringIdMin > 256) stringIdMin = 256;
		if(stringIdMax < 1) stringIdMax = 1;
		else if(stringIdMax > 256) stringIdMax = 256;

		// Loop through string ids
		for(int stringId = stringIdMin; stringId <= stringIdMax; stringId++) {

			// String request USB control transter
			bytesRead = tablet->hidDevice->StringRequest(stringId, buffer, 256);

			if(bytesRead > 0) {
				// Bytes to string
				stringReply = "";
				for(int i = 0; i < bytesRead; i += 2) {
					stringReply.push_back(buffer[i]);
				}
				LOG_INFO("HIDString (id %d, %d bytes): '%s'\n", stringId, bytesRead, stringReply.c_str());
			}
			else {
				LOG_INFO("HIDString (id %d, 0 bytes)\n", stringId);
			}
		}

		return true;
	}));


	//
	// Command: USBWrite
	//
	// Writes to WinUSB device
	//
	AddCommand(new Command("USBWrite", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 1) return false;

		if(tablet == NULL) return false;
		if(tablet->usbDevice == NULL) return false;
		int pipeId = cmd->GetInt(0, 0x80);
		int length = cmd->valueCount - 1;
		BYTE *buffer = new BYTE[length];
		for(int i = 0; i < length; i++) {
			buffer[i] = cmd->GetInt(i + 1, 0);
		}
		LOG_INFOBUFFER(buffer, length, "Write to USB: ");
		int written = tablet->usbDevice->Write(pipeId, buffer, length);
		LOG_INFO("  %d bytes written!\n", written);
		delete buffer;
		return true;
	}));


	//
	// Command: USBStringRequest, USBString
	//
	// Request a string from a WinUSB device
	//
	AddAlias("USBString", "USBStringRequest");
	AddCommand(new Command("USBStringRequest", [&](CommandLine *cmd) {
		UCHAR buffer[256];
		int bytesRead;
		string stringReply = "";

		if(cmd->valueCount <= 0) return false;
		if(tablet == NULL) return false;
		if(tablet->usbDevice == NULL) return false;
		int stringIdMin = cmd->GetInt(0, 0);
		int stringIdMax = cmd->GetInt(1, stringIdMin);

		// Limits
		if(stringIdMin < 1) stringIdMin = 1;
		else if(stringIdMin > 256) stringIdMin = 256;
		if(stringIdMax < 1) stringIdMax = 1;
		else if(stringIdMax > 256) stringIdMax = 256;

		// Loop through string ids
		for(int stringId = stringIdMin; stringId <= stringIdMax; stringId++) {

			// String request USB control transter
			bytesRead = tablet->usbDevice->StringRequest(stringId, buffer, 256);

			if(bytesRead > 0) {
				// Bytes to string
				stringReply = "";
				for(int i = 0; i < bytesRead; i += 2) {
					stringReply.push_back(buffer[i]);
				}
				LOG_INFO("USBString (id %d, %d bytes): '%s'\n", stringId, bytesRead, stringReply.c_str());
			}
			else {
				LOG_INFO("USBString (id %d, 0 bytes)\n", stringId);
			}
		}

		return true;
	}));


	//
	// Command: GetDeviceStrings, GetStrings
	//
	// Request strings from HID or WinUSB device
	//
	AddAlias("GetStrings", "GetDeviceStrings");
	AddCommand(new Command("GetDeviceStrings", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 0) return false;
		if(tablet == NULL) return false;

		int stringIdMin = cmd->GetInt(0, 0);
		int stringIdMax = cmd->GetInt(1, stringIdMin);

		// Limits
		if(stringIdMin < 1) stringIdMin = 1;
		else if(stringIdMin > 256) stringIdMin = 256;
		if(stringIdMax < 1) stringIdMax = 1;
		else if(stringIdMax > 256) stringIdMax = 256;

		// Min > Max
		if(stringIdMin > stringIdMax) {
			int tmp = stringIdMax;
			stringIdMax = stringIdMin;
			stringIdMin = tmp;
		}

		// Multiple ids
		if(stringIdMin != stringIdMax) {
			LOG_INFO("Requesting string ids from %d to %d:\n", stringIdMin, stringIdMax);
		}

		// Single id
		else {
			LOG_INFO("Requesting string id %d:\n", stringIdMin);
		}

		// Loop through string ids
		for(int stringId = stringIdMin; stringId <= stringIdMax; stringId++) {
			try {
				string deviceString = tablet->GetDeviceString(stringId);
				LOG_INFO("  String (%d) = '%s'\n", stringId, deviceString.c_str());
			} catch(const exception &e) {
				LOG_ERROR("%s\n", e.what());
				break;
			}

		}

		return true;

	}));


	//
	// Command: CheckDeviceString, CheckString
	//
	// Check if a device string matches
	//
	AddAlias("CheckString", "CheckDeviceString");
	AddCommand(new Command("CheckDeviceString", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 1) return false;
		if(tablet == NULL) return false;

		int stringId = cmd->GetInt(0, 0);
		string matchString = cmd->GetString(1, "");

		// Request and read device string 
		string deviceString = tablet->GetDeviceString(stringId);

		// Does the string match?
		if(deviceString.size() >= matchString.size() && deviceString.compare(0, matchString.size(), matchString) == 0) {
			LOG_INFO("Device string (id %d) '%s' matches with '%s'\n",
				stringId,
				deviceString.c_str(),
				matchString.c_str()
			);
			return true;
		}

		// Does not match
		else {
			LOG_INFO("Device string (id %d) '%s' does not match with '%s'. Tablet invalid!\n",
				stringId,
				deviceString.c_str(),
				matchString.c_str()
			);
			delete tablet;
			tablet = NULL;
			return false;
		}
		return true;

	}));

}