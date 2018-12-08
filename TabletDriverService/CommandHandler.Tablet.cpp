#include "stdafx.h"
#include "CommandHandler.h"

#define LOG_MODULE ""
#include "Logger.h"

//
// Create tablet commands
//
void CommandHandler::CreateTabletCommands() {

	//
	// Command: TabletIsValid
	//
	AddCommand(new Command("TabletValid", [&](CommandLine *cmd) {
		if(tablet == NULL) {
			return false;
		}
		return true;
	}));


	//
	// Command: CheckTablet
	//
	AddCommand(new Command("CheckTablet", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) {
			LOG_ERROR("Tablet not found!\n");
			LOG_ERROR("Check the list of supported tablets from the GitHub page.\n");
			LOG_ERROR("http://github.com/hawku/TabletDriver\n");
			CleanupAndExit(1);
			return false;
		}
		return true;
	}));


	//
	// Command: TabletName, Name
	//
	AddAlias("Name", "TabletName");
	AddCommand(new Command("TabletName", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->name = cmd->GetString(0, tablet->name);
		LOG_INFO("Tablet name = '%s'\n", tablet->name.c_str());
		return true;
	}));


	//
	// Command: ReportId
	//
	AddCommand(new Command("ReportId", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.reportId = cmd->GetInt(0, tablet->settings.reportId);
		LOG_INFO("Tablet report id = %d\n", tablet->settings.reportId);
		return true;
	}));


	//
	// Command: ReportLength
	//
	AddCommand(new Command("ReportLength", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.reportLength = cmd->GetInt(0, tablet->settings.reportLength);
		LOG_INFO("Tablet report length = %d\n", tablet->settings.reportLength);
		return true;
	}));


	//
	// Command: DetectMask
	//
	AddCommand(new Command("DetectMask", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.detectMask = cmd->GetInt(0, tablet->settings.detectMask);
		LOG_INFO("Tablet detect mask = %02X\n", tablet->settings.detectMask);
		return true;
	}));


	//
	// Command: IgnoreMask
	//
	AddCommand(new Command("IgnoreMask", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.ignoreMask = cmd->GetInt(0, tablet->settings.ignoreMask);
		LOG_INFO("Tablet ignore mask = %02X\n", tablet->settings.ignoreMask);
		return true;
	}));


	//
	// Command: MaxX
	//
	AddCommand(new Command("MaxX", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.maxX = cmd->GetInt(0, tablet->settings.maxX);
		LOG_INFO("Tablet max X = %d\n", tablet->settings.maxX);
		return true;
	}));


	//
	// Command: MaxY
	//
	AddCommand(new Command("MaxY", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.maxY = cmd->GetInt(0, tablet->settings.maxY);
		LOG_INFO("Tablet max Y = %d\n", tablet->settings.maxY);
		return true;
	}));


	//
	// Command: MaxPressure
	//
	AddCommand(new Command("MaxPressure", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.maxPressure = cmd->GetInt(0, tablet->settings.maxPressure);
		LOG_INFO("Tablet max pressure = %d\n", tablet->settings.maxPressure);
		return true;
	}));

	//
	// Command: ClickPressure
	//
	AddCommand(new Command("ClickPressure", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.clickPressure = cmd->GetInt(0, tablet->settings.clickPressure);
		LOG_INFO("Tablet click pressure = %d\n", tablet->settings.clickPressure);
		return true;
	}));


	//
	// Command: KeepTipDown
	//
	AddCommand(new Command("KeepTipDown", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.keepTipDown = cmd->GetInt(0, tablet->settings.keepTipDown);
		LOG_INFO("Tablet pen tip keep down = %d reports\n", tablet->settings.keepTipDown);
		return true;
	}));


	//
	// Command: Width
	//
	AddCommand(new Command("Width", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.width = cmd->GetDouble(0, tablet->settings.width);
		LOG_INFO("Tablet width = %0.2f mm\n", tablet->settings.width);
		return true;
	}));


	//
	// Command: Height
	//
	AddCommand(new Command("Height", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.height = cmd->GetDouble(0, tablet->settings.height);
		LOG_INFO("Tablet height = %0.2f mm\n", tablet->settings.height);
		return true;
	}));


	//
	// Command: Skew
	//
	AddCommand(new Command("Skew", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		tablet->settings.skew = cmd->GetDouble(0, tablet->settings.skew);
		LOG_INFO("Tablet skew = Shift X-axis %0.2f mm per Y-axis mm\n", tablet->settings.skew);
		return true;
	}));


	//
	// Command: TabletDataFormat, TabletFormat, Format
	//
	// Sets tablet data format
	//
	AddAlias("Format", "TabletDataFormat");
	AddAlias("TabletFormat", "TabletDataFormat");
	AddCommand(new Command("TabletDataFormat", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;
		string format = cmd->GetStringLower(0, "");

		// Wacom Intuos Format V2 (Wacom CTL-490)
		if(format == "wacomintuosv2") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatWacomIntuosV2;
		}

		// Wacom Intuos Format V3 (Wacom CTL-4100)
		else if(format == "wacomintuosv3" || format == "wacom4100") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatWacomIntuosV3;
		}

		// Wacom Drivers
		else if(format == "wacomdrivers") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatWacomDrivers;
		}

		// Skip first data byte (VEIKK S640)
		else if(format == "skipfirstdatabyte") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatSkipFirstDataByte;
		}

		// Unknown type
		else {
			LOG_ERROR("Unknown tablet data format: %s\n", format.c_str());
		}

		LOG_INFO("Tablet format = %d\n", tablet->settings.dataFormat);

		return true;
	}));


	//
	// Command: InitFeature
	//
	AddCommand(new Command("InitFeature", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 0) return false;
		if(tablet == NULL) return false;
		tablet->initFeature = new BYTE[cmd->valueCount];
		for(int i = 0; i < (int)cmd->valueCount; i++) {
			tablet->initFeature[i] = cmd->GetInt(i, 0);
		}
		tablet->initFeatureLength = cmd->valueCount;
		LOG_INFOBUFFER(tablet->initFeature, tablet->initFeatureLength, "Tablet initialization feature report: ");
		return true;
	}));


	//
	// Command: InitReport
	//
	// Initialization HID output report
	//
	AddCommand(new Command("InitReport", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 0) return false;

		if(tablet == NULL) return false;
		tablet->initReport = new BYTE[cmd->valueCount];
		for(int i = 0; i < (int)cmd->valueCount; i++) {
			tablet->initReport[i] = cmd->GetInt(i, 0);
		}
		tablet->initReportLength = cmd->valueCount;
		LOG_INFOBUFFER(tablet->initReport, tablet->initReportLength, "Tablet initialization report: ");

		return true;
	}));


	//
	// Command: InitStrings, InitString
	//
	AddAlias("InitString", "InitStrings");
	AddCommand(new Command("InitStrings", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 0) return false;
		if(tablet == NULL) return false;
		for(int i = 0; i < (int)cmd->valueCount; i++) {
			int stringId = cmd->GetInt(i, 0);
			if(stringId > 0 && stringId <= 256) {
				tablet->initStrings.push_back(stringId);
			}
		}
		string stringIdsString = "";
		for(int stringId : tablet->initStrings) {
			stringIdsString += to_string(stringId) + " ";
		}
		LOG_INFO("Tablet initialization string ids = %s\n", stringIdsString.c_str());

		return true;
	}));


	//
	// Command: TabletArea, Area
	//
	// Sets the user tablet area
	//
	AddAlias("Area", "TabletArea");
	AddCommand(new Command("TabletArea", [&](CommandLine *cmd) {

		if(!ExecuteCommand("TabletValid")) return false;
		mapper->areaTablet.width = cmd->GetDouble(0, mapper->areaTablet.width);
		mapper->areaTablet.height = cmd->GetDouble(1, mapper->areaTablet.height);
		mapper->areaTablet.x = cmd->GetDouble(2, mapper->areaTablet.x);
		mapper->areaTablet.y = cmd->GetDouble(3, mapper->areaTablet.y);

		ExecuteCommand("LogTabletArea", "Tablet area");
		return true;
	}));


	//
	// Command: ButtonMap, Buttons
	//
	// Maps input buttons to output buttons
	//
	AddAlias("Buttons", "ButtonMap");
	AddCommand(new Command("ButtonMap", [&](CommandLine *cmd) {

		if(!ExecuteCommand("TabletValid")) return false;
		char buttonMapBuffer[32];
		int index = 0;
		for(int i = 0; i < 8; i++) {
			tablet->buttonMap[i] = cmd->GetInt(i, tablet->buttonMap[i]);
			index += sprintf_s(buttonMapBuffer + index, 32 - index, "%d ", tablet->buttonMap[i]);
		}
		LOG_INFO("Button Map = %s\n", buttonMapBuffer);
		return true;
	}));


	//
	// Command: TabletMove
	//
	// Move tablet area offset from the area borders
	//
	AddAlias("Move", "TabletMove");
	AddCommand(new Command("TabletMove", [&](CommandLine *cmd) {
		if(cmd->valueCount <= 0) return false;
		if(!ExecuteCommand("TabletValid")) return false;
		string border;
		double offset;
		bool moved = true;

		for(int i = 0; i < cmd->valueCount; i += 2) {

			border = cmd->GetStringLower(i, "");
			offset = cmd->GetDouble(i + 1, 0.0);
			moved = true;

			if(border == "top") {
				mapper->areaTablet.y = offset + mapper->areaTablet.height / 2.0;
			}
			else if(border == "bottom") {
				mapper->areaTablet.y = tablet->settings.height - mapper->areaTablet.height / 2.0 - offset;
			}
			else if(border == "left") {
				mapper->areaTablet.x = offset + mapper->areaTablet.width / 2.0;;
			}
			else if(border == "right") {
				mapper->areaTablet.x = tablet->settings.width - mapper->areaTablet.width / 2.0 - offset;
			}
			else {
				moved = false;
			}
			if(moved) {
				LOG_INFO("Tablet area moved to %s border with %0.2f mm margin.\n", border.c_str(), offset);
				ExecuteCommand("LogTabletArea", "  New tablet area");
			}
		}
		return true;
	}));


	//
	// Command: Rotate
	//
	// Sets tablet area rotation
	//
	AddCommand(new Command("Rotate", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;

		double value = cmd->GetDouble(0, 0);
		mapper->SetRotation(value);
		LOG_INFO("Rotation matrix = [%f,%f,%f,%f]\n",
			mapper->rotationMatrix[0],
			mapper->rotationMatrix[1],
			mapper->rotationMatrix[2],
			mapper->rotationMatrix[3]
		);
		return true;
	}));


	//
	// Command: RelativeSensitivity, Sensitivity
	//
	// Sets the relative mouse output sensitivity
	//
	AddAlias("Sensitivity", "RelativeSensitivity");
	AddCommand(new Command("RelativeSensitivity", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;
		outputManager->settings->relativeSensitivity = cmd->GetDouble(0, outputManager->settings->relativeSensitivity);
		LOG_INFO("Relative mode sensitivity = %0.2f px/mm\n", outputManager->settings->relativeSensitivity);
		return true;
	}));


	//
	// Command: ResetDistance
	//
	// Sets relative mouse output reset distance
	//
	AddAlias("ResetDistance", "RelativeResetDistance");
	AddCommand(new Command("RelativeResetDistance", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;
		outputManager->settings->relativeResetDistance = cmd->GetDouble(0, outputManager->settings->relativeResetDistance);
		LOG_INFO("Relative mode reset distance = %0.2f mm\n", outputManager->settings->relativeResetDistance);
		return true;
	}));


	//
	// Command: ResetTime
	//
	// Sets relative mouse output reset distance
	//
	AddAlias("ResetTime", "RelativeResetTime");
	AddCommand(new Command("RelativeResetTime", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;
		outputManager->settings->relativeResetTime = cmd->GetDouble(0, outputManager->settings->relativeResetTime);
		LOG_INFO("Relative mode reset time = %0.2f milliseconds\n", outputManager->settings->relativeResetTime);
		return true;
	}));


}
