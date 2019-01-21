#include "precompiled.h"
#include "CommandHandler.h"

#define LOG_MODULE ""
#include "Logger.h"

//
// Create other commands
//
void CommandHandler::CreateOtherCommands() {

	//
	// Command: ScreenArea
	//
	// Sets the monitor mapping
	//
	AddCommand(new Command("ScreenArea", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;

		int index = cmd->GetInt(4, 0);

		// Validate map index
		if (index >= sizeof(mapper->screenMaps) / sizeof(ScreenMapper::ScreenMap)) {
			LOG_ERROR("Invalid map index!\n");
			return false;
		}

		ScreenMapper::Area *area = &mapper->screenMaps[index].screen;

		area->width = cmd->GetDouble(0, area->width);
		area->height = cmd->GetDouble(1, area->height);
		area->x = cmd->GetDouble(2, area->x);
		area->y = cmd->GetDouble(3, area->y);
		LOG_INFO("Screen area = (w=%0.2f, h=%0.2f, x=%0.2f, y=%0.2f)\n",
			area->width,
			area->height,
			area->x,
			area->y
		);

		mapper->UpdateValues();

		return true;
	}));

	//
	// Command: ScreenMapCount
	//
	// Sets the screen mapper map count
	//
	AddCommand(new Command("ScreenMapCount", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;

		mapper->screenMapCount = cmd->GetInt(0, mapper->screenMapCount);
		LOG_INFO("Screen map count = %d\n", mapper->screenMapCount);

		return true;
	}));



	//
	// Command: DesktopSize
	//
	// Sets the desktop area that the driver output can reach
	//
	AddAlias("Desktop", "DesktopSize");
	AddCommand(new Command("DesktopSize", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;
		mapper->virtualScreen.width = cmd->GetDouble(0, mapper->virtualScreen.width);
		mapper->virtualScreen.height = cmd->GetDouble(1, mapper->virtualScreen.height);
		LOG_INFO("Desktop size = (%0.2f px x %0.2f px)\n",
			mapper->virtualScreen.width,
			mapper->virtualScreen.height
		);
		return true;
	}));


	//
	// Command: OutputMode, Mode
	//
	// Sets driver output mode
	//
	AddAlias("Mode", "OutputMode");
	AddCommand(new Command("OutputMode", [&](CommandLine *cmd) {

		std::string mode = cmd->GetStringLower(0, "");

		// Absolute mouse
		if (mode.compare(0, 3, "abs") == 0) {
			outputManager->SetOutputMode(OutputManager::ModeVMultiAbsolute);
			LOG_INFO("Output Mode = Absolute\n");
		}

		// Relative mouse
		else if (mode.compare(0, 3, "rel") == 0) {
			outputManager->SetOutputMode(OutputManager::ModeVMultiRelative);
			LOG_INFO("Output Mode = Relative\n");
		}

		// Digitizer Absolute
		else if (mode.compare(0, 12, "digitizerabs") == 0) {

			outputManager->SetOutputMode(OutputManager::ModeVMultiDigitizer);
			LOG_INFO("Output Mode = Digitizer Absolute\n");
		}

		// Digitizer Absolute
		else if (mode.compare(0, 12, "digitizerrel") == 0) {

			outputManager->SetOutputMode(OutputManager::ModeVMultiDigitizerRelative);
			LOG_INFO("Output Mode = Digitizer Relative\n");
		}



		// SendInput Absolute
		else if (mode.compare(0, 12, "sendinputabs") == 0) {
			outputManager->SetOutputMode(OutputManager::ModeSendInputAbsolute);
			LOG_INFO("Output Mode = SendInput Absolute\n");
		}

		// SendInput Relative
		else if (mode.compare(0, 12, "sendinputrel") == 0) {
			outputManager->SetOutputMode(OutputManager::ModeSendInputRelative);
			LOG_INFO("Output Mode = SendInput Relative\n");
		}

		// Dummy
		else if (mode == "dummy") {
			outputManager->SetOutputMode(OutputManager::ModeDummy);
			LOG_INFO("Output Mode = Dummy\n");
		}

		else {
			LOG_ERROR("Unknown output mode '%s'\n", mode.c_str());
		}

		return true;
	}));


	//
	// Command: Benchmark, Bench
	//
	AddAlias("Bench", "Benchmark");
	AddCommand(new Command("Benchmark", [&](CommandLine *cmd) {

		if (!ExecuteCommand("TabletValid")) return false;

		int timeLimit;
		int reportCount = cmd->GetInt(0, 200);

		// Limit report count
		if (reportCount < 10) reportCount = 10;
		if (reportCount > 1000) reportCount = 1000;

		// Time limit
		timeLimit = reportCount * 10;
		if (timeLimit < 1000) timeLimit = 1000;

		// Log
		LOG_INFO("Tablet benchmark starting in 3 seconds!\n");
		LOG_INFO("Keep the pen stationary on top of the tablet!\n");
		Sleep(3000);
		LOG_INFO("Benchmark started!\n");

		// Start measurement
		tablet->measurement.Start(reportCount);

		// Wait for the benchmark to finish
		for (int i = 0; i < timeLimit / 100; i++) {
			Sleep(100);

			// Benchmark results
			if (tablet->measurement.reportCounter <= 0) {

				double width = tablet->measurement.maximum.x - tablet->measurement.minimum.x;
				double height = tablet->measurement.maximum.y - tablet->measurement.minimum.y;
				LOG_INFO("\n");
				LOG_INFO("Benchmark result (%d positions):\n", tablet->measurement.totalReports);
				LOG_INFO("  Tablet: %s\n", tablet->name.c_str());
				LOG_INFO("  Area: %0.2f mm x %0.2f mm (%0.0f px x %0.0f px)\n",
					mapper->primaryTabletArea->width,
					mapper->primaryTabletArea->height,
					mapper->primaryTabletArea->width,
					mapper->primaryTabletArea->height
				);
				LOG_INFO("  X range: %0.3f mm <-> %0.3f mm\n", tablet->measurement.minimum.x, tablet->measurement.maximum.x);
				LOG_INFO("  Y range: %0.3f mm <-> %0.3f mm\n", tablet->measurement.minimum.y, tablet->measurement.maximum.y);
				LOG_INFO("  Width: %0.3f mm (%0.2f px)\n",
					width,
					mapper->primaryScreenArea->width / mapper->primaryTabletArea->width * width
				);
				LOG_INFO("  Height: %0.3f mm (%0.2f px)\n",
					height,
					mapper->primaryScreenArea->height / mapper->primaryTabletArea->height* height
				);
				LOG_INFO("  Noise filter threshold: %0.2f mm\n", sqrt(width * width + height * height));


				LOG_INFO("\n");
				LOG_STATUS("BENCHMARK %d %0.3f %0.3f %s\n", tablet->measurement.totalReports, width, height, tablet->name.c_str());
				break;
			}
		}

		// Stop measurement
		tablet->measurement.Stop();

		// Benchmark failed
		if (tablet->measurement.reportCounter > 0) {
			LOG_ERROR("Benchmark failed!\n");
			LOG_ERROR("Not enough reports captured in %0.2f seconds!\n",
				timeLimit / 1000.0
			);
		}

		return true;
	}));


	//
	// Command: Measure
	//
	AddCommand(new Command("Measure", [&](CommandLine *cmd) {

		if (!ExecuteCommand("TabletValid")) return false;

		int timeLimit = 30000;
		int pointCount = cmd->GetInt(0, 1);

		// Limits
		if (pointCount < 1) pointCount = 1;
		if (pointCount > 10) pointCount = 10;

		// Log
		LOG_INFO("Measurement started!\n");

		// Start measurement
		tablet->measurement.Start();

		// Wait for the measurement to finish
		for (int i = 0; i < timeLimit / 100; i++) {
			Sleep(100);

			// Result
			if (tablet->measurement.pointCount >= pointCount) {
				double distance = 0;
				double lastX = 0;
				double lastY = 0;
				Vector2D point, lastPoint;
				Vector2D minimum;
				Vector2D maximum;

				minimum.Set(10000, 10000);
				maximum.Set(-10000, -10000);

				LOG_INFO("\n");
				LOG_INFO("Measurement results:\n");
				for (int pointIndex = 0; pointIndex < tablet->measurement.pointCount; pointIndex++) {

					point.Set(tablet->measurement.points[pointIndex]);

					// Limits
					if (point.x < minimum.x) minimum.x = point.x;
					if (point.x > maximum.x) maximum.x = point.x;
					if (point.y < minimum.y) minimum.y = point.y;
					if (point.y > maximum.y) maximum.y = point.y;

					LOG_INFO(
						"  #%d: X = %0.2f mm, Y = %0.2f mm\n",
						pointIndex + 1,
						point.x,
						point.y
					);

					// Distance calculation
					if (pointIndex > 0) {
						distance += point.Distance(lastPoint);
					}
					lastPoint.Set(point);

				}
				if (tablet->measurement.pointCount > 1) {
					LOG_INFO("  Distance: %0.2f mm\n", distance);
					LOG_INFO("  Maximum area: %0.2f mm x %0.2f mm\n",
						maximum.x - minimum.x, maximum.y - minimum.y
					);
				}

				// Status message
				char pointString[1024];
				char pointStringIndex = 0;

				// Build point string
				for (int pointIndex = 0; pointIndex < tablet->measurement.pointCount; pointIndex++) {
					point.Set(tablet->measurement.points[pointIndex]);
					pointStringIndex += sprintf_s(pointString + pointStringIndex, 1024 - pointStringIndex, "%0.3f %0.3f ", point.x, point.y);
				}

				// Output measurement results
				LOG_STATUS("MEASUREMENT %s\n", pointString);

				LOG_INFO("\n");

				break;
			}
		}

		// Stop measurement
		tablet->measurement.Stop();

		// Measurement failed
		if (tablet->measurement.pointCount < pointCount) {
			LOG_ERROR("Measurement failed! No enough points detected in %0.0f seconds!\n", timeLimit / 1000.0);
			LOG_STATUS("MEASUREMENT 0\n");
		}

		return true;
	}));


	//
	// Command: StateOutput
	//
	// Enable/disable state output
	//
	AddCommand(new Command("StateOutput", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;
		bool isOutputEnabled = false;
		pipeState->lockServer.lock();
		pipeState->isStateOutputEnabled = cmd->GetBoolean(0, pipeState->isStateOutputEnabled);
		isOutputEnabled = pipeState->isStateOutputEnabled;
		pipeState->lockServer.unlock();
		LOG_INFO("State output = %s\n", pipeState->isStateOutputEnabled ? "Enabled" : "Disabled");
		return true;
	}));


	//
	// Command: ForceLowLatencyAudio
	//
	// Forces low latency audio on Windows 10
	//
	AddCommand(new Command("ForceLowLatencyAudio", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;

		int result = tabletHandler->inputEmulator.ForceLowLatencyAudio();
		if (result > 0) {
			LOG_INFO("Low latency audio forced!\n");
		}
		else if (result == 0) {
			LOG_INFO("Low latency audio is already forced!\n");
		}
		else {
			LOG_ERROR("Low latency audio error! Maybe you are not running this on Windows 10?\n");
		}

		return true;
	}));


	//
	// Command: Debug
	//
	// Enable/disable debugging output
	//
	AddCommand(new Command("Debug", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;
		bool debugEnabled = cmd->GetBoolean(0, logger.IsDebugOutputEnabled());
		logger.SetDebugOutput(debugEnabled);
		LOG_INFO("Debug logging = %s\n", logger.IsDebugOutputEnabled() ? "True" : "False");
		return true;
	}));


	//
	// Command: Log
	//
	// Start/stop logging messages to a log file
	//
	AddCommand(new Command("Log", [&](CommandLine *cmd) {
		std::string logPath = cmd->GetString(0, "log.txt");
		if (!cmd->GetBoolean(0, true)) {
			logger.CloseLogFile();
			LOG_INFO("Log file '%s' closed.\n", logger.logFilename.c_str());
		}
		else if (logger.OpenLogFile(logPath)) {
			LOG_INFO("Log file '%s' opened.\n", logPath.c_str());
		}
		else {
			LOG_ERROR("Cant open log file!\n");
		}
		return true;
	}));


	//
	// Command: Wait
	//
	AddCommand(new Command("Wait", [&](CommandLine *cmd) {
		int waitTime = cmd->GetInt(0, 0);
		Sleep(waitTime);
		return true;
	}));


	//
	// Command: Echo
	//
	commandHandler->AddCommand(new Command("Echo", [&](CommandLine *cmd) {
		if (cmd->valueCount > 0) {
			LOG_INFO("%s\n", cmd->line.c_str() + 5);
		}
		else {
			LOG_INFO("\n");
		}
		return true;
	}));


	//
	// Command: LogDirect
	//
	// Enables/disables direct logging without queuing the messages.
	//
	AddCommand(new Command("LogDirect", [&](CommandLine *cmd) {
		logger.ProcessQueue();
		logger.directPrint = cmd->GetBoolean(0, logger.directPrint);
		logger.ProcessQueue();

		LOG_INFO("Log direct print = %s\n", logger.directPrint ? "True" : "False");
		return true;
	}));


	//
	// Command: Output
	//
	// Enables/disables VMulti output
	//
	AddCommand(new Command("Output", [&](CommandLine *cmd) {
		vmulti->outputEnabled = cmd->GetBoolean(0, vmulti->outputEnabled);
		LOG_INFO("Output enabled = %s\n", vmulti->outputEnabled ? "True" : "False");
		return true;
	}));


	//
	// Command: Information, Info
	//
	// Shows driver information
	//
	AddAlias("Info", "Information");
	AddCommand(new Command("Information", [&](CommandLine *cmd) {

		if (!ExecuteCommand("TabletValid")) return false;

		char stringBuffer[1024];
		int maxLength = sizeof(stringBuffer) - 1;
		int stringIndex = 0;

		LOG_INFO("\n");

		// VMulti type
		switch (vmulti->type) {
		case VMulti::TypeXPPen: LOG_INFO("VMulti: XP-Pen\n"); break;
		case VMulti::TypeVEIKK: LOG_INFO("VMulti: VEIKK\n"); break;
		default: break;
		}

		LOG_INFO("\n");

		//
		// Tablet
		//
		LOG_INFO("Tablet: %s\n", tablet->name.c_str());
		LOG_INFO("  Width = %0.2f mm\n", tablet->settings.width);
		LOG_INFO("  Height = %0.2f mm\n", tablet->settings.height);
		LOG_INFO("  Max X = %d\n", tablet->settings.maxX);
		LOG_INFO("  Max Y = %d\n", tablet->settings.maxY);
		LOG_INFO("  Max Pressure = %d\n", tablet->settings.maxPressure);
		LOG_INFO("  Click Pressure = %d\n", tablet->settings.clickPressure);
		LOG_INFO("  Keep Tip Down = %d reports\n", tablet->settings.keepTipDown);
		LOG_INFO("  Report Id = 0x%02X\n", tablet->settings.reportId);
		LOG_INFO("  Report Length = %d bytes\n", tablet->settings.reportLength);
		LOG_INFO("  Detect Mask = 0x%02X\n", tablet->settings.detectMask);
		LOG_INFO("  Ignore Mask = 0x%02X\n", tablet->settings.ignoreMask);


		// Pen buttons
		for (int i = 0; i < tablet->settings.buttonCount; i++) {
			stringIndex += sprintf_s(stringBuffer + stringIndex, maxLength - stringIndex, "'%s' ", tablet->settings.buttonMap[i].ToString().c_str());
		}
		LOG_INFO("  Pen button map = %s\n", stringBuffer);

		// Aux buttons
		if (tablet->settings.auxButtonCount > 0) {
			stringIndex = 0;
			for (int i = 0; i < tablet->settings.auxButtonCount; i++) {
				stringIndex += sprintf_s(stringBuffer + stringIndex, maxLength - stringIndex, "'%s' ", tablet->settings.auxButtonMap[i].ToString().c_str());
			}
			LOG_INFO("  Aux button map = %s\n", stringBuffer);
		}
		LOG_INFO("  Aux button count = %d\n", tablet->settings.auxButtonCount);


		// Init strings
		if (tablet->initStrings.size() > 0) {
			std::string stringIds = "";
			for (int stringId : tablet->initStrings) {
				stringIds += std::to_string(stringId) + " ";
			}
			LOG_INFO("  Init string ids: %s\n", stringIds.c_str());
		}

		// Init feature reports
		if (tablet->initFeatureReports.size() > 0) {
			for (Tablet::InitReport *report : tablet->initFeatureReports) {
				LOG_INFOBUFFER(report->data, report->length, "  Init feature report: ");
			}
		}

		// Init output reports
		if (tablet->initOutputReports.size() > 0) {
			for (Tablet::InitReport *report : tablet->initOutputReports) {
				LOG_INFOBUFFER(report->data, report->length, "  Init output report: ");
			}
		}

		LOG_INFO("\n");

		//
		// Areas
		//
		LOG_INFO("Area:\n");
		LOG_INFO("  Desktop = %0.0fpx x %0.0fpx\n",
			mapper->virtualScreen.width, mapper->virtualScreen.height
		);
		LOG_INFO("  Screen Map = %0.0fpx x %0.0fpx @ X%+0.0fpx, Y%+0.0fpx\n",
			mapper->primaryScreenArea->width, mapper->primaryScreenArea->height,
			mapper->primaryScreenArea->x, mapper->primaryScreenArea->y
		);
		LOG_INFO("  Tablet =  %0.2fmm x %0.2fmm @ Min(%0.2fmm, %0.2fmm) Max(%0.2fmm, %0.2fmm)\n",
			mapper->primaryTabletArea->width, mapper->primaryTabletArea->height,
			mapper->primaryTabletArea->x, mapper->primaryTabletArea->y,
			mapper->primaryTabletArea->width + mapper->primaryTabletArea->x, mapper->primaryTabletArea->height + mapper->primaryTabletArea->y
		);
		LOG_INFO("  Rotation matrix = [%f,%f,%f,%f]\n",
			mapper->primaryRotationMatrix[0],
			mapper->primaryRotationMatrix[1],
			mapper->primaryRotationMatrix[2],
			mapper->primaryRotationMatrix[3]
		);
		LOG_INFO("\n");

		return true;
	}));


	//
	// Command: RequestSettings
	//
	AddCommand(new Command("RequestSettings", [&](CommandLine *cmd) {

		// Wait for a connection
		for (int i = 0; i < 20; i++) {
			if (pipeOutput == NULL) return false;
			if (pipeOutput != NULL && pipeOutput->IsRunning() && pipeOutput->IsClientConnected()) {
				break;
			}
			Sleep(100);
		}
		if (pipeOutput != NULL && pipeOutput->IsRunning()) {
			LOG_STATUS("SETTINGS_REQUEST 1\n");
		}

		return true;
	}));


	//
	// Command: RequestStartup
	//
	AddCommand(new Command("RequestStartup", [&](CommandLine *cmd) {

		// Wait for a connection
		for (int i = 0; i < 20; i++) {
			if (pipeOutput == NULL) return false;
			if (pipeOutput != NULL && pipeOutput->IsRunning() && pipeOutput->IsClientConnected()) {
				break;
			}
			Sleep(100);
		}
		if (pipeOutput != NULL && pipeOutput->IsRunning()) {
			LOG_STATUS("STARTUP_REQUEST 1\n");
		}

		return true;
	}));


	//
	// Command: Status
	//
	// Outputs driver infor as status messages
	//
	AddCommand(new Command("Status", [&](CommandLine *cmd) {
		if (!ExecuteCommand("TabletValid")) return false;

		LOG_STATUS("TABLET %s\n", tablet->name.c_str());

		if (tablet->hidDevice != NULL) {
			LOG_STATUS("HID %04X %04X %04X %04X\n",
				tablet->hidDevice->vendorId,
				tablet->hidDevice->productId,
				tablet->hidDevice->usagePage,
				tablet->hidDevice->usage
			);
		}
		else if (tablet->usbDevice != NULL) {
			LOG_STATUS("USB %s\n", tablet->usbDevice->guid.c_str());
		}
		LOG_STATUS("WIDTH %0.5f\n", tablet->settings.width);
		LOG_STATUS("HEIGHT %0.5f\n", tablet->settings.height);
		LOG_STATUS("MAX_X %d\n", tablet->settings.maxX);
		LOG_STATUS("MAX_Y %d\n", tablet->settings.maxY);
		LOG_STATUS("MAX_PRESSURE %d\n", tablet->settings.maxPressure);
		LOG_STATUS("AUX_BUTTONS %d\n", tablet->settings.auxButtonCount);
		if (tabletHandler != NULL) {
			LOG_STATUS("STARTED %d\n", tabletHandler->IsRunning());
		}

		return true;
	}));


	//
	// Command: Include
	//
	// Reads commands from a file and stops if tablet is redefined.
	//
	AddCommand(new Command("Include", [&](CommandLine *cmd) {
		std::string filename = cmd->GetString(0, "");
		if (filename == "") {
			LOG_ERROR("Invalid filename '%s'!\n", filename.c_str());
			return false;
		}
		else {
			if (ExecuteFile(filename)) {
			}
			else {
				LOG_ERROR("Can't open file '%s'\n", filename.c_str());
				return false;
			}
		}
		return true;
	}));


	//
	// Command: ListCommands, List
	//
	// List all commands
	//
	AddAlias("List", "ListCommands");
	AddCommand(new Command("ListCommands", [&](CommandLine *cmd) {

		LOG_INFO("Commands: \n");
		for (std::pair<std::string, Command*> cmdPair : commands) {
			std::string commandName = cmdPair.second->name;
			std::string lowerCaseName = commandName;
			std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(), ::tolower);

			std::string aliasString = "";
			for (std::pair<std::string, std::string> aliasPair : aliases) {
				if (aliasPair.second == lowerCaseName) {
					aliasString += ", " + aliasNames[aliasPair.first];
				}
			}
			LOG_INFO("  %s%s\n", commandName.c_str(), aliasString.c_str());
		}

		return true;
	}));


	//
	// Command: GetCommands
	//
	// Outputs all commands as status messages
	//
	AddCommand(new Command("GetCommands", [&](CommandLine *cmd) {

		std::string commandsString = "";

		LOG_STATUS("COMMANDS_CLEAR 1\n");

		for (std::pair<std::string, Command*> cmdPair : commands) {
			std::string commandName = cmdPair.second->name;
			std::string lowerCaseName = commandName;
			std::transform(lowerCaseName.begin(), lowerCaseName.end(), lowerCaseName.begin(), ::tolower);

			commandsString += commandName + " ";

			std::string aliasString = "";
			for (std::pair<std::string, std::string> aliasPair : aliases) {
				if (aliasPair.second == lowerCaseName) {
					commandsString += aliasNames[aliasPair.first] + " ";
				}
			}

			if (commandsString.size() > 80) {
				LOG_STATUS("COMMANDS %s\n", commandsString.c_str());
				commandsString = "";
			}
		}
		if (commandsString.size() > 0) {
			LOG_STATUS("COMMANDS %s\n", commandsString.c_str());
		}

		return true;
	}));

	//
	// Command: ListInputs
	//
	// List all input actions
	//
	AddCommand(new Command("ListInputs", [&](CommandLine *cmd) {

		LOG_INFO("Input actions:\n");
		for (std::string key : tabletHandler->inputEmulator.inputs) {
			if (tabletHandler->inputEmulator.inputMap.count(key) > 0) {
				std::string name = tabletHandler->inputEmulator.inputMap[key]->description;
				LOG_INFO("  % -20s %s\n", key.c_str(), name.c_str());
			}
		}

		return true;
	}));


	//
	// Command: Help
	//
	// Shows help for a command
	//
	AddHelp("Help", "...");
	AddCommand(new Command("Help", [&](CommandLine *cmd) {
		std::string commandName = cmd->GetStringLower(0, "");
		if (aliases.count(commandName) > 0) {
			commandName = aliases[commandName];
		}
		if (help.count(commandName)) {
			std::string helpText = help[commandName];
			LOG_INFO("Help:\n\n%s\n", helpText.c_str());
		}
		else {
			LOG_INFO("No help found :(\n");
		}

		return true;
	}));

}
