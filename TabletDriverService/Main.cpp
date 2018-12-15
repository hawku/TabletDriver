#include "stdafx.h"

#include <csignal>

#include "HIDDevice.h"
#include "USBDevice.h"
#include "VMulti.h"
#include "Tablet.h"
#include "ScreenMapper.h"
#include "CommandLine.h"
#include "DataFormatter.h"

#define LOG_MODULE ""
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

void InitConsole();
bool ProcessCommand(CommandLine *cmd);

//
// Main
//
int main(int argc, char**argv) {
	string line;
	string filename;
	CommandLine *cmd;

	// Init global variables
	vmulti = NULL;
	tablet = NULL;
	tabletHandler = NULL;
	
	// Init console
	InitConsole();

	// Tablet handler
	tabletHandler = new TabletHandler();

	// Command handler
	commandHandler = new CommandHandler();
	commandHandler->CreateCommands();

	// Screen mapper
	mapper = new ScreenMapper(tablet);
	mapper->SetRotation(0);

	// Output manager
	outputManager = new OutputManager();

	//
	// Command: Start
	//
	// Starts the driver
	//
	commandHandler->AddCommand(new Command("Start", [&](CommandLine *cmd) {

		if(tabletHandler->isRunning) {
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

		LOG_INFO("TabletDriver started!\n");
		commandHandler->ExecuteCommand("Status");

		return true;
	}));

	//
	// Command: Exit, Quit
	//
	commandHandler->AddAlias("Quit", "Exit");
	commandHandler->AddCommand(new Command("Exit", [&](CommandLine *cmd) {
		LOG_INFO("Bye!\n");
		CleanupAndExit(0);
		return true;
	}));


	// Start logger
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

	//
	// Main loop
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

			// Line to command line
			cmd = new CommandLine(line);

			// Process echo command directly
			if(cmd->is("Echo")) {
				commandHandler->ExecuteCommand(cmd);
			}

			// Hide console (for the service only mode)
			else if(cmd->is("Hide")) {
				::ShowWindow(::GetConsoleWindow(), SW_HIDE);
			}

			// Process all other commands
			else {
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

	// TabletHandler
	if(tabletHandler != NULL) {
		delete tabletHandler;
	}

	// OutputManager
	if(outputManager != NULL) {
		delete outputManager;
	}

	// VMulti
	if(vmulti != NULL)
		delete vmulti;
	
	// Tablet
	if(tablet != NULL)
		delete tablet;

	// Logger
	LOGGER_STOP();
	Sleep(500);

	//printf("Press enter to exit...");
	//getchar();
	exit(code);
}


//
// Process Command
//
bool ProcessCommand(CommandLine *cmd) {

	bool logResult = false;

	LOG_INFO(">> %s\n", cmd->line.c_str());

	//
	// Execute command handler command
	//
	if(cmd->command.back() == '?') {
		logResult = true;
		cmd->command.pop_back();
	}
	if(commandHandler->IsValidCommand(cmd->command)) {
		bool result = commandHandler->ExecuteCommand(cmd->command, cmd);
		if(logResult) {
			LOG_INFO("Result: %s\n", result ? "True" : "False");
		}
		return result;

	}

	// Unknown
	else if(cmd->isValid) {
		LOG_WARNING("Unknown command: %s\n", cmd->line.c_str());
	}

	return true;
}


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

