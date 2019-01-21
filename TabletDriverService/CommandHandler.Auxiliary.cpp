#include "precompiled.h"
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
		if (tablet == NULL) return false;
		if (cmd->valueCount == 4) {
			USHORT vendorID = cmd->GetInt(0, 0);
			USHORT productID = cmd->GetInt(1, 0);
			USHORT usagePage = cmd->GetInt(2, 0);
			USHORT usage = cmd->GetInt(3, 0);
			if (tablet->hidDeviceAux == NULL) {
				tablet->hidDeviceAux = new HIDDevice(vendorID, productID, usagePage, usage);
				if (tablet->hidDeviceAux->isOpen) {
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
	// Command: AuxReport
	//
	AddCommand(new Command("AuxReport", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.auxCurrentReportIndex = cmd->GetInt(0, tablet->settings.auxCurrentReportIndex);
		tablet->settings.auxCurrentReport = &tablet->settings.auxReports[tablet->settings.auxCurrentReportIndex];
		if (tablet->settings.auxCurrentReportIndex >= tablet->settings.auxReportCount - 1) {
			tablet->settings.auxReportCount = tablet->settings.auxCurrentReportIndex + 1;
		}
		LOG_INFO("Auxiliary report index = %d (%d reports)\n",
			tablet->settings.auxCurrentReportIndex,
			tablet->settings.auxReportCount
		);
		return true;
	}));

	//
	// Command: AuxReportId
	//
	AddCommand(new Command("AuxReportId", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.auxCurrentReport->reportId = cmd->GetInt(0, tablet->settings.auxCurrentReport->reportId);
		LOG_INFO("Auxiliary report id = %d\n", tablet->settings.auxCurrentReport->reportId);
		return true;
	}));


	//
	// Command: AuxReportLength
	//
	AddCommand(new Command("AuxReportLength", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.auxReportLength = cmd->GetInt(0, tablet->settings.auxReportLength);
		LOG_INFO("Tablet aux report length = %d\n", tablet->settings.auxReportLength);
		return true;
	}));


	//
	// Command: AuxDetectMask
	//
	AddCommand(new Command("AuxDetectMask", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.auxCurrentReport->detectMask = cmd->GetInt(0, tablet->settings.auxCurrentReport->detectMask);
		LOG_INFO("Auxiliary detect mask = 0x%02X\n", tablet->settings.auxCurrentReport->detectMask);
		return true;
	}));


	//
	// Command: AuxIgnoreMask
	//
	AddCommand(new Command("AuxIgnoreMask", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.auxCurrentReport->ignoreMask = cmd->GetInt(0, tablet->settings.auxCurrentReport->ignoreMask);
		LOG_INFO("Tablet aux ignore mask = 0x%02X\n", tablet->settings.auxCurrentReport->ignoreMask);
		return true;
	}));


	//
	// Command: AuxButtonCount
	//
	AddCommand(new Command("AuxButtonCount", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
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
		if (tablet == NULL) return false;
		tablet->settings.auxCurrentReport->formatter.instructionCount = 0;
		LOG_INFO("Custom aux data format instructions cleared!\n");
		return true;
	}));

}