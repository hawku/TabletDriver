#include "precompiled.h"

#include <csignal>

#include "VMulti.h"
#include "Tablet.h"
#include "CommandLine.h"
#include "DataFormatter.h"
#include "NamedPipeInput.h"

#define LOG_MODULE ""
#include "Logger.h"

#pragma comment(lib, "hid.lib")
#pragma comment(lib, "setupapi.lib")
#pragma comment(lib, "winusb.lib")
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "ntdll.lib")


// Global variables...
Tablet *tablet;
TabletHandler *tabletHandler;
VMulti *vmulti;
CommandHandler *commandHandler;
OutputManager *outputManager;
ScreenMapper *mapper;

// Named pipes
NamedPipeInput *pipeInput;
NamedPipeServer *pipeOutput;
NamedPipeState *pipeState;

void InitConsole();
bool ProcessCommand(CommandLine *cmd);
bool cleanupStarted = false;
void RunCleanupAndExit(int code);


//
// Main
//
int main(int argc, char**argv) {
	std::string line;
	std::string filename;
	CommandLine *cmd;

	// Init global variables
	vmulti = NULL;
	tablet = NULL;
	tabletHandler = NULL;

	// Init console
	InitConsole();

	// Initialize COM library (Volume control)
	CoInitializeEx(NULL, COINIT_MULTITHREADED);

	// Tablet handler
	tabletHandler = new TabletHandler();

	// Command handler
	commandHandler = new CommandHandler();
	commandHandler->CreateCommands();

	// Screen mapper
	mapper = new ScreenMapper(tablet);
	mapper->primaryMap->SetRotation(0);

	//
	// Command: Start
	//
	// Starts the driver
	//
	commandHandler->AddCommand(new Command("Start", [&](CommandLine *cmd) {

		if (tabletHandler->IsRunning()) {
			LOG_INFO("Driver is already started!\n");
			return true;
		}

		// Unknown tablet
		if (tablet == NULL) {
			LOG_ERROR("Tablet not found!\n");
			CleanupAndExit(0);
			return false;
		}

		// Tablet init
		if (!tablet->Init()) {
			LOG_ERROR("Tablet init failed!\n");
			LOG_ERROR("Possible fixes:\n");
			LOG_ERROR("1) Uninstall other tablet drivers.\n");
			LOG_ERROR("2) Stop other tablet driver services.\n");
			CleanupAndExit(0);
			return false;
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


	// Named pipes
	if (argc > 2) {
		pipeInput = new NamedPipeInput(std::string(argv[2]) + std::string("Input"));
		pipeOutput = new NamedPipeServer(std::string(argv[2]) + std::string("Output"));
		pipeState = new NamedPipeState(std::string(argv[2]) + std::string("State"));
	}
	else {
		pipeInput = new NamedPipeInput("TabletDriverInput");
		pipeOutput = new NamedPipeServer("TabletDriverOutput");
		pipeState = new NamedPipeState("TabletDriverState");
	}
	pipeInput->Start();
	pipeOutput->Start();
	pipeState->Start();

	// Logger output pipe
	logger.writeCallback = [&](void *buffer, int length) {
		if (pipeOutput != NULL && pipeOutput->IsRunning()) {
			pipeOutput->Write(buffer, length);
		}
	};
	//logger.namedPipe = pipeOutput;

	// VMulti XP-Pen
	vmulti = new VMulti(VMulti::TypeXPPen);

	// VMulti VEIKK
	if (!vmulti->isOpen) {
		LOG_ERROR("Can't open XP-Pen's VMulti! Trying VEIKK's VMulti!\n");
		delete vmulti;
		vmulti = new VMulti(VMulti::TypeVEIKK);
	}

	if (!vmulti->isOpen) {
		LOG_ERROR("Can't open VMulti device!\n\n");
		LOG_ERROR("Possible fixes:\n");
		LOG_ERROR("1) Install VMulti driver\n");
		LOG_ERROR("2) Kill PentabletService.exe (XP Pen driver)\n");
		LOG_ERROR("3) Uninstall other tablet drivers and reinstall VMulti driver\n");
		RunCleanupAndExit(0);
		return 0;
	}

	// Output manager
	outputManager = new OutputManager();

	// Read init file
	filename = "init.cfg";

	if (argc > 1) {
		filename = argv[1];
	}
	if (!commandHandler->ExecuteFileLock(filename)) {
		LOG_ERROR("Can't open '%s'\n", filename.c_str());
	}

	if (tablet != NULL) {
		commandHandler->ExecuteCommandLock("RequestStartup");
	}
	else {
		commandHandler->ExecuteCommandLock("CheckTablet");
	}


	//
	// Main loop
	//
	while (true) {

		// Broken pipe
		if (!std::cin) {
			Sleep(100);
		}

		// Read line from the console
		try {
			std::getline(std::cin, line);
		}
		catch (std::exception) {
			Sleep(100);
			//break;
		}

		// Process valid lines
		if (line.length() > 0) {

			// Line to command line
			cmd = new CommandLine(line);

			// Hide console (for the service only mode)
			if (cmd->is("Hide")) {
				::ShowWindow(::GetConsoleWindow(), SW_HIDE);
			}

			// Process all other commands
			else {
				ProcessCommand(cmd);
			}

			delete cmd;
		}
	}

	RunCleanupAndExit(0);
	return 0;
}



//
// Cleanup and exit
//
void RunCleanupAndExit(int code) {

	printf("Cleanup and exit %d\n", code);

	// Wait commands to go through
	printf("Cleanup CommandHandler\n");
	if(commandHandler != NULL)
		delete commandHandler;
		
	// Wait for named pipe writes
	Sleep(100);

	// Stop logger
	printf("Cleanup Logger\n");
	logger.writeCallback = NULL;
	LOGGER_STOP();

	// Named pipes
	printf("Cleanup Input Pipe\n");
	if (pipeInput != NULL) {
		delete pipeInput;
	}
	printf("Cleanup Output Pipe\n");
	if (pipeOutput != NULL) {
		delete pipeOutput;
	}
	printf("Cleanup State Pipe\n");
	if (pipeState != NULL) {
		delete pipeState;
	}

	// TabletHandler
	printf("Cleanup TabletHandler\n");
	if (tabletHandler != NULL) {
		delete tabletHandler;
	}

	// OutputManager
	printf("Cleanup OutputManager\n");
	if (outputManager != NULL) {
		delete outputManager;
	}

	// VMulti
	printf("Cleanup VMulti\n");
	if (vmulti != NULL)
		delete vmulti;

	// Tablet
	printf("Cleanup Tablet\n");
	if (tablet != NULL)
		delete tablet;

	// Uninitialize COM
	CoUninitialize();

	Sleep(500);

	//printf("Press enter to exit...");
	//getchar();
	exit(code);
}

// Cleanup thread
void CleanupAndExit(int code) {
	if (cleanupStarted) return;
	cleanupStarted = true;

	std::thread *t = new std::thread(RunCleanupAndExit, code);
}



//
// Process Command
//
bool ProcessCommand(CommandLine *cmd) {

	// Invalid command
	if (!cmd->isValid) return false;

	// Hide echo input
	if (cmd->is("Echo")) {}
	else { LOG_INFO(">> %s\n", cmd->line.c_str()); }

	//
	// Execute command handler command
	//
	bool logResult = false;
	if (cmd->command.back() == '?') {
		logResult = true;
		cmd->command.pop_back();
	}

	// Command is valid?
	if (commandHandler->IsValidCommand(cmd->command)) {
		bool result = commandHandler->ExecuteCommandLock(cmd->command, cmd);
		if (logResult) {
			LOG_INFO("Result: %s\n", result ? "True" : "False");
		}
		return result;

	}
	// Unknown
	else if (cmd->isValid) {
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

