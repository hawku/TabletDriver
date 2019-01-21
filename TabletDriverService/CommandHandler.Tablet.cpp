#include "precompiled.h"
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
		if (tablet == NULL) {
			return false;
		}
		return true;
	}));


	//
	// Command: CheckTablet
	//
	AddCommand(new Command("CheckTablet", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) {
			LOG_ERROR("Tablet not found!\n");
			LOG_ERROR("Check the list of supported tablets from the GitHub page.\n");
			LOG_ERROR("http://github.com/hawku/TabletDriver\n");
			CleanupAndExit(0);
			return false;
		}
		return true;
	}));


	//
	// Command: TabletName, Name
	//
	AddAlias("Name", "TabletName");
	AddCommand(new Command("TabletName", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->name = cmd->GetString(0, tablet->name);
		LOG_INFO("Tablet name = '%s'\n", tablet->name.c_str());
		return true;
	}));


	//
	// Command: ReportId
	//
	AddCommand(new Command("ReportId", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.reportId = cmd->GetInt(0, tablet->settings.reportId);
		LOG_INFO("Tablet report id = %d\n", tablet->settings.reportId);
		return true;
	}));


	//
	// Command: ReportLength
	//
	AddCommand(new Command("ReportLength", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.reportLength = cmd->GetInt(0, tablet->settings.reportLength);
		LOG_INFO("Tablet report length = %d\n", tablet->settings.reportLength);
		return true;
	}));


	//
	// Command: DetectMask
	//
	AddCommand(new Command("DetectMask", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.detectMask = cmd->GetInt(0, tablet->settings.detectMask);
		LOG_INFO("Tablet detect mask = 0x%02X\n", tablet->settings.detectMask);
		return true;
	}));


	//
	// Command: IgnoreMask
	//
	AddCommand(new Command("IgnoreMask", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.ignoreMask = cmd->GetInt(0, tablet->settings.ignoreMask);
		LOG_INFO("Tablet ignore mask = 0x%02X\n", tablet->settings.ignoreMask);
		return true;
	}));


	//
	// Command: MaxX
	//
	AddCommand(new Command("MaxX", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.maxX = cmd->GetInt(0, tablet->settings.maxX);
		LOG_INFO("Tablet maximum X value = %d\n", tablet->settings.maxX);
		return true;
	}));


	//
	// Command: MaxY
	//
	AddCommand(new Command("MaxY", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.maxY = cmd->GetInt(0, tablet->settings.maxY);
		LOG_INFO("Tablet maximum Y value = %d\n", tablet->settings.maxY);
		return true;
	}));


	//
	// Command: MaxPressure
	//
	AddCommand(new Command("MaxPressure", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.maxPressure = cmd->GetInt(0, tablet->settings.maxPressure);
		LOG_INFO("Tablet maximum pressure value = %d\n", tablet->settings.maxPressure);
		return true;
	}));


	//
	// Command: MaxHeight
	//
	AddCommand(new Command("MaxHeight", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.maxHeight = cmd->GetInt(0, tablet->settings.maxHeight);
		LOG_INFO("Tablet maximum height value = %d\n", tablet->settings.maxHeight);
		return true;
	}));


	//
	// Command: ClickPressure
	//
	AddCommand(new Command("ClickPressure", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.clickPressure = cmd->GetInt(0, tablet->settings.clickPressure);
		LOG_INFO("Tablet click pressure = %d\n", tablet->settings.clickPressure);
		return true;
	}));


	//
	// Command: KeepTipDown
	//
	AddCommand(new Command("KeepTipDown", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.keepTipDown = cmd->GetInt(0, tablet->settings.keepTipDown);
		LOG_INFO("Tablet pen tip keep down = %d reports\n", tablet->settings.keepTipDown);
		return true;
	}));


	//
	// Command: Width
	//
	AddCommand(new Command("Width", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.width = cmd->GetDouble(0, tablet->settings.width);
		LOG_INFO("Tablet width = %0.2f mm\n", tablet->settings.width);
		return true;
	}));


	//
	// Command: Height
	//
	AddCommand(new Command("Height", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.height = cmd->GetDouble(0, tablet->settings.height);
		LOG_INFO("Tablet height = %0.2f mm\n", tablet->settings.height);
		return true;
	}));


	//
	// Command: PressureSensitivity
	//
	AddCommand(new Command("PressureSensitivity", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.pressureSensitivity = cmd->GetDouble(0, tablet->settings.pressureSensitivity);
		LOG_INFO("Tablet pressure sensitivity = %0.5f\n", tablet->settings.pressureSensitivity);
		return true;
	}));


	//
	// Command: PressureDeadzone
	//
	AddCommand(new Command("PressureDeadzone", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.pressureDeadzoneLow = cmd->GetDouble(0, tablet->settings.pressureDeadzoneLow);
		tablet->settings.pressureDeadzoneHigh = cmd->GetDouble(1, tablet->settings.pressureDeadzoneHigh);
		LOG_INFO("Tablet pressure deadzone = low:%0.2f high:%0.2f\n",
			tablet->settings.pressureDeadzoneLow,
			tablet->settings.pressureDeadzoneHigh
		);
		return true;
	}));


	//
	// Command: ScrollSensitivity
	//
	AddCommand(new Command("ScrollSensitivity", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.scrollSensitivity = cmd->GetDouble(0, tablet->settings.scrollSensitivity);
		LOG_INFO("Tablet scroll sensitivity = %0.2f scrolls per millimeter\n", tablet->settings.scrollSensitivity);
		return true;
	}));


	//
	// Command: ScrollAcceleration, ScrollAcc
	//
	AddAlias("ScrollAcc", "ScrollAcceleration");
	AddCommand(new Command("ScrollAcceleration", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.scrollAcceleration = cmd->GetDouble(0, tablet->settings.scrollAcceleration);
		if (tablet->settings.scrollAcceleration < 0.1) tablet->settings.scrollAcceleration = 0.1;
		LOG_INFO("Tablet scroll acceleration = %0.2f\n", tablet->settings.scrollAcceleration);
		return true;
	}));


	//
	// Command: ScrollDrag
	//
	AddCommand(new Command("ScrollDrag", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.scrollDrag = cmd->GetBoolean(0, tablet->settings.scrollDrag);
		LOG_INFO("Tablet scroll drag = %s\n", tablet->settings.scrollDrag ? "true" : "false");
		return true;
	}));


	//
	// Command: ScrollStopCursor
	//
	AddCommand(new Command("ScrollStopCursor", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->settings.scrollStopCursor = cmd->GetBoolean(0, tablet->settings.scrollStopCursor);
		LOG_INFO("Tablet scroll stop cursor = %s\n", tablet->settings.scrollStopCursor ? "true" : "false");
		return true;
	}));



	//
	// Command: Skew
	//
	AddCommand(new Command("Skew", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
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
		if (tablet == NULL) return false;
		std::string format = cmd->GetStringLower(0, "");

		// Wacom Intuos Format V2 (Wacom CTL-490)
		if (format == "wacomintuosv2") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatWacomIntuosV2;
		}

		// Wacom Intuos Format V3 (Wacom CTL-4100)
		else if (format == "wacomintuosv3" || format == "wacom4100") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatWacomIntuosV3;
		}

		// Wacom Drivers
		else if (format == "wacomdrivers") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatWacomDrivers;
		}

		// Skip first data byte (VEIKK S640)
		else if (format == "skipfirstdatabyte") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatSkipFirstDataByte;
		}

		// Custom data format
		else if (format == "custom") {
			tablet->settings.dataFormat = TabletSettings::TabletFormatCustom;
		}

		// Unknown type
		else {
			LOG_ERROR("Unknown tablet data format: %s\n", format.c_str());
		}

		LOG_INFO("Tablet format = %d\n", tablet->settings.dataFormat);

		return true;
	}));


	//
	// Command: CustomDataInstruction, CustomData
	//
	// Sets tablet data format
	//
	// CustomData <target byte> [TargetMask=<target mask(&)>] [Source=<source byte>] [SourceMask=<source mask(&)>] [SourceShift=<bit shift (<<)>]
	//
	AddAlias("CustomData", "CustomDataInstruction");
	AddAlias("AuxCustomData", "CustomDataInstruction");
	AddCommand(new Command("CustomDataInstruction", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;

		std::string commandName = cmd->GetCommandLowerCase();

		if (cmd->valueCount >= 3) {

			DataFormatter::DataInstruction instruction;

			int targetByte = cmd->GetInt(0, 0);
			std::string targetByteName = cmd->GetStringLower(0, "");
			/*
			int targetBitMask = cmd->GetInt(1, 0xFF);
			int sourceByte = cmd->GetInt(2, 0);
			int sourceBitMask = cmd->GetInt(3, 0xFF);
			int sourceBitShift = cmd->GetInt(4, 0);
			*/

			// Auxiliary byte names
			if (commandName.compare(0, 3, "aux") == 0) {
				if (tablet->auxReportByteNames.count(targetByteName) > 0) {
					targetByte = tablet->auxReportByteNames.at(targetByteName);
				}
			}

			// Tablet byte names
			else {
				if (tablet->reportByteNames.count(targetByteName) > 0) {
					targetByte = tablet->reportByteNames.at(targetByteName);
				}
			}


			instruction.targetByte = targetByte;
			instruction.writeMode = DataFormatter::DataInstruction::WriteModeOR;

			// Loop through parameters
			for (int i = 1; i < cmd->valueCount; i += 2) {
				std::string parameter = cmd->GetStringLower(i, "");
				int value = cmd->GetInt(i + 1, -1);
				std::string stringValue = cmd->GetStringLower(i + 1, "");

				// Set instruction parameters
				if (parameter == "targetmask" || parameter == "mask") instruction.targetBitMask = value;
				if (parameter == "source") instruction.sourceByte = value;
				if (parameter == "sourcemask") instruction.sourceBitMask = value;
				if (parameter == "sourceshift" || parameter == "shift") instruction.sourceBitShift = value;
				if (parameter == "writemode") {
					if (stringValue == "or") instruction.writeMode = DataFormatter::DataInstruction::WriteModeOR;
					else if (stringValue == "and") instruction.writeMode = DataFormatter::DataInstruction::WriteModeAND;
					else if (stringValue == "set") instruction.writeMode = DataFormatter::DataInstruction::WriteModeSet;
				}

			}


			// Add aux data format instruction
			if (commandName.compare(0, 3, "aux") == 0) {
				tablet->settings.auxCurrentReport->formatter.AddInstruction(&instruction);
			}
			// Add tablet data format instruction
			else {
				tablet->dataFormatter.AddInstruction(&instruction);
			}


			//
			LOG_INFO("Custom data format: Target=%d, TargetMask=0x%02X, Source=%d, SourceMask=0x%02X, SourceShift=%d WriteMode=%d\n",
				instruction.targetByte,
				instruction.targetBitMask,
				instruction.sourceByte,
				instruction.sourceBitMask,
				instruction.sourceBitShift,
				instruction.writeMode
			);

		}
		else {
			LOG_INFO("Usage: %s <target byte> [TargetMask=<target mask(&)>] [Source=<source byte>] [SourceMask=<source mask(&)>] [SourceShift=<bit shift (<<)>]\n",
				cmd->command.c_str()
			);
		}

		return true;
	}));


	//
	// Command: ClearCustomData
	//
	// Clears custom data formatter instructions
	//
	AddCommand(new Command("ClearCustomData", [&](CommandLine *cmd) {
		if (tablet == NULL) return false;
		tablet->dataFormatter.instructionCount = 0;
		LOG_INFO("Custom data format instructions cleared!\n");
		return true;
	}));


	//
	// Command: InitFeatureReport, InitFeature
	//
	AddAlias("InitFeature", "InitFeatureReport");
	AddCommand(new Command("InitFeatureReport", [&](CommandLine *cmd) {
		if (cmd->valueCount <= 0) return false;
		if (tablet == NULL) return false;

		Tablet::InitReport *report = new Tablet::InitReport(cmd->valueCount);
		for (int i = 0; i < (int)cmd->valueCount; i++) {
			report->data[i] = cmd->GetInt(i, 0);
		}
		tablet->initFeatureReports.push_back(report);


		LOG_INFOBUFFER(report->data, report->length, "Tablet init feature report: ");
		return true;
	}));


	//
	// Command: InitOutputReport, InitReport
	//
	// Initialization HID output report
	//
	AddAlias("InitReport", "InitOutputReport");
	AddCommand(new Command("InitOutputReport", [&](CommandLine *cmd) {
		if (cmd->valueCount <= 0) return false;
		if (tablet == NULL) return false;

		Tablet::InitReport *report = new Tablet::InitReport(cmd->valueCount);
		for (int i = 0; i < (int)cmd->valueCount; i++) {
			report->data[i] = cmd->GetInt(i, 0);
		}
		tablet->initOutputReports.push_back(report);

		LOG_INFOBUFFER(report->data, report->length, "Tablet init output report: ");

		return true;
	}));


	//
	// Command: InitStrings, InitString
	//
	AddAlias("InitString", "InitStrings");
	AddCommand(new Command("InitStrings", [&](CommandLine *cmd) {
		if (cmd->valueCount <= 0) return false;
		if (tablet == NULL) return false;
		for (int i = 0; i < (int)cmd->valueCount; i++) {
			int stringId = cmd->GetInt(i, 0);
			if (stringId > 0 && stringId <= 256) {
				tablet->initStrings.push_back(stringId);
			}
		}
		std::string stringIdsString = "";
		for (int stringId : tablet->initStrings) {
			stringIdsString += std::to_string(stringId) + " ";
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

		if (!ExecuteCommand("TabletValid")) return false;

		int index = cmd->GetInt(4, 0);

		// Validate map index
		if (index >= sizeof(mapper->screenMaps) / sizeof(ScreenMapper::ScreenMap)) {
			LOG_ERROR("Invalid map index!\n");
			return false;
		}

		ScreenMapper::Area *area = &mapper->screenMaps[index].tablet;

		area->width = cmd->GetDouble(0, area->width);
		area->height = cmd->GetDouble(1, area->height);
		area->x = cmd->GetDouble(2, area->x);
		area->y = cmd->GetDouble(3, area->y);

		LOG_INFO("Tablet area = (%0.2f mm x %0.2f mm X+%0.2f mm Y+%0.2f mm)\n",
			area->width,
			area->height,
			area->x,
			area->y
		);

		mapper->UpdateValues();

		return true;
	}));


	//
	// Command: ClearButtonMap
	//
	AddCommand(new Command("ClearButtonMap", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;

		// Set input emulator
		tablet->settings.SetInputEmulator(&tabletHandler->inputEmulator);

		for (InputEmulator::InputActionCollection &collection : tablet->settings.buttonMap) {
			collection.Clear();
		}
		LOG_INFO("Pen button map cleared!\n");
		return true;
	}));


	//
	// Command: ClearAuxButtonMap
	//
	AddCommand(new Command("ClearAuxButtonMap", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;

		// Set input emulator
		tablet->settings.SetInputEmulator(&tabletHandler->inputEmulator);

		for (InputEmulator::InputActionCollection &collection : tablet->settings.auxButtonMap) {
			collection.Clear();
		}
		LOG_INFO("Aux button map cleared!\n");
		return true;
	}));


	//
	// Command: ButtonMap, Buttons, AuxButtonMap, AuxButtons
	//
	// Maps input buttons to output buttons
	//
	AddAlias("Buttons", "ButtonMap");
	AddAlias("AuxButtonMap", "ButtonMap");
	AddAlias("AuxButtons", "ButtonMap");
	AddCommand(new Command("ButtonMap", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;

		std::string commandName = cmd->GetCommandLowerCase();
		int button = cmd->GetInt(0, 0);

		// Value count and button valid?
		if (cmd->valueCount >= 2 && button >= 1) {

			InputEmulator::InputActionCollection *collection;

			// Set input emulator
			tablet->settings.SetInputEmulator(&tabletHandler->inputEmulator);

			// Aux buttons
			if (commandName.compare(0, 3, "aux") == 0) {
				if (button <= tablet->settings.auxButtonCount) {
					collection = &tablet->settings.auxButtonMap[button - 1];
					collection->Clear();
					for (int i = 1; i < cmd->valueCount; i++) {
						collection->Add(cmd->GetString(i, ""));
					}
					LOG_INFO("Aux button %d mapped to '%s'\n", button, collection->ToString().c_str());
				}
				else {
					LOG_INFO("There are only %d auxiliary buttons!\n", tablet->settings.auxButtonCount);
				}
			}

			// Pen buttons
			else {
				if (button <= tablet->settings.buttonCount) {
					collection = &tablet->settings.buttonMap[button - 1];
					collection->Clear();
					for (int i = 1; i < cmd->valueCount; i++) {
						collection->Add(cmd->GetString(i, ""));
					}
					LOG_INFO("Pen button %d mapped to '%s'\n", button, collection->ToString().c_str());
				}
				else {
					LOG_INFO("There are only %d buttons!\n", tablet->settings.buttonCount);
				}

			}
		}
		else {
			LOG_INFO("Usage: %s <button> <action> [<action2> ...]\n", cmd->command.c_str());
		}

		return true;
	}));


	//
	// Command: TabletMove
	//
	// Move tablet area offset from the area borders
	//
	AddAlias("Move", "TabletMove");
	AddCommand(new Command("TabletMove", [&](CommandLine *cmd) {
		if (cmd->valueCount <= 0) return false;
		if (!ExecuteCommand("TabletValid")) return false;
		std::string border;
		double offset;
		bool moved = true;

		for (int i = 0; i < cmd->valueCount; i += 2) {

			border = cmd->GetStringLower(i, "");
			offset = cmd->GetDouble(i + 1, 0.0);
			moved = true;

			if (border == "top") {
				mapper->primaryTabletArea->y = offset + mapper->primaryTabletArea->height / 2.0;
			}
			else if (border == "bottom") {
				mapper->primaryTabletArea->y = tablet->settings.height - mapper->primaryTabletArea->height / 2.0 - offset;
			}
			else if (border == "left") {
				mapper->primaryTabletArea->x = offset + mapper->primaryTabletArea->width / 2.0;;
			}
			else if (border == "right") {
				mapper->primaryTabletArea->x = tablet->settings.width - mapper->primaryTabletArea->width / 2.0 - offset;
			}
			else {
				moved = false;
			}
			if (moved) {
				LOG_INFO("Tablet area moved to %s border with %0.2f mm margin.\n", border.c_str(), offset);

				ScreenMapper::Area *area = mapper->primaryTabletArea;
				LOG_INFO("Tablet area = (%0.2f mm x %0.2f mm X+%0.2f mm Y+%0.2f mm)\n",
					area->width,
					area->height,
					area->x,
					area->y
				);
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
		if (!ExecuteCommand("TabletValid")) return false;

		int index = cmd->GetInt(1, 0);

		// Validate map index
		if (index >= sizeof(mapper->screenMaps) / sizeof(ScreenMapper::ScreenMap)) {
			LOG_ERROR("Invalid map index!\n");
			return false;
		}

		ScreenMapper::ScreenMap *screenMap = &mapper->screenMaps[index];

		double value = cmd->GetDouble(0, 0);
		screenMap->SetRotation(value);
		LOG_INFO("Rotation matrix = [%f,%f,%f,%f]\n",
			screenMap->rotationMatrix[0],
			screenMap->rotationMatrix[1],
			screenMap->rotationMatrix[2],
			screenMap->rotationMatrix[3]
		);

		mapper->UpdateValues();

		return true;
	}));


	//
	// Command: RelativeSensitivity, Sensitivity
	//
	// Sets the relative mouse output sensitivity
	//
	AddAlias("Sensitivity", "RelativeSensitivity");
	AddCommand(new Command("RelativeSensitivity", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;
		outputManager->settings->relativeSensitivity.x = cmd->GetDouble(0, outputManager->settings->relativeSensitivity.x);
		outputManager->settings->relativeSensitivity.y = cmd->GetDouble(1, outputManager->settings->relativeSensitivity.y);

		if (cmd->valueCount == 1) {
			outputManager->settings->relativeSensitivity.y = outputManager->settings->relativeSensitivity.x;
		}

		LOG_INFO("Relative mode sensitivity = X=%0.2f px/mm, Y=%0.2f px/mm\n",
			outputManager->settings->relativeSensitivity.x,
			outputManager->settings->relativeSensitivity.y
		);
		return true;
	}));

	//
	// Command: RelativeDragMove
	//
	// Sets if relative mouse position only moves when pen tip is pressed down
	//
	AddCommand(new Command("RelativeDragMove", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;
		outputManager->settings->relativeDragMove = cmd->GetBoolean(0, outputManager->settings->relativeDragMove);
		LOG_INFO("Relative mode drag move = %s\n",
			outputManager->settings->relativeDragMove ? "true" : "false"
		);
		return true;
	}));


	//
	// Command: ResetDistance
	//
	// Sets relative mouse output reset distance
	//
	AddAlias("ResetDistance", "RelativeResetDistance");
	AddCommand(new Command("RelativeResetDistance", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;
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
		if (!ExecuteCommand("TabletValid")) return false;
		outputManager->settings->relativeResetTime = cmd->GetDouble(0, outputManager->settings->relativeResetTime);
		LOG_INFO("Relative mode reset time = %0.2f milliseconds\n", outputManager->settings->relativeResetTime);
		return true;
	}));

}
