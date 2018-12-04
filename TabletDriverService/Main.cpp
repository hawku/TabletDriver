#include "stdafx.h"

#include <csignal>

#include "HIDDevice.h"
#include "USBDevice.h"
#include "VMulti.h"
#include "Tablet.h"
#include "ScreenMapper.h"
#include "CommandLine.h"
#include "ProcessCommand.h"

#define LOG_MODULE "Main"
#include "Logger.h"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "winusb.lib")


// Global variables...
Tablet *tablet;
VMulti *vmulti;
OutputManager *outputManager;
ScreenMapper *mapper;
thread *tabletThread;
void SetOutput(TabletState *state);

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
// Tablet process
//
void RunTabletThread() {
	int status;
	bool isFirstReport = true;
	bool isResent = false;
	TabletFilter *filter;
	bool filterTimedEnabled;

	chrono::high_resolution_clock::time_point timeBegin = chrono::high_resolution_clock::now();

	//
	// Main Loop
	//

	while(true) {

		//
		// Read tablet position
		//
		status = tablet->ReadPosition();

		// Position OK
		if(status == Tablet::ReportValid) {
			isResent = false;

		// Invalid report id
		} else if(status == Tablet::ReportInvalid) {
			continue;

		// Valid report but position is not in-range or invalid
		} else if(status == Tablet::ReportPositionInvalid) {
			if(!isResent && tablet->state.isValid) {
				isResent = true;
				tablet->state.isValid = false;
			} else {
				continue;
			}

		// Ignore report
		} else if(status == Tablet::ReportIgnore) {
			continue;

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
		if(logger.debugEnabled) {
			double delta = (tablet->state.time - timeBegin).count() / 1000000.0;
			LOG_DEBUG("TabletState: T=%0.3f, B=%d, X=%0.3f, Y=%0.3f, P=%0.3f\n",
				delta,
				tablet->state.buttons,
				tablet->state.position.x,
				tablet->state.position.y,
				tablet->state.pressure
			);
		}


		// Set output values
		if(status == Tablet::ReportPositionInvalid) {
			tablet->state.buttons = 0;
		}

		//
		// Report filters
		//
		// Is there any filters?
		if(tablet->filterReportCount > 0) {

			// Loop through filters
			for(int filterIndex = 0; filterIndex < tablet->filterReportCount; filterIndex++) {

				// Filter
				filter = tablet->filterReport[filterIndex];

				// Enabled?
				if(filter != NULL && filter->isEnabled) {

					// Process
					filter->SetTarget(&tablet->state);
					filter->Update();
					filter->GetPosition(&tablet->state.position);
				}

			}
		}


		// Timed filter enabled?
		filterTimedEnabled = false;
		for(int filterIndex = 0; filterIndex < tablet->filterTimedCount; filterIndex++) {
			if(tablet->filterTimed[filterIndex]->isEnabled)
				filterTimedEnabled = true;
		}

		// Do not write report when timed filter is enabled
		if(filterTimedEnabled) {
			continue;
		}

		SetOutput(&tablet->state);

	}

}

//
// Tablet filter timer callback
//
VOID CALLBACK FilterTimerCallback(_In_ PVOID lpParameter, _In_ BOOLEAN TimerOrWaitFired) {
	Vector2D position;
	TabletFilter *filter;
	TabletState outputState;
	bool filterEnabled = false;

	// Set position
	if(tablet->state.isValid) {
		position.Set(tablet->state.position);
	} else {
		return;
	}

	memcpy(&outputState, &tablet->state, sizeof(outputState));
	


	// Loop through filters
	for(int filterIndex = 0; filterIndex < tablet->filterTimedCount; filterIndex++) {

		// Filter
		filter = tablet->filterTimed[filterIndex];

		// Filter enabled?
		if(filter->isEnabled) {
			filterEnabled = true;
		} else {
			continue;
		}

		// Set filter targets
		filter->SetTarget(&outputState);

		// Update filter position
		filter->Update();

		// Set output vector
		filter->GetPosition(&outputState.position);

	}

	if(!filterEnabled) {
		return;
	}

	SetOutput(&outputState);
}


//
// Set Output
//
void SetOutput(TabletState *outputState) {
	outputManager->Set(outputState);
	outputManager->Write();
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
	outputManager = new OutputManager();

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
				if(tablet->filterReportCount > 0) {
					tablet->filterTimed[0]->callback = FilterTimerCallback;
					tablet->filterTimed[0]->StartTimer();
				}

				// Start the tablet thread
				tabletThread = new thread(RunTabletThread);

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
		if(tablet->filterTimedCount != 0) {
			tablet->filterTimed[0]->StopTimer();
		}
	}

	if(outputManager != NULL) {
		outputManager->Reset();
	}
	LOGGER_STOP();
	Sleep(500);

	//printf("Press enter to exit...");
	//getchar();
	exit(code);
}

