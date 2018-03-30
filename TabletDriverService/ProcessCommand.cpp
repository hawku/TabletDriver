#include "stdafx.h"
#include "ProcessCommand.h"

#define LOG_MODULE ""
#include "Logger.h"


//
// Process Command
//
bool ProcessCommand(CommandLine *cmd) {

	LOG_INFO(">> %s\n", cmd->line.c_str());

	//
	// Tablet
	//
	if(cmd->is("Tablet")) {

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
	}

	// Check tablet
	else if(cmd->is("CheckTablet")) {
		if(!CheckTablet()) {
			LOG_ERROR("Tablet not found!\n");
			LOG_ERROR("Check the list of supported tablets from the GitHub page.\n");
			LOG_ERROR("http://github.com/hawku/TabletDriver\n");
			CleanupAndExit(1);
		}
	}

	// HID List
	else if(cmd->is("HIDList")) {
		HANDLE hidHandle = 0;
		HIDDevice *hid = new HIDDevice();
		hid->debugEnabled = true;
		hid->OpenDevice(&hidHandle, 1, 1, 1, 1);
		if(hidHandle > 0) {
			CloseHandle(hidHandle);
		}
		delete hid;
	}


	// HID Device 2
	else if(cmd->is("HID2")) {

		if(cmd->valueCount == 4) {
			USHORT vendorID = cmd->GetInt(0, 0);
			USHORT productID = cmd->GetInt(1, 0);
			USHORT usagePage = cmd->GetInt(2, 0);
			USHORT usage = cmd->GetInt(3, 0);
			if(tablet->hidDevice2 == NULL) {
				tablet->hidDevice2 = new HIDDevice(vendorID, productID, usagePage, usage);
				if(tablet->hidDevice2->isOpen) {
					LOG_INFO("HID Device found!\n");
				} else {
					LOG_ERROR("Can't open HID device 0x%04X 0x%04X 0x%04X 0x%04X\n", vendorID, productID, usagePage, usage);
					delete tablet->hidDevice2;
					tablet->hidDevice2 = NULL;
				}
			}
		}
	}

	// Tablet Name
	else if(cmd->is("Name")) {
		if(tablet == NULL) return false;
		tablet->name = cmd->GetString(0, tablet->name);
		LOG_INFO("Tablet name = '%s'\n", tablet->name.c_str());
	}

	// Report Id
	else if(cmd->is("ReportId")) {
		if(tablet == NULL) return false;
		tablet->settings.reportId = cmd->GetInt(0, tablet->settings.reportId);
		LOG_INFO("Tablet report id = %d\n", tablet->settings.reportId);
	}

	// Report Length
	else if(cmd->is("ReportLength")) {
		if(tablet == NULL) return false;
		tablet->settings.reportLength = cmd->GetInt(0, tablet->settings.reportLength);
		LOG_INFO("Tablet report length = %d\n", tablet->settings.reportLength);
	}

	// Detect Mask
	else if(cmd->is("DetectMask")) {
		if(tablet == NULL) return false;
		tablet->settings.detectMask = cmd->GetInt(0, tablet->settings.detectMask);
		LOG_INFO("Tablet detect mask = %02X\n", tablet->settings.detectMask);
	}

	// Ignore Mask
	else if(cmd->is("IgnoreMask")) {
		if(tablet == NULL) return false;
		tablet->settings.ignoreMask = cmd->GetInt(0, tablet->settings.ignoreMask);
		LOG_INFO("Tablet ignore mask = %02X\n", tablet->settings.ignoreMask);
	}

	// Max X
	else if(cmd->is("MaxX")) {
		if(tablet == NULL) return false;
		tablet->settings.maxX = cmd->GetInt(0, tablet->settings.maxX);
		LOG_INFO("Tablet max X = %d\n", tablet->settings.maxX);
	}

	// Max Y
	else if(cmd->is("MaxY")) {
		if(tablet == NULL) return false;
		tablet->settings.maxY = cmd->GetInt(0, tablet->settings.maxY);
		LOG_INFO("Tablet max Y = %d\n", tablet->settings.maxY);
	}

	// Max Pressure
	else if(cmd->is("MaxPressure")) {
		if(tablet == NULL) return false;
		tablet->settings.maxPressure = cmd->GetInt(0, tablet->settings.maxPressure);
		LOG_INFO("Tablet max pressure = %d\n", tablet->settings.maxPressure);
	}

	// Click Pressure
	else if(cmd->is("ClickPressure")) {
		if(tablet == NULL) return false;
		tablet->settings.clickPressure = cmd->GetInt(0, tablet->settings.clickPressure);
		LOG_INFO("Tablet click pressure = %d\n", tablet->settings.clickPressure);
	}


	// Keep pen tip down
	else if(cmd->is("KeepTipDown")) {
		if(tablet == NULL) return false;
		tablet->settings.keepTipDown = cmd->GetInt(0, tablet->settings.keepTipDown);
		LOG_INFO("Tablet pen tip keep down = %d packets\n", tablet->settings.keepTipDown);
	}


	// Width
	else if(cmd->is("Width")) {
		if(tablet == NULL) return false;
		tablet->settings.width = cmd->GetDouble(0, tablet->settings.width);
		LOG_INFO("Tablet width = %0.2f mm\n", tablet->settings.width);
	}

	// Height
	else if(cmd->is("Height")) {
		if(tablet == NULL) return false;
		tablet->settings.height = cmd->GetDouble(0, tablet->settings.height);
		LOG_INFO("Tablet height = %0.2f mm\n", tablet->settings.height);
	}

	// Skew
	else if(cmd->is("Skew")) {
		if(tablet == NULL) return false;
		tablet->settings.skew = cmd->GetDouble(0, tablet->settings.skew);
		LOG_INFO("Tablet skew = Shift X-axis %0.2f mm per Y-axis mm\n", tablet->settings.skew);
	}

	// Skew
	else if(cmd->is("Type")) {
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
	}


	// Init Feature Report
	else if(cmd->is("InitFeature") && cmd->valueCount > 0) {
		if(tablet == NULL) return false;
		tablet->initFeature = new BYTE[cmd->valueCount];
		for(int i = 0; i < (int)cmd->valueCount; i++) {
			tablet->initFeature[i] = cmd->GetInt(i, 0);
		}
		tablet->initFeatureLength = cmd->valueCount;
		LOG_INFOBUFFER(tablet->initFeature, tablet->initFeatureLength, "Tablet init feature report: ");
	}

	// Init Output Report
	else if(cmd->is("InitReport") && cmd->valueCount > 0) {
		if(tablet == NULL) return false;
		tablet->initReport = new BYTE[cmd->valueCount];
		for(int i = 0; i < (int)cmd->valueCount; i++) {
			tablet->initReport[i] = cmd->GetInt(i, 0);
		}
		tablet->initReportLength = cmd->valueCount;
		LOG_INFOBUFFER(tablet->initReport, tablet->initReportLength, "Tablet init report: ");

	}


	//
	// Set Feature Report
	//
	else if((cmd->is("SetFeature") || cmd->is("Feature")) && cmd->valueCount > 1) {
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
	}

	//
	// Get Feature Report
	//
	else if(cmd->is("GetFeature") && cmd->valueCount > 1) {
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
	}


	//
	// Send Output Report
	//
	else if((cmd->is("OutputReport") || cmd->is("Report")) && cmd->valueCount > 1) {
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
	}

	//
	// Tablet Area
	//
	else if(cmd->is("TabletArea") || cmd->is("Area")) {
		if(!CheckTablet()) return true;
		mapper->areaTablet.width = cmd->GetDouble(0, mapper->areaTablet.width);
		mapper->areaTablet.height = cmd->GetDouble(1, mapper->areaTablet.height);
		mapper->areaTablet.x = cmd->GetDouble(2, mapper->areaTablet.x);
		mapper->areaTablet.y = cmd->GetDouble(3, mapper->areaTablet.y);

		LogTabletArea("Tablet area");
	}

	// Button Map
	else if(cmd->is("ButtonMap") || cmd->is("Buttons")) {
		if(!CheckTablet()) return true;
		char buttonMapBuffer[32];
		int index = 0;
		for(int i = 0; i < 8; i++) {
			tablet->buttonMap[i] = cmd->GetInt(i, tablet->buttonMap[i]);
			index += sprintf_s(buttonMapBuffer + index, 32 - index, "%d ", tablet->buttonMap[i]);
		}
		LOG_INFO("Button Map = %s\n", buttonMapBuffer);
	}


	// Screen Map Area
	else if(cmd->is("ScreenArea") || cmd->is("Screen")) {
		if(!CheckTablet()) return true;
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
	}

	// Screen Map Area
	else if(cmd->is("DesktopSize") || cmd->is("Desktop")) {
		if(!CheckTablet()) return true;
		mapper->areaVirtualScreen.width = cmd->GetDouble(0, mapper->areaVirtualScreen.width);
		mapper->areaVirtualScreen.height = cmd->GetDouble(1, mapper->areaVirtualScreen.height);
		LOG_INFO("Desktop size = (%0.2f px x %0.2f px)\n",
			mapper->areaVirtualScreen.width,
			mapper->areaVirtualScreen.height
		);
	}

	// Move tablet area to border
	else if(cmd->is("TabletMove") || cmd->is("Move") && cmd->valueCount > 0) {
		if(!CheckTablet()) return true;
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
				LogTabletArea("  New tablet area");
			}
		}
	}

	//
	// Rotate tablet area
	//
	else if(cmd->is("Rotate")) {
		if(!CheckTablet()) return true;
		double value = cmd->GetDouble(0, 0);
		mapper->SetRotation(value);
		LOG_INFO("Rotation matrix = [%f,%f,%f,%f]\n",
			mapper->rotationMatrix[0],
			mapper->rotationMatrix[1],
			mapper->rotationMatrix[2],
			mapper->rotationMatrix[3]
		);
	}


	//
	// Relative mode sensitivity
	//
	else if(cmd->is("Sensitivity")) {
		if(!CheckTablet()) return true;
		vmulti->relativeData.sensitivity = cmd->GetDouble(0, vmulti->relativeData.sensitivity);
		LOG_INFO("Relative mode sensitivity = %0.2f px/mm\n", vmulti->relativeData.sensitivity);
	}


	//
	// VMulti output mode
	//
	else if(cmd->is("Mode")) {
		string mode = cmd->GetStringLower(0, "");

		// Absolute mouse
		if(mode.compare(0, 3, "abs") == 0) {
			if(vmulti->mode != VMulti::ModeAbsoluteMouse)
				vmulti->ResetReport();
			vmulti->mode = VMulti::ModeAbsoluteMouse;
			LOG_INFO("Output Mode = Absolute\n");
		}

		// Relative mouse
		else if(mode.compare(0, 3, "rel") == 0) {
			if(vmulti->mode != VMulti::ModeRelativeMouse)
				vmulti->ResetReport();
			vmulti->mode = VMulti::ModeRelativeMouse;
			LOG_INFO("Output Mode = Relative\n");
		}

		// Digitizer
		else if(mode.compare(0, 3, "dig") == 0 || mode.compare(0, 3, "pen") == 0) {
			if(vmulti->mode != VMulti::ModeDigitizer)
				vmulti->ResetReport();
			vmulti->mode = VMulti::ModeDigitizer;
			LOG_INFO("Output Mode = Digitizer\n");

		} else {
			LOG_ERROR("Unknown output mode '%s'\n", mode.c_str());
		}

	}


	//
	// Smoothing filter
	//
	else if(cmd->is("Smoothing")) {
		if(!CheckTablet()) return true;
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
	}

	//
	// Smoothing filter interval
	//
	else if(cmd->is("SmoothingInterval")) {
		int interval = cmd->GetInt(0, (int)round(tablet->smoothing.timerInterval));

		// 10 Hz
		if(interval > 100) interval = 100;

		// 1000 Hz
		if(interval < 1) interval = 1;

		// Interval changed?
		if(interval != (int)round(tablet->smoothing.timerInterval)) {
			tablet->smoothing.timerInterval = interval;
			tablet->smoothing.SetLatency(tablet->smoothing.latency);
			if(tablet->smoothing.StopTimer()) {
				tablet->smoothing.StartTimer();
			}
		}

		LOG_INFO("Smoothing Interval = %d (%0.2f Hz, %0.2f ms, %f)\n", interval, 1000.0 / interval, tablet->smoothing.latency, tablet->smoothing.weight);

	}


	//
	// Noise reduction filter
	//
	else if(cmd->is("Noise")) {

		string stringValue = cmd->GetStringLower(0, "");

		// Off / False
		if(stringValue == "off" || stringValue == "false") {
			tablet->noise.isEnabled = false;
			LOG_INFO("Noise Reduction = off\n");

		// Set parameters
		} else {

			int length = cmd->GetInt(0, tablet->noise.bufferLength);
			double distanceThreshold = cmd->GetDouble(1, tablet->noise.distanceThreshold);
			int iterations = cmd->GetInt(2, tablet->noise.iterations);

			// Limits
			if(length < 0) length = 0;
			else if(length > 50) length = 50;

			if(distanceThreshold < 0) distanceThreshold = 0;
			else if(distanceThreshold > 100) distanceThreshold = 100;

			if(iterations < 1) iterations = 1;
			else if(iterations > 100) iterations = 100;

			// Set
			tablet->noise.SetBufferLength(length);
			tablet->noise.distanceThreshold = distanceThreshold;
			tablet->noise.iterations = iterations;

			// Enable filter
			if(tablet->noise.bufferLength > 0) {
				tablet->noise.isEnabled = true;
				LOG_INFO("Noise Reduction = %d packets, %0.3f mm threshold, %d iterations\n", length, distanceThreshold, iterations);
			} else {
				tablet->noise.isEnabled = false;
				LOG_INFO("Noise Reduction = off\n");
			}

		}




	}

	// Debug
	else if(cmd->is("Debug")) {
		if(!CheckTablet()) return true;
		tablet->debugEnabled = cmd->GetBoolean(0, tablet->debugEnabled);
		//vmulti->debugEnabled = tablet->debugEnabled;
		LOG_INFO("Tablet debug = %s\n", tablet->debugEnabled ? "True" : "False");
	}


	// Log
	else if(cmd->is("Log") && cmd->valueCount > 0) {
		string logPath = cmd->GetString(0, "log.txt");
		if(!cmd->GetBoolean(0, true)) {
			logger.CloseLogFile();
			LOG_INFO("Log file '%s' closed.\n", logger.logFilename.c_str());
		} else if(logger.OpenLogFile(logPath)) {
			LOG_INFO("Log file '%s' opened.\n", logPath.c_str());
		} else {
			LOG_ERROR("Cant open log file!\n");
		}
	}


	// Wait
	else if(cmd->is("Wait")) {
		int waitTime = cmd->GetInt(0, 0);
		Sleep(waitTime);
	}

	// Direct logging
	else if(cmd->is("LogDirect")) {
		logger.ProcessMessages();
		logger.directPrint = cmd->GetBoolean(0, logger.directPrint);
		logger.ProcessMessages();

		LOG_INFO("Log direct print = %s\n", logger.directPrint ? "True" : "False");
	}


	// Output
	else if(cmd->is("Output")) {
		vmulti->outputEnabled = cmd->GetBoolean(0, vmulti->outputEnabled);
		LOG_INFO("Output enabled = %s\n", vmulti->outputEnabled ? "True" : "False");
	}

	// Info
	else if(cmd->is("Info")) {
		if(!CheckTablet()) return true;
		LogInformation();
	}

	// Status
	else if(cmd->is("Status")) {
		if(!CheckTablet()) return true;
		LogStatus();
	}

	// Benchmark
	else if(cmd->is("Benchmark") || cmd->is("Bench")) {
		if(!CheckTablet()) return true;

		int timeLimit;
		int packetCount = cmd->GetInt(0, 200);

		// Limit packet count
		if(packetCount < 10) packetCount = 10;
		if(packetCount > 1000) packetCount = 1000;

		// Time limit
		timeLimit = packetCount * 10;
		if(timeLimit < 1000) timeLimit = 1000;

		// Log
		LOG_DEBUG("Tablet benchmark starting in 3 seconds!\n");
		LOG_DEBUG("Keep the pen stationary on top of the tablet!\n");
		Sleep(3000);
		LOG_DEBUG("Benchmark started!\n");

		// Benchmark
		tablet->benchmark.Start(packetCount);

		// Wait for the benchmark to finish
		for(int i = 0; i < timeLimit / 100; i++) {
			Sleep(100);

			// Benchmark result
			if(tablet->benchmark.packetCounter <= 0) {

				double width = tablet->benchmark.maxX - tablet->benchmark.minX;
				double height = tablet->benchmark.maxY - tablet->benchmark.minY;
				LOG_DEBUG("\n");
				LOG_DEBUG("Benchmark result (%d positions):\n", tablet->benchmark.totalPackets);
				LOG_DEBUG("  Tablet: %s\n", tablet->name.c_str());
				LOG_DEBUG("  Area: %0.2f mm x %0.2f mm (%0.0f px x %0.0f px)\n",
					mapper->areaTablet.width,
					mapper->areaTablet.height,
					mapper->areaScreen.width,
					mapper->areaScreen.height
				);
				LOG_DEBUG("  X range: %0.3f mm <-> %0.3f mm\n", tablet->benchmark.minX, tablet->benchmark.maxX);
				LOG_DEBUG("  Y range: %0.3f mm <-> %0.3f mm\n", tablet->benchmark.minY, tablet->benchmark.maxY);
				LOG_DEBUG("  Width: %0.3f mm (%0.2f px)\n",
					width,
					mapper->areaScreen.width / mapper->areaTablet.width * width
				);
				LOG_DEBUG("  Height: %0.3f mm (%0.2f px)\n",
					height,
					mapper->areaScreen.height / mapper->areaTablet.height* height
				);
				LOG_DEBUG("\n");
				LOG_STATUS("BENCHMARK %d %0.3f %0.3f %s\n", tablet->benchmark.totalPackets, width, height, tablet->name.c_str());
				break;
			}
		}

		// Benchmark failed
		if(tablet->benchmark.packetCounter > 0) {
			LOG_ERROR("Benchmark failed!\n");
			LOG_ERROR("Not enough packets captured in %0.2f seconds!\n",
				timeLimit / 1000.0
			);
		}


	}


	// Include
	else if(cmd->is("Include")) {
		string filename = cmd->GetString(0, "");
		if(filename == "") {
			LOG_ERROR("Invalid filename '%s'!\n", filename.c_str());
		} else {
			if(ReadCommandFile(filename)) {
			} else {
				LOG_ERROR("Can't open file '%s'\n", filename.c_str());
			}
		}
	}


	// Exit
	else if(cmd->is("Exit") || cmd->is("Quit")) {
		LOG_INFO("Bye!\n");
		CleanupAndExit(0);
	}


	// Unknown
	else if(cmd->isValid) {
		LOG_WARNING("Unknown command: %s\n", cmd->line.c_str());
	}

	return true;
}



//
// Read command file
//
bool ReadCommandFile(string filename) {
	CommandLine *cmd;
	ifstream file;
	string line = "";

	// Open config file
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
		ProcessCommand(cmd);
		delete cmd;
	}
	file.close();

	LOG_INFO("/ End of '%s'\n", filename.c_str());

	return true;
}



//
// Log general information
//
void LogInformation() {
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
	LOG_INFO("  Keep Tip Down = %d packets\n", tablet->settings.keepTipDown);
	LOG_INFO("  Report Id = %02X\n", tablet->settings.reportId);
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
}

//
// Log Status
//
void LogStatus() {
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
}

//
// Log tablet area
//
void LogTabletArea(string text) {
	LOG_INFO("%s: (%0.2f mm x %0.2f mm X+%0.2f mm Y+%0.2f mm)\n",
		text.c_str(),
		mapper->areaTablet.width,
		mapper->areaTablet.height,
		mapper->areaTablet.x,
		mapper->areaTablet.y
	);
}


bool CheckTablet() {
	if(tablet == NULL) {
		return false;
	}
	return true;
}
