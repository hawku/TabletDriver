#include "precompiled.h"
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
		if (cmd->valueCount >= 1) {
			std::string guid = cmd->GetString(0, "");
			if (tablet == NULL) {
				tablet = new Tablet(guid);
				if (tablet->isOpen) {
					LOG_INFO("WinUSB tablet found! Manufacturer = '%s', Product = '%s', Serial = '%s'\n",
						tablet->GetDeviceManufacturerName().c_str(),
						tablet->GetDeviceProductName().c_str(),
						tablet->GetDeviceSerialNumber().c_str()
					);
				}
				else {
					LOG_WARNING("Can't open WinUSB tablet '%s'\n", guid.c_str());
					delete tablet;
					tablet = NULL;

					// Slow down to save CPU time on startup
					Sleep(10);
				}
			}
		}

		// Show USB tablet info
		else {
			if (tablet != NULL) {
				if (tablet->usbDevice != NULL) {
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
	AddHelp("HIDTablet", "Usage: HIDTablet <VendorId> <ProductId> <Usage Page> <Usage> [Exclusive=True]");
	AddHelp("HIDTablet", "");
	AddCommand(new Command("HIDTablet", [&](CommandLine *cmd) {

		//
		if (cmd->valueCount >= 4) {
			USHORT vendorID = cmd->GetInt(0, 0);
			USHORT productID = cmd->GetInt(1, 0);
			USHORT usagePage = cmd->GetInt(2, 0);
			USHORT usage = cmd->GetInt(3, 0);
			bool isExclusive = false;
			for (int i = 4; i < cmd->valueCount; i += 2) {
				std::string parameter = cmd->GetStringLower(i, "");
				if (parameter == "exclusive") {
					isExclusive = cmd->GetBoolean(i + 1, false);
				}
			}
			if (tablet == NULL) {
				tablet = new Tablet(vendorID, productID, usagePage, usage, isExclusive);
				if (tablet->isOpen) {
					LOG_INFO("Tablet found! Manufacturer='%s', Product='%s', Serial='%s'\n",
						tablet->GetDeviceManufacturerName().c_str(),
						tablet->GetDeviceProductName().c_str(),
						tablet->GetDeviceSerialNumber().c_str()
					);
				}
				else {
					LOG_WARNING("Can't open HID tablet Vendor=0x%04X Product=0x%04X UsagePage=0x%04X Usage=0x%04X Exclusive=%s\n",
						vendorID, productID, usagePage, usage,
						isExclusive ? "True" : "False");
					delete tablet;
					tablet = NULL;

					// Slow down to save CPU time on startup
					Sleep(10);
				}
			}
		}

		// Show HID tablet info
		else {
			if (tablet != NULL) {
				if (tablet->hidDevice != NULL) {
					HIDDevice *hid = tablet->hidDevice;
					LOG_INFO("HIDTablet = [Vendor=0x%04X, Product=0x%04X, UsagePage=0x%04X, Usage=0x%04X, Exclusive=%s]\n",
						hid->vendorId,
						hid->productId,
						hid->usagePage,
						hid->usage,
						hid->isExclusive ? "True" : "False"
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
		if (tablet != NULL) {
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
	// Command: HIDList
	//
	AddCommand(new Command("HIDList", [&](CommandLine *cmd) {
		HANDLE hidHandle = 0;
		HIDDevice *hid = new HIDDevice();
		hid->debugEnabled = true;
		hid->OpenDevice(&hidHandle, 1, 1, 1, 1);
		SAFE_CLOSE_HANDLE(hidHandle);
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
		if (cmd->valueCount <= 1) return false;
		if (tablet == NULL) return false;
		if (tablet->hidDevice == NULL) return false;
		BYTE *buffer = new BYTE[cmd->valueCount];
		for (int i = 0; i < cmd->valueCount; i++) {
			buffer[i] = cmd->GetInt(i, 0);
		}
		LOG_INFOBUFFER(buffer, cmd->valueCount, "Set Feature Report (%d): ", cmd->valueCount);
		tablet->hidDevice->SetFeature(buffer, cmd->valueCount);
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
		if (cmd->valueCount <= 1) return false;
		if (tablet == NULL) return false;
		if (tablet->hidDevice == NULL) return false;
		int length = cmd->GetInt(0, 1);
		BYTE *buffer = new BYTE[length];
		for (int i = 0; i < length; i++) {
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
		if (cmd->valueCount <= 1) return false;
		if (tablet == NULL) return false;
		if (tablet->hidDevice == NULL) return false;
		BYTE *buffer = new BYTE[cmd->valueCount];
		for (int i = 0; i < cmd->valueCount; i++) {
			buffer[i] = cmd->GetInt(i, 0);
		}
		LOG_INFOBUFFER(buffer, cmd->valueCount, "Sending HID Report: ");
		tablet->hidDevice->Write(buffer, cmd->valueCount);
		LOG_INFO("Report sent!\n");
		delete buffer;
		return true;
	}));


	//
	// Command: USBWrite
	//
	// Writes to WinUSB device
	//
	AddCommand(new Command("USBWrite", [&](CommandLine *cmd) {
		if (cmd->valueCount <= 1) return false;

		if (tablet == NULL) return false;
		if (tablet->usbDevice == NULL) return false;
		int pipeId = cmd->GetInt(0, 0x80);
		int length = cmd->valueCount - 1;
		BYTE *buffer = new BYTE[length];
		for (int i = 0; i < length; i++) {
			buffer[i] = cmd->GetInt(i + 1, 0);
		}
		LOG_INFOBUFFER(buffer, length, "Write to USB: ");
		int written = tablet->usbDevice->Write(pipeId, buffer, length);
		LOG_INFO("  %d bytes written!\n", written);
		delete buffer;
		return true;
	}));


	//
	// Command: GetDeviceStrings, GetStrings, GetString
	//
	// Request strings from HID or WinUSB device
	//
	AddAlias("GetStrings", "GetDeviceStrings");
	AddAlias("GetString", "GetDeviceStrings");
	AddCommand(new Command("GetDeviceStrings", [&](CommandLine *cmd) {
		if (cmd->valueCount <= 0) return false;
		if (tablet == NULL) return false;

		std::string stringName = cmd->GetStringLower(0, "");
		std::string deviceString;

		// Manufacturer
		if (stringName.compare(0, 3, "man") == 0) {
			LOG_INFO("  Manufacturer string = '%s'\n", tablet->GetDeviceManufacturerName().c_str());
			return true;
		}

		// Product name
		else if (stringName.compare(0, 3, "pro") == 0) {
			LOG_INFO("  Product string = '%s'\n", tablet->GetDeviceProductName().c_str());
			return true;
		}

		// Serial number
		else if (stringName.compare(0, 3, "ser") == 0) {
			LOG_INFO("  Serial number string = '%s'\n", tablet->GetDeviceSerialNumber().c_str());
			return true;
		}

		int stringIdMin = cmd->GetInt(0, 0);
		int stringIdMax = cmd->GetInt(1, stringIdMin);

		// Limits
		if (stringIdMin < 1) stringIdMin = 1;
		else if (stringIdMin > 256) stringIdMin = 256;
		if (stringIdMax < 1) stringIdMax = 1;
		else if (stringIdMax > 256) stringIdMax = 256;

		// Min > Max
		if (stringIdMin > stringIdMax) {
			int tmp = stringIdMax;
			stringIdMax = stringIdMin;
			stringIdMin = tmp;
		}

		// Multiple ids
		if (stringIdMin != stringIdMax) {
			LOG_INFO("Requesting string ids from %d to %d:\n", stringIdMin, stringIdMax);
		}

		// Single id
		else {
			LOG_INFO("Requesting string id %d:\n", stringIdMin);
		}

		// Loop through string ids
		for (int stringId = stringIdMin; stringId <= stringIdMax; stringId++) {
			try {
				std::string deviceString = tablet->GetDeviceString(stringId);
				LOG_INFO("  String (%d) = '%s'\n", stringId, deviceString.c_str());
			}
			catch (const std::exception &e) {
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
	AddHelp("CheckDeviceString", "Usage:");
	AddHelp("CheckDeviceString", "  CheckDeviceString <string id> \"<match>\"");
	AddHelp("CheckDeviceString", "  CheckDeviceString Manufacturer \"<match>\"");
	AddHelp("CheckDeviceString", "  CheckDeviceString Product \"<match>\"");
	AddHelp("CheckDeviceString", "  CheckDeviceString SerialNumber \"<match>\"");
	AddCommand(new Command("CheckDeviceString", [&](CommandLine *cmd) {
		if (cmd->valueCount <= 1) {
			ExecuteCommand("Help", "CheckDeviceString");
			return false;
		}
		if (tablet == NULL) return false;

		int stringId = cmd->GetInt(0, 0);
		std::string stringName = cmd->GetStringLower(0, "");
		std::string matchString = cmd->GetString(1, "");
		std::string deviceString = "";

		// Manufacturer
		if (stringName.compare(0, 3, "man") == 0) {
			stringName = "Manufacturer";
			deviceString = tablet->GetDeviceManufacturerName();
		}

		// Product name
		else if (stringName.compare(0, 3, "pro") == 0) {
			stringName = "Product";
			deviceString = tablet->GetDeviceProductName();
		}

		// Serial number
		else if (stringName.compare(0, 3, "ser") == 0) {
			stringName = "SerialNumber";
			deviceString = tablet->GetDeviceSerialNumber();
		}

		// Request device string 
		else {
			stringName = std::to_string(stringId);
			if (stringId == 0) {
				LOG_ERROR("Invalid string id!\n");
				return false;
			}
			try {
				deviceString = tablet->GetDeviceString(stringId);
			}
			catch (std::exception&e) {
				LOG_ERROR("%s\n", e.what());
			}
		}

		// Does the string match?
		if (deviceString.size() >= matchString.size() && deviceString.compare(0, matchString.size(), matchString) == 0) {
			LOG_INFO("Device string (%s) '%s' matches with '%s'\n",
				stringName.c_str(),
				deviceString.c_str(),
				matchString.c_str()
			);
			return true;
		}

		// Does not match
		else {
			LOG_INFO("Device string (%s) '%s' does not match with '%s'. Tablet invalid!\n",
				stringName.c_str(),
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