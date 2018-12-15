#include "stdafx.h"
#include "CommandHandler.h"

#define LOG_MODULE ""
#include "Logger.h"

//
// Create auxiliary device commands
//
void CommandHandler::CreateAuxCommands() {

	//
	// Command: AuxHID
	//
	AddCommand(new Command("AuxHID", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		if(cmd->valueCount == 4) {
			USHORT vendorID = cmd->GetInt(0, 0);
			USHORT productID = cmd->GetInt(1, 0);
			USHORT usagePage = cmd->GetInt(2, 0);
			USHORT usage = cmd->GetInt(3, 0);
			if(tablet->hidDeviceAux == NULL) {
				tablet->hidDeviceAux = new HIDDevice(vendorID, productID, usagePage, usage);
				if(tablet->hidDeviceAux->isOpen) {
					LOG_INFO("HID auxiliary device found!\n");
					return true;
				}
				else {
					LOG_ERROR("Can't open auxiliary HID 0x%04X 0x%04X 0x%04X 0x%04X\n", vendorID, productID, usagePage, usage);
					delete tablet->hidDeviceAux;
					tablet->hidDeviceAux = NULL;
				}
			}
		}

		return false;
	}));


	//
	// Command: AuxReportId
	//
	AddCommand(new Command("AuxReportId", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.auxReportId = cmd->GetInt(0, tablet->settings.auxReportId);
		LOG_INFO("Tablet aux report id = %d\n", tablet->settings.auxReportId);
		return true;
	}));


	//
	// Command: AuxReportLength
	//
	AddCommand(new Command("AuxReportLength", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.auxReportLength = cmd->GetInt(0, tablet->settings.auxReportLength);
		LOG_INFO("Tablet aux report length = %d\n", tablet->settings.auxReportLength);
		return true;
	}));


	//
	// Command: AuxDetectMask
	//
	AddCommand(new Command("AuxDetectMask", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.auxDetectMask = cmd->GetInt(0, tablet->settings.auxDetectMask);
		LOG_INFO("Tablet aux detect mask = 0x%02X\n", tablet->settings.auxDetectMask);
		return true;
	}));


	//
	// Command: AuxIgnoreMask
	//
	AddCommand(new Command("AuxIgnoreMask", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.auxIgnoreMask = cmd->GetInt(0, tablet->settings.auxIgnoreMask);
		LOG_INFO("Tablet aux ignore mask = 0x%02X\n", tablet->settings.auxIgnoreMask);
		return true;
	}));


	//
	// Command: AuxButtonCount
	//
	AddCommand(new Command("AuxButtonCount", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.auxButtonCount = cmd->GetInt(0, tablet->settings.auxButtonCount);
		LOG_INFO("Tablet aux button count = %d\n", tablet->settings.auxButtonCount);
		return true;
	}));


	//
	// Command: ClearAuxCustomData
	//
	// Clears custom auxiliary data formatter instructions
	//
	AddCommand(new Command("ClearAuxCustomData", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->auxDataFormatter.instructionCount = 0;
		LOG_INFO("Custom aux data format instructions cleared!\n");
		return true;
	}));

}