#include "stdafx.h"
#include "CommandHandler.h"

#define LOG_MODULE ""
#include "Logger.h"


//
// Constructor
//
CommandHandler::CommandHandler() {
}


//
// Destructor
//
CommandHandler::~CommandHandler() {
}



//
// Add command
//
bool CommandHandler::AddCommand(Command *command) {
	string name = command->name;
	transform(name.begin(), name.end(), name.begin(), ::tolower);
	if(commands.count(name) <= 0) {
		commands.insert(pair<string, Command*>(name, command));
		return true;
	}
	return false;
}

//
// Add command alias
//
bool CommandHandler::AddAlias(string alias, string commandName) {
	transform(alias.begin(), alias.end(), alias.begin(), ::tolower);
	transform(commandName.begin(), commandName.end(), commandName.begin(), ::tolower);

	if(aliases.count(alias) <= 0) {
		aliases.insert(pair<string, string>(alias, commandName));
		return true;
	}

	return false;
}

bool CommandHandler::ExecuteCommand(string command, string parameters) {
	CommandLine *cmd = new CommandLine(command + " " + parameters);
	bool result = ExecuteCommand(command, cmd);
	delete cmd;
	return result;
}

//
// Create commands
//
void CommandHandler::CreateCommands() {

	//
	// Command: Tablet
	//
	AddCommand(new Command("Tablet", [&](CommandLine *cmd) {

		// USB Tablet
		if(cmd->valueCount == 3) {
			string guid = cmd->GetString(0, "");
			int stringId = cmd->GetInt(1, 0);
			string stringMatch = cmd->GetString(2, "");
			if(tablet == NULL) {
				tablet = new Tablet(guid, stringId, stringMatch);
				if(tablet->isOpen) {
					LOG_INFO("Tablet found!\n");
				} else {
					LOG_WARNING("Can't open USB tablet '%s' %d '%s'\n", guid.c_str(), stringId, stringMatch.c_str());
					delete tablet;
					tablet = NULL;
				}
			}
		}

		// HID Tablet
		else if(cmd->valueCount == 4) {
			USHORT vendorID = cmd->GetInt(0, 0);
			USHORT productID = cmd->GetInt(1, 0);
			USHORT usagePage = cmd->GetInt(2, 0);
			USHORT usage = cmd->GetInt(3, 0);
			if(tablet == NULL) {
				tablet = new Tablet(vendorID, productID, usagePage, usage);
				if(tablet->isOpen) {
					LOG_INFO("Tablet found!\n");
				} else {
					LOG_WARNING("Can't open HID tablet 0x%04X 0x%04X 0x%04X 0x%04X\n", vendorID, productID, usagePage, usage);
					delete tablet;
					tablet = NULL;
				}
			}
		} else {
			if(tablet != NULL) {
				if(tablet->usbDevice != NULL) {
					USBDevice *usb = tablet->usbDevice;
					LOG_INFO("Tablet = USB(\"%s\", %d, \"%s\")\n",
						usb->guid.c_str(),
						usb->stringId,
						usb->stringMatch.c_str()
					);
				} else if(tablet->hidDevice != NULL) {
					HIDDevice *hid = tablet->hidDevice;
					LOG_INFO("Tablet = HID(0x%04X 0x%04X 0x%04X 0x%04X)\n",
						hid->vendorId,
						hid->productId,
						hid->usagePage,
						hid->usage
					);
				}
			} else {
				LOG_INFO("Tablet is not defined.\n");
			}
		}

		return true;
	}));


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
				} else {
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

	if(false) {

	}


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
	// Command: TabletType, Type
	//
	AddAlias("Type", "TabletType");
	AddCommand(new Command("TabletType", [&](CommandLine *cmd) {
		if(tablet == NULL) return false;

		// Wacom Intuos (490)
		if(cmd->GetStringLower(0, "") == "wacomintuos") {
			tablet->settings.type = TabletSettings::TypeWacomIntuos;
		}

		// Wacom CTL-4100
		else if(cmd->GetStringLower(0, "") == "wacom4100") {
			tablet->settings.type = TabletSettings::TypeWacom4100;
		}

		// Wacom Drivers
		else if(cmd->GetStringLower(0, "") == "wacomdrivers") {
			tablet->settings.type = TabletSettings::TypeWacomDrivers;
		}

		LOG_INFO("Tablet type = %d\n", tablet->settings.type);

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
		LOG_INFOBUFFER(tablet->initFeature, tablet->initFeatureLength, "Tablet init feature report: ");
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
		LOG_INFOBUFFER(tablet->initReport, tablet->initReportLength, "Tablet init report: ");

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
	// Command: LogTabletArea
	//
	// Prints current tablet area
	//
	AddCommand(new Command("LogTabletArea", [&](CommandLine *cmd) {
		LOG_INFO("%s: (%0.2f mm x %0.2f mm X+%0.2f mm Y+%0.2f mm)\n",
			cmd->GetParameterString().c_str(),
			mapper->areaTablet.width,
			mapper->areaTablet.height,
			mapper->areaTablet.x,
			mapper->areaTablet.y
		);
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
	// Command: ScreenArea
	//
	// Sets the monitor mapping
	//
	AddCommand(new Command("ScreenArea", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;
		mapper->areaScreen.width = cmd->GetDouble(0, mapper->areaScreen.width);
		mapper->areaScreen.height = cmd->GetDouble(1, mapper->areaScreen.height);
		mapper->areaScreen.x = cmd->GetDouble(2, mapper->areaScreen.x);
		mapper->areaScreen.y = cmd->GetDouble(3, mapper->areaScreen.y);
		LOG_INFO("Screen area = (w=%0.2f, h=%0.2f, x=%0.2f, y=%0.2f)\n",
			mapper->areaScreen.width,
			mapper->areaScreen.height,
			mapper->areaScreen.x,
			mapper->areaScreen.y
		);
		return true;
	}));


	//
	// Command: DesktopSize
	//
	// Sets the desktop area that the driver output can reach
	//
	AddAlias("Desktop", "DesktopSize");
	AddCommand(new Command("DesktopSize", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;
		mapper->areaVirtualScreen.width = cmd->GetDouble(0, mapper->areaVirtualScreen.width);
		mapper->areaVirtualScreen.height = cmd->GetDouble(1, mapper->areaVirtualScreen.height);
		LOG_INFO("Desktop size = (%0.2f px x %0.2f px)\n",
			mapper->areaVirtualScreen.width,
			mapper->areaVirtualScreen.height
		);
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
				mapper->areaTablet.y = offset;
			} else if(border == "bottom") {
				mapper->areaTablet.y = tablet->settings.height - mapper->areaTablet.height - offset;
			} else if(border == "left") {
				mapper->areaTablet.x = offset;
			} else if(border == "right") {
				mapper->areaTablet.x = tablet->settings.width - mapper->areaTablet.width - offset;
			} else {
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
	// Command: Sensitivity
	//
	// Sets the relative mouse output sensitivity
	//
	AddCommand(new Command("Sensitivity", [&](CommandLine *cmd) {
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
	AddCommand(new Command("ResetDistance", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;
		outputManager->settings->relativeResetDistance = cmd->GetDouble(0, outputManager->settings->relativeResetDistance);
		LOG_INFO("Relative mode reset distance = %0.2f mm\n", outputManager->settings->relativeResetDistance);
		return true;
	}));


	//
	// Command: OutputMode, Mode
	//
	// Sets driver output mode
	//
	AddAlias("Mode", "OutputMode");
	AddCommand(new Command("OutputMode", [&](CommandLine *cmd) {

		string mode = cmd->GetStringLower(0, "");

		// Absolute mouse
		if(mode.compare(0, 3, "abs") == 0) {
			outputManager->SetOutputMode(OutputManager::ModeVMultiAbsolute);
			LOG_INFO("Output Mode = Absolute\n");
		}

		// Relative mouse
		else if(mode.compare(0, 3, "rel") == 0) {
			outputManager->SetOutputMode(OutputManager::ModeVMultiRelative);
			LOG_INFO("Output Mode = Relative\n");
		}

		// Digitizer
		else if(mode.compare(0, 3, "dig") == 0 || mode.compare(0, 3, "pen") == 0) {

			outputManager->SetOutputMode(OutputManager::ModeVMultiDigitizer);
			LOG_INFO("Output Mode = Digitizer\n");
		}

		// SendInput
		else if(mode.compare(0, 12, "sendinputabs") == 0) {
			outputManager->SetOutputMode(OutputManager::ModeSendInputAbsolute);
			LOG_INFO("Output Mode = SendInput Absolute\n");
		}

		// Dummy
		else if(mode == "dummy") {
			outputManager->SetOutputMode(OutputManager::ModeDummy);
			LOG_INFO("Output Mode = Dummy\n");
		}

		else {
			LOG_ERROR("Unknown output mode '%s'\n", mode.c_str());
		}

		return true;
	}));


	//
	// Command: SmoothingFilter, Smoothing
	//
	// Sets smoothing filter parameters
	//
	AddAlias("Smoothing", "SmoothingFilter");
	AddCommand(new Command("SmoothingFilter", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;

		double latency = cmd->GetDouble(0, tablet->smoothing.GetLatency());
		double threshold = cmd->GetDouble(1, tablet->smoothing.threshold * 100);

		threshold /= 100;

		string stringValue = cmd->GetStringLower(0, "");

		// Off / False
		if(stringValue == "off" || stringValue == "false") {
			latency = 0;
		}

		// Limits
		if(latency < 0) latency = 1;
		if(latency > 1000) latency = 1000;
		if(threshold < 0.1) threshold = 0.1;
		if(threshold > 0.99) threshold = 0.99;

		// Set threshold
		tablet->smoothing.threshold = threshold;

		// Set smoothing filter latency
		tablet->smoothing.SetLatency(latency);

		// Print output
		if(tablet->smoothing.weight < 1.0) {
			tablet->smoothing.isEnabled = true;
			LOG_INFO("Smoothing = %0.2f ms to reach %0.0f%% (weight = %f)\n", latency, tablet->smoothing.threshold * 100, tablet->smoothing.weight);
		} else {
			tablet->smoothing.isEnabled = false;
			LOG_INFO("Smoothing = off\n");
		}

		return true;
	}));
	


	//
	// Command: GravityFilter, Gravity
	//
	// Sets gravity filter parameters
	//
	AddAlias("Gravity", "GravityFilter");
	AddCommand(new Command("GravityFilter", [&](CommandLine *cmd) {

		if(!ExecuteCommand("TabletValid")) return false;
		double gravity = cmd->GetDouble(0, tablet->gravityFilter.gravity);
		double friction = cmd->GetDouble(1, 0);
		double pressureGravity = cmd->GetDouble(2, 0);
		double pressureFriction = cmd->GetDouble(3, 0);


		// Limits
		if(gravity < 0) gravity = 0.0;
		if(gravity > 100) gravity = 100;
		if(friction < 0.1) friction = 0.1;
		if(friction > 1000) friction = 1000;

		if(gravity > 0.1) {
			tablet->gravityFilter.isEnabled = true;
			tablet->gravityFilter.gravity = gravity;
			tablet->gravityFilter.friction = friction;
			tablet->gravityFilter.pressureGravity = pressureGravity;
			tablet->gravityFilter.pressureFriction = pressureFriction;
		} else {
			tablet->gravityFilter.isEnabled = false;
			tablet->gravityFilter.gravity = 0;
			tablet->gravityFilter.friction = 0;
			tablet->gravityFilter.pressureGravity = 0;
			tablet->gravityFilter.pressureFriction = 0;

		}

		LOG_INFO("Gravity = %s (%0.2f gravity, %0.2f friction, %0.2f pressure gravity, %0.2f pressure friction)\n",
			tablet->gravityFilter.isEnabled ? "True" : "False",
			tablet->gravityFilter.gravity,
			tablet->gravityFilter.friction,
			tablet->gravityFilter.pressureGravity,
			tablet->gravityFilter.pressureFriction
		);
		return true;
	}));
	


	//
	// Command: FilterTimerInterval
	//
	// Sets filter timer interval
	//
	AddCommand(new Command("FilterTimerInterval", [&](CommandLine *cmd) {
		if(tabletHandler == NULL) return true;
		int oldInterval = (int)round(tabletHandler->timerInterval);

		int interval = cmd->GetInt(0, oldInterval);

		// 10 Hz
		if(interval > 100) interval = 100;

		// 1000 Hz
		if(interval < 1) interval = 1;

		// Change interval
		tabletHandler->ChangeTimerInterval(interval);

		LOG_INFO("Filter Timer Interval = %0.0f (%0.2f Hz)\n",
			tabletHandler->timerInterval,
			1000.0 / tabletHandler->timerInterval
		);
		return true;
	}));



	//
	// Command: NoiseReduction, Noise
	//
	// Sets noise reduction filter parameters
	//
	AddAlias("Noise", "NoiseReduction");
	AddCommand(new Command("NoiseReduction", [&](CommandLine *cmd) {

		string stringValue = cmd->GetStringLower(0, "");

		// Off / False
		if(stringValue == "off" || stringValue == "false") {
			tablet->noiseFilter.isEnabled = false;
			LOG_INFO("Noise Reduction = off\n");

		// Enable
		} else {

			// Position buffer length
			int length = cmd->GetInt(0, tablet->noiseFilter.buffer.length);

			// Threshold where the noise filter starts to reduce the amount of filtering.
			double distanceThreshold = cmd->GetDouble(1, tablet->noiseFilter.distanceThreshold);

			// Distance where the amount of filtering will be zero. Default maximum is 2 times the threshold.
			double distanceMaximum = cmd->GetDouble(2, distanceThreshold * 2.0);

			// Geometric median calculation iteration count
			int iterations = cmd->GetInt(3, tablet->noiseFilter.iterations);

			// Limits
			if(length < 0) length = 0;
			else if(length > 50) length = 50;

			if(distanceThreshold < 0.0) distanceThreshold = 0.0;
			else if(distanceThreshold > 1000) distanceThreshold = 1000;

			if(distanceMaximum < 0.1) distanceMaximum = 0.1;
			else if(distanceMaximum > 1000) distanceMaximum = 1000;

			if(distanceThreshold > distanceMaximum) distanceThreshold = distanceMaximum;

			if(iterations < 1) iterations = 1;
			else if(iterations > 100) iterations = 100;

			// Set noise filter values
			tablet->noiseFilter.buffer.SetLength(length);
			tablet->noiseFilter.distanceThreshold = distanceThreshold;
			tablet->noiseFilter.distanceMaximum = distanceMaximum;
			tablet->noiseFilter.iterations = iterations;

			// Enable filter
			if(tablet->noiseFilter.buffer.length > 0) {
				tablet->noiseFilter.isEnabled = true;
				LOG_INFO("Noise Reduction = [\n");
				LOG_INFO("  %d samples\n", length);
				LOG_INFO("  %0.2f mm threshold (Wacom %0.2f mm/s)\n", distanceThreshold, distanceThreshold * 133.0);
				LOG_INFO("  %0.2f mm maximum (Wacom %0.2f mm/s)\n", distanceMaximum, distanceMaximum * 133.0);
				LOG_INFO("  %d iterations\n", iterations);
				LOG_INFO("]\n");
			} else {
				tablet->noiseFilter.isEnabled = false;
				LOG_INFO("Noise Reduction = off\n");
			}

		}
		return true;
	}));
	


	//
	// Command: AntiSmoothingFilter, AntiSmoothing, Anti
	//
	// Sets anti-smoothing filter parameters
	//
	AddAlias("AntiSmoothing", "AntiSmoothingFilter");
	AddAlias("Anti", "AntiSmoothingFilter");
	AddCommand(new Command("AntiSmoothingFilter", [&](CommandLine *cmd) {

		string stringValue = cmd->GetStringLower(0, "");

		// Off / False
		if(stringValue == "off" || stringValue == "false") {
			tablet->antiSmoothing.isEnabled = false;
			LOG_INFO("Anti-smoothing = off\n");

		// Enable
		} else {

			double shape = cmd->GetDouble(0, 1.0);
			double compensation = cmd->GetDouble(1, 1.0);
			bool ignoreWhenDragging = cmd->GetBoolean(2, false);

			if(shape <= 0) {
				tablet->antiSmoothing.isEnabled = false;
				LOG_INFO("Anti-smoothing = off\n");
			} else {
				tablet->antiSmoothing.shape = shape;
				tablet->antiSmoothing.compensation = compensation;
				tablet->antiSmoothing.ignoreWhenDragging = ignoreWhenDragging;
				tablet->antiSmoothing.isEnabled = true;
				LOG_INFO("Anti-smoothing = Shape:%0.2f Compensation:%0.2f DragIgnore:%s\n",
					tablet->antiSmoothing.shape,
					tablet->antiSmoothing.compensation,
					tablet->antiSmoothing.ignoreWhenDragging ? "true" : "false"
				);
			}
		}
		return true;
	}));


	//
	// Command: FilterTester, Tester
	//
	AddAlias("Tester", "FilterTester");
	AddCommand(new Command("FilterTester", [&](CommandLine *cmd) {

		string filterName = cmd->GetStringLower(0, "");
		string inputFilepath = cmd->GetStringLower(1, "tester_input.txt");
		string outputFilepath = cmd->GetStringLower(2, "tester_output.txt");
		TabletFilter *filter = NULL;
		TabletFilterTester *tester = NULL;
		char settingsString[1024];

		if(filterName.compare(0, 4, "anti") == 0) {
			LOG_DEBUG("Anti!\n");
			filter = new TabletFilterAntiSmoothing();
			((TabletFilterAntiSmoothing*)filter)->shape = tablet->antiSmoothing.shape;
			((TabletFilterAntiSmoothing*)filter)->compensation = tablet->antiSmoothing.compensation;
			((TabletFilterAntiSmoothing*)filter)->ignoreWhenDragging = tablet->antiSmoothing.ignoreWhenDragging;
			sprintf_s(settingsString, "settings AntiSmoothing shape=%0.3f compensation=%0.3f dragignore=%s",
				tablet->antiSmoothing.shape,
				tablet->antiSmoothing.compensation,
				tablet->antiSmoothing.ignoreWhenDragging ? "true" : "false"
			);

		} else if(filterName.compare(0, 5, "noise") == 0) {
			filter = new TabletFilterNoiseReduction();
			((TabletFilterNoiseReduction*)filter)->buffer.SetLength(tablet->noiseFilter.buffer.length);
			((TabletFilterNoiseReduction*)filter)->distanceThreshold = tablet->noiseFilter.distanceThreshold;
			((TabletFilterNoiseReduction*)filter)->distanceMaximum = tablet->noiseFilter.distanceMaximum;
			((TabletFilterNoiseReduction*)filter)->iterations = tablet->noiseFilter.iterations;
			sprintf_s(settingsString, "settings NoiseReduction buffer=%d threshold=%0.3f maximum=%0.3f iterations=%d",
				tablet->noiseFilter.buffer.length,
				tablet->noiseFilter.distanceThreshold,
				tablet->noiseFilter.distanceMaximum,
				tablet->noiseFilter.iterations
			);


		}
		if(filter != NULL) {

			logger.directPrint = true;

			LOG_INFO("Filter test starting!\n");

			tester = new TabletFilterTester(filter, inputFilepath, outputFilepath);
			bool result = tester->Open();
			if(!result) {
				LOG_ERROR("Filter tester can't open files!\n");
			}
			tester->outputFile << settingsString << "\n";
			tester->Run();
			LOG_INFO("Filter test ended!\n");

			logger.ProcessMessages();
			logger.directPrint = false;

		} else {
			LOG_INFO("Usage: FilterTester <filter name>\n");
		}

		if(filter != NULL) {
			delete filter;
		}
		if(tester != NULL) {
			delete tester;
		}
		return true;
	}));



	//
	// Command: Benchmark, Bench
	//
	AddAlias("Bench", "Benchmark");
	AddCommand(new Command("Benchmark", [&](CommandLine *cmd) {

		if(!ExecuteCommand("TabletValid")) return false;

		int timeLimit;
		int reportCount = cmd->GetInt(0, 200);

		// Limit report count
		if(reportCount < 10) reportCount = 10;
		if(reportCount > 1000) reportCount = 1000;

		// Time limit
		timeLimit = reportCount * 10;
		if(timeLimit < 1000) timeLimit = 1000;

		// Log
		LOG_INFO("Tablet benchmark starting in 3 seconds!\n");
		LOG_INFO("Keep the pen stationary on top of the tablet!\n");
		Sleep(3000);
		LOG_INFO("Benchmark started!\n");

		// Start measurement
		tablet->measurement.Start(reportCount);

		// Wait for the benchmark to finish
		for(int i = 0; i < timeLimit / 100; i++) {
			Sleep(100);

			// Benchmark results
			if(tablet->measurement.reportCounter <= 0) {

				double width = tablet->measurement.maximum.x - tablet->measurement.minimum.x;
				double height = tablet->measurement.maximum.y - tablet->measurement.minimum.y;
				LOG_INFO("\n");
				LOG_INFO("Benchmark result (%d positions):\n", tablet->measurement.totalReports);
				LOG_INFO("  Tablet: %s\n", tablet->name.c_str());
				LOG_INFO("  Area: %0.2f mm x %0.2f mm (%0.0f px x %0.0f px)\n",
					mapper->areaTablet.width,
					mapper->areaTablet.height,
					mapper->areaScreen.width,
					mapper->areaScreen.height
				);
				LOG_INFO("  X range: %0.3f mm <-> %0.3f mm\n", tablet->measurement.minimum.x, tablet->measurement.maximum.x);
				LOG_INFO("  Y range: %0.3f mm <-> %0.3f mm\n", tablet->measurement.minimum.y, tablet->measurement.maximum.y);
				LOG_INFO("  Width: %0.3f mm (%0.2f px)\n",
					width,
					mapper->areaScreen.width / mapper->areaTablet.width * width
				);
				LOG_INFO("  Height: %0.3f mm (%0.2f px)\n",
					height,
					mapper->areaScreen.height / mapper->areaTablet.height* height
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
		if(tablet->measurement.reportCounter > 0) {
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

		if(!ExecuteCommand("TabletValid")) return false;

		int timeLimit = 30000;
		int pointCount = cmd->GetInt(0, 1);

		// Limits
		if(pointCount < 1) pointCount = 1;
		if(pointCount > 10) pointCount = 10;

		// Log
		LOG_INFO("Measurement started!\n");

		// Start measurement
		tablet->measurement.Start();

		// Wait for the measurement to finish
		for(int i = 0; i < timeLimit / 100; i++) {
			Sleep(100);

			// Result
			if(tablet->measurement.pointCount >= pointCount) {
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
				for(int pointIndex = 0; pointIndex < tablet->measurement.pointCount; pointIndex++) {

					point.Set(tablet->measurement.points[pointIndex]);

					// Limits
					if(point.x < minimum.x) minimum.x = point.x;
					if(point.x > maximum.x) maximum.x = point.x;
					if(point.y < minimum.y) minimum.y = point.y;
					if(point.y > maximum.y) maximum.y = point.y;

					LOG_INFO(
						"  #%d: X = %0.2f mm, Y = %0.2f mm\n",
						pointIndex + 1,
						point.x,
						point.y
					);

					// Distance calculation
					if(pointIndex > 0) {
						distance += point.Distance(lastPoint);
					}
					lastPoint.Set(point);

				}
				if(tablet->measurement.pointCount > 1) {
					LOG_INFO("  Distance: %0.2f mm\n", distance);
					LOG_INFO("  Maximum area: %0.2f mm x %0.2f mm\n",
						maximum.x - minimum.x, maximum.y - minimum.y
					);
				}

				// Status message
				char pointString[1024];
				char pointStringIndex = 0;

				// Build point string
				for(int pointIndex = 0; pointIndex < tablet->measurement.pointCount; pointIndex++) {
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
		if(tablet->measurement.pointCount < pointCount) {
			LOG_ERROR("Measurement failed! No enough points detected in %0.0f seconds!\n", timeLimit / 1000.0);
			LOG_STATUS("MEASUREMENT 0\n");
		}

		return true;
	}));


	//
	// Command: Debug
	//
	// Enable/disable debugging output
	//
	AddCommand(new Command("Debug", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;
		logger.debugEnabled = cmd->GetBoolean(0, logger.debugEnabled);
		LOG_INFO("Debug logging = %s\n", logger.debugEnabled ? "True" : "False");
		return true;
	}));


	//
	// Command: Log
	//
	// Start/stop logging messages to a log file
	//
	AddCommand(new Command("Log", [&](CommandLine *cmd) {
		string logPath = cmd->GetString(0, "log.txt");
		if(!cmd->GetBoolean(0, true)) {
			logger.CloseLogFile();
			LOG_INFO("Log file '%s' closed.\n", logger.logFilename.c_str());
		} else if(logger.OpenLogFile(logPath)) {
			LOG_INFO("Log file '%s' opened.\n", logPath.c_str());
		} else {
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
	// Command: LogDirect
	//
	// Enables/disables direct logging without queuing the messages.
	//
	AddCommand(new Command("LogDirect", [&](CommandLine *cmd) {
		logger.ProcessMessages();
		logger.directPrint = cmd->GetBoolean(0, logger.directPrint);
		logger.ProcessMessages();

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

		if(!ExecuteCommand("TabletValid")) return false;

		char stringBuffer[64];
		int maxLength = sizeof(stringBuffer) - 1;
		int stringIndex = 0;

		LOG_INFO("\n");
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

		for(int i = 0; i < 8; i++) {
			stringIndex += sprintf_s(stringBuffer + stringIndex, maxLength - stringIndex, "%d ", tablet->buttonMap[i]);
		}
		LOG_INFO("  Button Map = %s\n", stringBuffer);


		if(tablet->initFeatureLength > 0) {
			LOG_INFOBUFFER(tablet->initFeature, tablet->initFeatureLength, "  Tablet init feature report: ");
		}
		if(tablet->initReportLength > 0) {
			LOG_INFOBUFFER(tablet->initReport, tablet->initReportLength, "  Tablet init report: ");
		}
		LOG_INFO("\n");
		LOG_INFO("Area:\n");
		LOG_INFO("  Desktop = %0.0fpx x %0.0fpx\n",
			mapper->areaVirtualScreen.width, mapper->areaVirtualScreen.height
		);
		LOG_INFO("  Screen Map = %0.0fpx x %0.0fpx @ X%+0.0fpx, Y%+0.0fpx\n",
			mapper->areaScreen.width, mapper->areaScreen.height,
			mapper->areaScreen.x, mapper->areaScreen.y
		);
		LOG_INFO("  Tablet =  %0.2fmm x %0.2fmm @ Min(%0.2fmm, %0.2fmm) Max(%0.2fmm, %0.2fmm)\n",
			mapper->areaTablet.width, mapper->areaTablet.height,
			mapper->areaTablet.x, mapper->areaTablet.y,
			mapper->areaTablet.width + mapper->areaTablet.x, mapper->areaTablet.height + mapper->areaTablet.y
		);
		LOG_INFO("  Rotation matrix = [%f,%f,%f,%f]\n",
			mapper->rotationMatrix[0],
			mapper->rotationMatrix[1],
			mapper->rotationMatrix[2],
			mapper->rotationMatrix[3]
		);
		LOG_INFO("\n");

		return true;
	}));



	//
	// Command: Status
	//
	// Outputs driver infor as status messages
	//
	AddCommand(new Command("Status", [&](CommandLine *cmd) {
		if(!ExecuteCommand("TabletValid")) return false;

		LOG_STATUS("TABLET %s\n", tablet->name.c_str());

		if(tablet->hidDevice != NULL) {
			LOG_STATUS("HID %04X %04X %04X %04X\n",
				tablet->hidDevice->vendorId,
				tablet->hidDevice->productId,
				tablet->hidDevice->usagePage,
				tablet->hidDevice->usage
			);
		} else if(tablet->usbDevice != NULL) {
			LOG_STATUS("USB %d %s\n",
				tablet->usbDevice->stringId,
				tablet->usbDevice->stringMatch.c_str()
			);
		}
		LOG_STATUS("WIDTH %0.5f\n", tablet->settings.width);
		LOG_STATUS("HEIGHT %0.5f\n", tablet->settings.height);
		LOG_STATUS("MAX_X %d\n", tablet->settings.maxX);
		LOG_STATUS("MAX_Y %d\n", tablet->settings.maxY);
		LOG_STATUS("MAX_PRESSURE %d\n", tablet->settings.maxPressure);

		return true;
	}));


	//
	// Command: Include
	//
	// Reads commands from a file and stops if tablet is redefined.
	//
	AddCommand(new Command("Include", [&](CommandLine *cmd) {
		string filename = cmd->GetString(0, "");
		if(filename == "") {
			LOG_ERROR("Invalid filename '%s'!\n", filename.c_str());
			return false;
		} else {
			if(ExecuteFile(filename)) {
			} else {
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
		for(pair<string, Command*> cmdPair : commands) {
			string commandName = cmdPair.second->name;
			LOG_INFO("  %s\n", commandName.c_str());
		}
		
		return true;
	}));




}

//
// Is valid command?
//
bool CommandHandler::IsValidCommand(string command) {
	transform(command.begin(), command.end(), command.begin(), ::tolower);
	if(aliases.count(command) > 0) {
		command = aliases[command];
	}
	if(commands.count(command) > 0) {
		return true;
	}
	return false;
}

//
// Execute a command using command name
//
bool CommandHandler::ExecuteCommand(string command) {
	return ExecuteCommand(command, NULL);
}

//
// Execute a command using command line
//
bool CommandHandler::ExecuteCommand(CommandLine *cmd) {
	return ExecuteCommand(cmd->command, cmd);
}

//
// Execute a command
//
bool CommandHandler::ExecuteCommand(string command, CommandLine * cmd) {
	transform(command.begin(), command.end(), command.begin(), ::tolower);
	if(aliases.count(command) > 0) {
		command = aliases[command];
	}
	if(commands.count(command) > 0) {
		return commands[command]->Execute(cmd);
	}
	return false;
}

//
// Execute commands from a file
//
bool CommandHandler::ExecuteFile(string filename) {

	CommandLine *cmd;
	ifstream file;
	string line = "";

	// Open file
	file.open(filename);
	if(!file.is_open()) {
		return false;
	}


	LOG_INFO("\\ Reading '%s'\n", filename.c_str());

	// Loop through lines
	while(!file.eof()) {
		getline(file, line);
		if(line.length() == 0) continue;
		cmd = new CommandLine(line);

		//
		// Do not redefine tablet if one is already open
		//
		if(cmd->is("Tablet") && tablet != NULL && tablet->IsConfigured()) {
			LOG_INFO(">> %s\n", cmd->line.c_str());
			LOG_INFO("Tablet is already defined!\n");
			delete cmd;
			break;
		}
		LOG_INFO(">> %s\n", cmd->line.c_str());

		ExecuteCommand(cmd);

		delete cmd;
	}
	file.close();

	LOG_INFO("/ End of '%s'\n", filename.c_str());

	return true;
}
