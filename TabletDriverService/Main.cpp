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
TabletHandler *tabletHandler;
VMulti *vmulti;
CommandHandler *commandHandler;
OutputManager *outputManager;
ScreenMapper *mapper;
//thread *tabletThread;


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
	tabletHandler = NULL;
	//tabletThread = NULL;
	outputManager = new OutputManager();

	// Init console
	InitConsole();

	// Screen mapper
	mapper = new ScreenMapper(tablet);
	mapper->SetRotation(0);

	// Command handler
	commandHandler = new CommandHandler();
	commandHandler->CreateCommands();

	//
	// Command: Start
	//
	// Starts the driver
	//
	commandHandler->AddCommand(new Command("Start", [&](CommandLine *cmd) {
		LOG_INFO(">> %s\n", cmd->line.c_str());

		if(running) {
			LOG_INFO("Driver is already started!\n");
			return true;
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

		// Start tablet handler
		tabletHandler->tablet = tablet;
		tabletHandler->Start();

		// Set running state
		running = true;


		LOG_INFO("TabletDriver started!\n");
		commandHandler->ExecuteCommand("Status");

		return true;
	}));

	//
	// Command: Echo
	//
	commandHandler->AddCommand(new Command("Echo", [&](CommandLine *cmd) {
		if(cmd->valueCount > 0) {
			LOG_INFO("%s\n", cmd->line.c_str() + 5);
		} else {
			LOG_INFO("\n");
		}
		return true;
	}));


	//
	// Command: Exit
	//
	commandHandler->AddCommand(new Command("Exit", [&](CommandLine *cmd) {
		LOG_INFO("Bye!\n");
		CleanupAndExit(0);
		return true;
	}));


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
	if(!commandHandler->ExecuteFile(filename)) {
		LOG_ERROR("Can't open '%s'\n", filename.c_str());
	}

	// Create tablet handler
	tabletHandler = new TabletHandler();

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
			// Hide echo input 
			//
			if(cmd->is("Echo")) {
				commandHandler->ExecuteCommand(cmd);
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

	// Stop filter timer
	if(tabletHandler != NULL) {
		tabletHandler->StopTimer();
	}

	// Reset output
	if(outputManager != NULL) {
		outputManager->Reset();
	}

	LOGGER_STOP();
	Sleep(500);

	//printf("Press enter to exit...");
	//getchar();
	exit(code);
}

