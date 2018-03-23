#include "stdafx.h"

#include <csignal>

#include "HIDDevice.h"
#include "USBDevice.h"
#include "VMulti.h"
#include "Tablet.h"
#include "ScreenMapper.h"
#include "CommandLine.h"
#include "ProcessCommand.h"

#define LOG_MODULE ""
#include "Logger.h"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "winusb.lib")


// Global variables...
Tablet *tablet;
VMulti *vmulti;
ScreenMapper *mapper;
thread *tabletThread;
thread *tabletAuxThread;

//
// Init console parameters
//
void InitConsole() {
	HANDLE inputHandle;
	DWORD consoleMode = 0;
	inputHandle = GetStdHandle(STD_INPUT_HANDLE);
	GetConsoleMode(inputHandle, &consoleMode);
	consoleMode = (consoleMode & ~ENABLE_QUICK_EDIT_MODE);
	consoleMode = (consoleMode & ~ENABLE_MOUSE_INPUT);
	consoleMode = (consoleMode & ~ENABLE_WINDOW_INPUT);
	SetConsoleMode(inputHandle, consoleMode);
}

//
// Tablet thread
//
void RunTabletThread() {
	int status;
	bool isFirstReport = true;
	bool isResent = false;
	double x, y;

	chrono::high_resolution_clock::time_point timeBegin = chrono::high_resolution_clock::now();
	chrono::high_resolution_clock::time_point timeNow = chrono::high_resolution_clock::now();

	//
	// Main Loop
	//

	while(true) {

		//
		// Read tablet position
		//
		status = tablet->ReadPosition();
		// Position OK
		if(status == Tablet::PacketValid) {
			isResent = false;
			// Position invalid

		// Invalid packet id
		} else if(status == Tablet::PacketInvalid) {
			continue;

		// Valid packet but position is not in-range or invalid
		} else if(status == Tablet::PacketPositionInvalid) {
			if(!isResent && tablet->state.isValid) {
				isResent = true;
				tablet->state.isValid = false;
			} else {
				continue;
			}
			// Reading failed
		} else {
			LOG_ERROR("Tablet Read Error!\n");
			CleanupAndExit(1);
		}

		//
		// Don't send the first report
		//
		if(isFirstReport) {
			isFirstReport = false;
			continue;
		}

		// Debug messages
		if(tablet->debugEnabled) {
			timeNow = chrono::high_resolution_clock::now();
			double delta = (timeNow - timeBegin).count() / 1000000.0;
			LOG_DEBUG("STATE: %0.3f, %d, %0.3f, %0.3f, %0.3f\n",
				delta,
				tablet->state.buttons,
				tablet->state.position.x,
				tablet->state.position.y,
				tablet->state.pressure
			);
		}


		// Set output values
		if(status == Tablet::PacketPositionInvalid) {
			tablet->state.buttons = 0;
		}

		//
		// Packet filter
		//
		if(tablet->filterPacket != NULL && tablet->filterPacket->isEnabled) {
			tablet->filterPacket->SetTarget(tablet->state.position);
			tablet->filterPacket->Update();
			tablet->filterPacket->GetPosition(&tablet->state.position);
		}


		// Do not write report when timed filter is enabled
		if(tablet->filterTimed == NULL || !tablet->filterTimed->isEnabled) {

			// Relative mode
			if(vmulti->mode == VMulti::ModeRelativeMouse) {

				x = tablet->state.position.x;
				y = tablet->state.position.y;

				// Map position to virtual screen (values between 0 and 1)
				mapper->GetRotatedTabletPosition(&x, &y);

				// Create VMulti report
				vmulti->CreateReport(tablet->state.buttons, x, y, tablet->state.pressure);

				// Write report to VMulti device
				vmulti->WriteReport();



			// Absolute / Digitizer mode
			} else {
				// Get x & y from the tablet state
				x = tablet->state.position.x;
				y = tablet->state.position.y;

				// Map position to virtual screen (values betweeb 0->1)
				mapper->GetScreenPosition(&x, &y);

				// Create VMulti report
				vmulti->CreateReport(tablet->state.buttons, x, y, tablet->state.pressure);

				// Write report to VMulti device
				vmulti->WriteReport();
			}
		}
	}

}

//
// Tablet auxiliary thread
//
void RunTabletAuxThread() {

	BYTE buffer[256];
	int status;

	while(true) {

		//
		// Wacom 480 aux
		//
		if(tablet->settings.auxType == TabletSettings::AuxWacom480) {

			// Read 64 bytes from the HID device
			status = tablet->hidDeviceAux->Read(buffer, 64);
			if(status < 0) {
				LOG_ERROR("HID auxiliary device error!\n");
				break;
			}

			//LOG_DEBUGBUFFER(buffer, 16, "AUX: ");

			// Report Id == 2?
			if(buffer[0] == 0x02) {

				// Loop through packet items
				for(int offset = 2; offset < 64; offset += 8) {

					// Buttons
					if(buffer[offset] == 0x80) {
						BYTE buttons = buffer[3];
						LOG_DEBUG("Aux buttons: %02X\n", buttons);
					}

					// Touch (max 16 points)
					if(buffer[offset] >= 2 && buffer[offset] <= 17) {
						BYTE touchIndex = buffer[offset] - 1;

						int touchX = (buffer[offset + 2] << 4) | (buffer[offset + 4] >> 4);
						int touchY = (buffer[offset + 3] << 4) | (buffer[offset + 4] & 0x0f);
						LOG_DEBUG("Aux Touch(%02X): % 5d, % 5d\n", touchIndex, touchX, touchY);
					}
				}
			}
		}
	}
}

//
// Tablet filter timer callback
//
VOID CALLBACK FilterTimerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
	Vector2D position;

	TabletFilter *filter = tablet->filterTimed;

	// Filter enabled?
	if(!filter->isEnabled) return;

	// Set filter targets
	filter->SetTarget(tablet->state.position);

	// Update filter position
	filter->Update();

	// Set output vector
	filter->GetPosition(&position);


	// Relative mode
	if(vmulti->mode == VMulti::ModeRelativeMouse) {

		// Map position to virtual screen (values between 0 and 1)
		mapper->GetRotatedTabletPosition(&position.x, &position.y);

		// Large distance -> Reset relative position
		double distance = tablet->state.position.Distance(vmulti->relativeData.lastPosition);
		if(distance > 10) {
			vmulti->ResetRelativeData(position.x, position.y);
		}

		// Create VMulti report
		vmulti->CreateReport(
			tablet->state.buttons,
			position.x,
			position.y,
			tablet->state.pressure
		);

		// Write report to VMulti device if report has changed
		if(vmulti->HasReportChanged()
			||
			vmulti->reportRelativeMouse.x != 0
			||
			vmulti->reportRelativeMouse.y != 0
		) {
			vmulti->WriteReport();
		}

	// Absolute / Digitizer mode
	} else {


		// Map position to virtual screen (values betweeb 0->1)
		mapper->GetScreenPosition(&position.x, &position.y);

		// Create VMulti report
		vmulti->CreateReport(
			tablet->state.buttons,
			position.x,
			position.y,
			tablet->state.pressure
		);


		// Write report to VMulti device
		if(vmulti->HasReportChanged() && tablet->state.isValid) {
			vmulti->WriteReport();
		}
	}


}



//
// Main
//
int main(int argc, char**argv) {
	string line;
	string filename;
	CommandLine *cmd;
	bool running = false;

	// Init global variables
	vmulti = NULL;
	tablet = NULL;
	tabletThread = NULL;
	tabletAuxThread = NULL;

	// Init console
	InitConsole();

	// Screen mapper
	mapper = new ScreenMapper(tablet);
	mapper->SetRotation(0);

	// Logger
	//LOGGER_DIRECT = true;
	LOGGER_START();

	// VMulti Device
	vmulti = new VMulti();
	if(!vmulti->isOpen) {
		LOG_ERROR("Can't open VMulti device!\n\n");
		LOG_ERROR("Possible fixes:\n");
		LOG_ERROR("1) Install VMulti driver\n");
		LOG_ERROR("2) Kill PentabletService.exe (XP Pen driver)\n");
		LOG_ERROR("3) Uninstall other tablet drivers and reinstall VMulti driver\n");
		CleanupAndExit(1);
	}

	// Read init file
	filename = "init.cfg";

	if(argc > 1) {
		filename = argv[1];
	}
	if(!ReadCommandFile(filename)) {
		LOG_ERROR("Can't open '%s'\n", filename.c_str());
	}


	//
	// Main loop that reads input from the console.
	//
	while(true) {

		// Broken pipe
		if(!cin) break;

		// Read line from the console
		try {
			getline(cin, line);
		} catch(exception) {
			break;
		}

		// Process valid lines
		if(line.length() > 0) {
			cmd = new CommandLine(line);


			//
			// Start command
			//
			if(cmd->is("start")) {
				LOG_INFO(">> %s\n", cmd->line.c_str());

				if(running) {
					LOG_INFO("Driver is already started!\n");
					continue;
				}

				// Unknown tablet
				if(tablet == NULL) {
					LOG_ERROR("Tablet not found!\n");
					CleanupAndExit(1);
				}

				// Tablet init
				if(!tablet->Init()) {
					LOG_ERROR("Tablet init failed!\n");
					LOG_ERROR("Possible fixes:\n");
					LOG_ERROR("1) Uninstall other tablet drivers.\n");
					LOG_ERROR("2) Stop other tablet driver services.\n");
					CleanupAndExit(1);
				}

				// Set screen mapper tablet
				mapper->tablet = tablet;

				// Set running state
				running = true;

				// Timed filter timer
				tablet->filterTimed->callback = FilterTimerCallback;
				tablet->filterTimed->StartTimer();

				// Start the tablet thread
				tabletThread = new thread(RunTabletThread);

				// Start the tablet auxiliary thread
				if(tablet->hidDeviceAux != NULL) {
					tabletAuxThread = new thread(RunTabletAuxThread);
				}

				LOG_INFO("TabletDriver started!\n");
				LogStatus();


			//
			// Echo
			//
			} else if(cmd->is("echo")) {
				if(cmd->valueCount > 0) {
					LOG_INFO("%s\n", cmd->line.c_str() + 5);
				} else {
					LOG_INFO("\n");
				}


			//
			// Process all other commands
			//
			} else {
				ProcessCommand(cmd);
			}
			delete cmd;
		}

	}

	CleanupAndExit(0);
	return 0;
}



//
// Cleanup and exit
//
void CleanupAndExit(int code) {
	/*
	if(tablet != NULL)
		delete tablet;
	if(vmulti != NULL)
		delete vmulti;
		*/

	// Delete filter timer
	if(tablet != NULL) {
		if(tablet->filterTimed != NULL) {
			tablet->filterTimed->StopTimer();
		}
	}

	if(vmulti != NULL) {
		vmulti->ResetReport();
	}
	LOGGER_STOP();
	Sleep(500);

	//printf("Press enter to exit...");
	//getchar();
	exit(code);
}

